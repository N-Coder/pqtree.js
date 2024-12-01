#include <pctree/PCTree.h>

#include <iostream>
#include <string>
#include <variant>
#include <vector>

#include <emscripten/bind.h>

#include "layout.h"

using namespace std;
using namespace pc_tree;
vector<PCNode*> leaves;
unique_ptr<PCTree> tree;
PCTreeNodeArray<string> labels;

bool setRestrictions(string spec, bool is_circular) {
	istringstream f(spec);
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
			tree.reset();
			return false;
		}
	}
	return true;
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

void addSvgNode(stringstream& ss, const string& tag,
		initializer_list<tuple<string, variant<string, double, int>>> values) {
	string text;
	ss << "<" << tag << " ";
	for (auto [key, var] : values) {
		if (key == "text") {
			text = get<string>(var);
			continue;
		}
		string value;
		if (holds_alternative<int>(var)) {
			value = to_string(get<int>(var));
		} else if (holds_alternative<double>(var)) {
			value = to_string(get<double>(var));
		} else {
			value = get<string>(var);
		}
		ss << key << "=\"" << value << "\" ";
	}
	if (!text.empty()) {
		ss << ">" << text << "</" << tag << ">";
	} else {
		ss << "/>";
	}
}

string getLeaves(PCNode* below) {
	stringstream ss;
	bool first = true;
	for (PCNode* leaf : FilteringPCTreeDFS(*tree, below)) {
		if (leaf->isLeaf()) {
			if (first) {
				first = false;
			} else {
				ss << " ";
			}
			ss << labels[leaf];
		}
	}
	return ss.str();
}

void drawSvgNodesCircular(PCTree& tree, stringstream& ss, Layout& positions, double off_x = 0,
		double off_y = 0) {
	for (auto node : tree.allNodes()) {
		auto [cx, cy] = positions[node];
		cx += off_x;
		cy += off_y;
		if (node->isLeaf()) {
			addSvgNode(ss, "circle",
					{{"cx", cx}, {"cy", cy}, {"r", "15"}, {"fill", "white"}, {"stroke", "#666666"},
							{"data-leaves", labels[node]}});
			addSvgNode(ss, "text",
					{
							{"x", cx},
							{"y", cy},
							{"text-anchor", "middle"},
							{"dominant-baseline", "central"},
							{"text", labels[node]},
					});
			continue;
		}

		for (auto child : node->children()) {
			auto [childCX, childCY] = positions[child];
			childCX += off_x;
			childCY += off_y;
			addSvgNode(ss, "line",
					{{"x1", cx}, {"y1", cy}, {"x2", childCX}, {"y2", childCY}, {"stroke", "black"}});
		}

		if (node->getNodeType() == PCNodeType::PNode) {
			addSvgNode(ss, "circle",
					{
							{"cx", cx}, {"cy", cy}, {"r", "5"}, {"fill", "black"}, {"stroke", "black"},
							// {"dataLeaves", getLeaves(node)},
					});
		} else {
			addSvgNode(ss, "circle",
					{
							{"cx", cx}, {"cy", cy}, {"r", "20"}, {"fill", "#ececec"},
							{"stroke", "black"},
							// {"dataLeaves", getLeaves(node)},
					});
			addSvgNode(ss, "circle",
					{{"cx", cx}, {"cy", cy}, {"r", "15"}, {"fill", "transparent"},
							{"stroke", "black"}, {"pointer-events", "none"}});
			addSvgNode(ss, "text",
					{{"x", (cx - 1)}, {"y", cy}, {"text-anchor", "middle"},
							{"dominant-baseline", "central"}, {"text", "C"}});
		}
	}
}

void drawSvgNodesLinear(PCTree& tree, stringstream& ss, Layout& positions,
		PCTreeNodeArray<double>* widths) {
	for (auto node : tree.allNodes()) {
		if (node->isLeaf() && node == tree.getRootNode()) {
			continue;
		}
		auto [cx, cy] = positions[node];
		if (node->isLeaf()) {
			// cy -= 15;
			double ratio = 0.866; // equilateral triangle
			double sideLength = 40;
			stringstream points;
			points << cx << "," << cy << " ";
			points << cx - sideLength / 2 << "," << cy + sideLength * ratio << " ";
			points << cx + sideLength / 2 << "," << cy + sideLength * ratio;
			addSvgNode(ss, "polygon",
					{{"points", points.str()}, {"fill", "white"}, {"stroke", "black"},
							{"data-leaves", labels[node]}});
			addSvgNode(ss, "text",
					{{"x", cx}, {"y", (cy + 0.69 * sideLength * ratio)}, {"text-anchor", "middle"},
							{"dominant-baseline", "central"}, {"text", labels[node]}});
			continue;
		}

		if (node->getNodeType() == PCNodeType::PNode && node != tree.getRootNode()) {
			cy += 15;
		}

		for (auto child : node->children()) {
			auto [childCX, childCY] = positions[child];
			if (child->getNodeType() == PCNodeType::PNode) {
				childCY += 15;
			}
			addSvgNode(ss, "line",
					{{"x1", (node->getNodeType() == PCNodeType::PNode ? cx : childCX)}, {"y1", cy},
							{"x2", childCX}, {"y2", childCY}, {"stroke", "black"}});
		}

		if (node->getNodeType() == PCNodeType::PNode) {
			addSvgNode(ss, "circle",
					{
							{"cx", cx},
							{"cy", cy},
							{"r", "15"},
							{"fill", "#ececec"},
							{"stroke", "black"},
							{"data-leaves", getLeaves(node)},
					});
			addSvgNode(ss, "text",
					{{"x", (cx + 0.4)}, {"y", cy}, {"text-anchor", "middle"},
							{"dominant-baseline", "central"}, {"text", "P"}});
		} else {
			auto height = 0.3 * 80; // levelHeight
			double left_x = 0, width = 0;
			if (widths && (*widths)[node] > 0) {
				auto buffer = 0.1 * 70; // leafWidth
				width = (*widths)[node];
				left_x = cx - width / 2 + buffer;
				width -= 2 * buffer;
			} else {
				auto buffer = 0.4 * 70; // leafWidth
				double first_child_x = get<0>(positions[node->getChild1()]);
				double last_child_x = get<0>(positions[node->getChild2()]);
				if (first_child_x > last_child_x) {
					swap(first_child_x, last_child_x);
				}
				left_x = first_child_x - buffer;
				width = (last_child_x - first_child_x) + 2 * buffer;
			}
			addSvgNode(ss, "rect",
					{
							{"x", left_x},
							{"y", cy},
							{"width", width},
							{"height", height},
							{"fill", "#ececec"},
							{"stroke", "black"},
							{"data-leaves", getLeaves(node)},
					});
			addSvgNode(ss, "text",
					{{"x", cx}, {"y", (cy + height / 2)}, {"text-anchor", "middle"},
							{"dominant-baseline", "central"}, {"text", "Q"}});
		}
	}
}

string drawSVG(bool is_circular) {
	if (!tree) {
		return "";
	}

	Layout positions(*tree);
	stringstream ss;
	if (is_circular) {
		PCTreeNodeArray<double> weights(*tree);
		// TODO improve line length ratios by using better weights
		// double max_weight = computeCircularWeightByHeight(*tree, weights);
		// for (auto node : tree->allNodes()) {
		//   weights[node] = max_weight - weights[node] + 1;
		// }
		computePositionsCircular(*tree, positions, 200, nullptr, 216, 216);

		ss << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << 432 << "\" height=\"" << 432
		   << "\">";
		addSvgNode(ss, "circle",
				{{"cx", 216}, {"cy", 216}, {"r", 200}, {"fill", "transparent"},
						{"stroke", "#cccccc"}, {"pointer-events", "none"}});
		drawSvgNodesCircular(*tree, ss, positions);
	} else {
		PCTreeNodeArray<double> widths(*tree);
		auto [width, height] = computePositionsLinear(*tree, positions, &widths, 80, 70, 0, -79);

		ss << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << width << "\" height=\""
		   << (height - 119) << "\">";
		drawSvgNodesLinear(*tree, ss, positions, &widths);
	}
	ss << "</svg>";
	return ss.str();
}

string drawTikz(bool is_circular) {
	if (!tree) {
		return "";
	}
	stringstream ss;
	ss << "\\begin{tikzpicture}[leaf/.style={fill=black!10},pnode/"
		  ".style={leaf},qnode/.style={leaf}]\n";

	// TODO implement

	ss << "\\end{tikzpicture}";
	return ss.str();
}

// TODO add to-ipe code

EMSCRIPTEN_BINDINGS(PCTreeModule) {
	emscripten::function("setRestrictions", &setRestrictions);
	emscripten::function("getLeafOrder", &getLeafOrder);
	emscripten::function("getOrderCount", &getOrderCount);
	emscripten::function("getAllOrders", &getAllOrders);
	emscripten::function("drawSVG", &drawSVG);
	emscripten::function("drawTikz", &drawTikz);
}
