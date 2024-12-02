#include <pctree/PCTree.h>

#include <fstream>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

#include "drawing.h"
#include "layout.h"

using namespace std;
using namespace pc_tree;

vector<PCNode*> leaves;
unique_ptr<PCTree> tree;
PCTreeNodeArray<string> labels;

bool readRestrictions(bool is_circular) {
	string line;
	size_t degree = 0;
	vector<PCNode*> cons;
	while (std::getline(cin, line)) {
		if (line.empty() || line[0] == '#') {
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

void draw(Drawer* drawer, std::ostream& ss) {
	if (!tree) {
		return;
	}

	Layout positions(*tree);
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
}

void drawSVG(bool is_circular, std::ostream& file) {
	unique_ptr<Drawer> ptr;
	if (is_circular) {
		ptr.reset(new SVGCircularDrawer());
	} else {
		ptr.reset(new SVGLinearDrawer());
	}
	draw(ptr.get(), file);
}

void drawIPE(bool is_circular, std::ostream& file) {
	unique_ptr<Drawer> ptr;
	if (is_circular) {
		ptr.reset(new IPECircularDrawer());
	} else {
		ptr.reset(new IPELinearDrawer());
	}
	draw(ptr.get(), file);
}

void drawTikz(bool is_circular, std::ostream& file) {
	unique_ptr<Drawer> ptr;
	if (is_circular) {
		ptr.reset(new TikzCircularDrawer());
	} else {
		ptr.reset(new TikzLinearDrawer());
	}
	draw(ptr.get(), file);
}

int main(int argc, char** argv) {
	bool is_circular = argc > 1;
	readRestrictions(is_circular);
	ofstream svgf {"tree-svg.svg"};
	drawSVG(is_circular, svgf);
	ofstream ipef {"tree-ipe.ipe"};
	drawIPE(is_circular, ipef);
	ofstream tikzf {"tree.tex"};
	tikzf << "\\documentclass{article}\n\\usepackage{tikz}\n\\begin{document}\n";
	tikzf << "\\section{SVG}\n\\includegraphics{tree-svg}\n\\newpage";
	tikzf << "\\section{IPE}\n\\includegraphics{tree-ipe}\n\\newpage";
	tikzf << "\\section{TiKZ}\n";
	drawTikz(is_circular, tikzf);
	tikzf << "\\end{document}\n";
	return 0;
}
