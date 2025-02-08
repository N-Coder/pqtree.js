#include <pctree/PCTree.h>

#include <iostream>
#include <string>
#include <variant>
#include <vector>

#include <emscripten/bind.h>

#include "drawing.h"
#include "layout.h"

using namespace std;
using namespace pc_tree;
vector<PCNode*> leaves;
unique_ptr<PCTree> tree;
PCTreeNodeArray<string> labels;

bool compareNodes(PCNode* a, PCNode* b) { return labels[a] < labels[b]; }

void printLabel(std::ostream& os, PCNode* n, int) {
	if (!labels[n].empty()) {
		os << labels[n];
	}
}

int setRestrictions(string spec, bool is_circular) {
	istringstream f(spec);
	int restr_nr = 0;
	string line;
	size_t degree = 0;
	vector<PCNode*> cons;
	while (std::getline(f, line)) {
		if (line.empty()) {
			continue;
		} else if (degree == 0) {
			degree = line.size();

			leaves.clear();
			// cout << "PCTree(" << degree + (is_circular ? 0 : 1) << ")" << endl;
			tree = make_unique<PCTree>(degree + (is_circular ? 0 : 1), &leaves);
			labels.init(*tree);
			for (size_t i = 0; i < degree; i++) {
				labels[leaves[i]] = to_string(i);
			}
			if (!is_circular) {
				tree->changeRoot(leaves.back());
			}
		} else if (degree != line.size()) {
			cerr << "Line size mismatch! Line '" << line << "' should have length " << degree << endl;
			return false;
		}

		cons.clear();
		// cout << "makeConsecutive(";
		for (size_t i = 0; i < line.size(); i++) {
			if (line[i] == '1') {
				cons.push_back(leaves[i]);
				// cout << i << ", ";
			}
		}
		// cout << ")" << endl;
		if (!tree->makeConsecutive(cons)) {
			// tree.reset();
			return restr_nr;
		}
		restr_nr++;
	}
	return -1;
}

string serializeTree(bool is_circular) {
	if (!tree) {
		return "";
	} else if (is_circular) {
		return tree->uniqueID(printLabel, compareNodes);
	} else {
		return tree->uniqueIDRooted(printLabel, compareNodes, false);
	}
}

string treeSpecToMatrix(const string& spec, bool is_circular) {
	unique_ptr<PCTree> tree;
	try {
		cout << spec << endl;
		tree = make_unique<PCTree>(spec, true);
	} catch (const invalid_argument& e) {
		return string("!") + e.what();
	}
	if (!is_circular) {
		tree->changeRoot(tree->newNode(PCNodeType::Leaf, tree->getRootNode()));
	}
	cout << tree << endl;
	vector<vector<PCNode*>> restr;
	tree->getRestrictions(restr, is_circular ? nullptr : tree->getRootNode());

	vector<PCNode*> leaves(tree->getLeaves().begin(), tree->getLeaves().end());
	if (!is_circular) {
		leaves.erase(std::remove(leaves.begin(), leaves.end(), tree->getRootNode()), leaves.end());
	}
	sort(leaves.begin(), leaves.end(), [](PCNode* a, PCNode* b) { return a->index() < b->index(); });

	stringstream ss;
	PCTreeNodeSet<> cons(*tree);
	for (const auto& row : restr) {
		cons.clear();
		for (auto leaf : row) {
			cons.insert(leaf);
		}
		for (auto leaf : leaves) {
			ss << (cons.isMember(leaf) ? "1" : "0");
		}
		ss << endl;
	}
	return ss.str();
}

void printLeafOrder(stringstream& ss) {
	if (!tree) {
		return;
	}
	bool first = true;
	for (PCNode* l : FilteringPCTreeDFS(*tree, tree->getRootNode())) {
		if (l->isLeaf() && !labels[l].empty()) {
			if (first) {
				first = false;
			} else {
				ss << " ";
			}
			ss << labels[l];
		}
	}
}

string getLeafOrder() {
	stringstream ss;
	printLeafOrder(ss);
	return ss.str();
}

uint32_t getOrderCount() {
	if (tree) {
		return tree->possibleOrders<uint32_t>();
	} else {
		return 0;
	}
}

string getAllOrders() {
	if (!tree) {
		return "";
	}
	stringstream ss;
	tree->firstEmbedding();
	do {
		printLeafOrder(ss);
		ss << endl;
	} while (tree->nextEmbedding());
	return ss.str();
}

string draw(Drawer* drawer) {
	if (!tree) {
		return "";
	}

	Layout positions(*tree);
	stringstream ss;
	if (dynamic_cast<CircularDrawer*>(drawer)) {
		PCTreeNodeArray<double> weights(*tree);
		// TODO improve line length ratios by using better weights
		// double max_weight = computeCircularWeightByHeight(*tree, weights);
		// for (auto node : tree->allNodes()) {
		//   weights[node] = max_weight - weights[node] + 1;
		// }
		computePositionsCircular(*tree, positions, 200, nullptr);

		CircularDrawer& cdrawer = *dynamic_cast<CircularDrawer*>(drawer);
		cdrawer.labels = &labels;
		cdrawer.radius = 200;
		cdrawer.draw(*tree, positions, ss);
	} else {
		PCTreeNodeArray<double> widths(*tree);
		auto [width, height] = computePositionsLinear(*tree, positions, &widths, 80, 70, 0, -79);

		LinearDrawer& ldrawer = *dynamic_cast<LinearDrawer*>(drawer);
		ldrawer.labels = &labels;
		ldrawer.widths = &widths;
		ldrawer.width = width;
		ldrawer.height = height - 119;
		ldrawer.draw(*tree, positions, ss);
	}

	return ss.str();
}

string drawSVG(bool is_circular) {
	unique_ptr<Drawer> ptr;
	if (is_circular) {
		ptr.reset(new SVGCircularDrawer());
	} else {
		ptr.reset(new SVGLinearDrawer());
	}
	return draw(ptr.get());
}

string drawIPE(bool is_circular) {
	unique_ptr<Drawer> ptr;
	if (is_circular) {
		ptr.reset(new IPECircularDrawer());
	} else {
		ptr.reset(new IPELinearDrawer());
	}
	return draw(ptr.get());
}

string drawTikz(bool is_circular) {
	unique_ptr<Drawer> ptr;
	if (is_circular) {
		ptr.reset(new TikzCircularDrawer());
	} else {
		ptr.reset(new TikzLinearDrawer());
	}
	return draw(ptr.get());
}

EMSCRIPTEN_BINDINGS(PCTreeModule) {
	emscripten::function("setRestrictions", &setRestrictions);
	emscripten::function("serializeTree", &serializeTree);
	emscripten::function("treeSpecToMatrix", &treeSpecToMatrix);
	emscripten::function("getLeafOrder", &getLeafOrder);
	emscripten::function("getOrderCount", &getOrderCount);
	emscripten::function("getAllOrders", &getAllOrders);
	emscripten::function("drawSVG", &drawSVG);
	emscripten::function("drawIPE", &drawIPE);
	emscripten::function("drawTikz", &drawTikz);
}
