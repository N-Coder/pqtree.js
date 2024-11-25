#include <iostream>
#include <pctree/PCTree.h>
#include <string>

#include <emscripten/bind.h>

using namespace std;
using namespace pc_tree;
vector<PCNode *> leaves;
unique_ptr<PCTree> tree;
PCTreeNodeArray<string> labels;

bool setRestrictions(string spec, bool is_circular) {
  istringstream f(spec);
  string line;
  size_t degree = 0;
  vector<PCNode *> cons;
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
      cerr << "Line size mismatch! Line '" << line << "' should have length "
           << degree << endl;
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

string getLeafOrder() {
  if (!tree) {
    return "";
  }
  vector<PCNode *> L;
  tree->currentLeafOrder(L);
  stringstream ss;
  bool first = true;
  for (auto l : L) {
    if (!labels[l].empty()) {
      if (first) {
        first = false;
      } else {
        ss << " ";
      }
      ss << labels[l];
    }
  }
  return ss.str();
}

uint32_t getOrderCount() {
  if (tree)
    return tree->possibleOrders<uint32_t>();
  else
    return 0;
}

string getAllOrders() {
  if (tree)
    return getLeafOrder() + "\n";
  else
    return "";
}

string drawSVG(bool is_circular) {
  if (!tree)
    return "";
  stringstream ss;
  ss << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"490\" "
        "height=\"259.64\">";

  ss << "</svg>";
  return ss.str();
}

string drawTikz(bool is_circular) {
  if (!tree)
    return "";
  stringstream ss;
  ss << "\\begin{tikzpicture}[leaf/.style={fill=black!10},pnode/"
        ".style={leaf},qnode/.style={leaf}]\n";

  ss << "\\end{tikzpicture}";
  return ss.str();
}

EMSCRIPTEN_BINDINGS(PCTreeModule) {
  emscripten::function("setRestrictions", &setRestrictions);
  emscripten::function("getLeafOrder", &getLeafOrder);
  emscripten::function("getOrderCount", &getOrderCount);
  emscripten::function("getAllOrders", &getAllOrders);
  emscripten::function("drawSVG", &drawSVG);
  emscripten::function("drawTikz", &drawTikz);
}
