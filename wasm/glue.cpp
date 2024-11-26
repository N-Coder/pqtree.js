#include <iostream>
#include <pctree/PCTree.h>
#include <string>

#include "layout.h"

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

void addSvgNode(stringstream &ss, const string &tag,
                initializer_list<tuple<string, string>> values) {
  string text;
  ss << "<" << tag << " ";
  for (auto value : values) {
    if (get<0>(value) == "text") {
      text = get<1>(value);
      continue;
    }
    ss << get<0>(value) << "=\"" << get<1>(value) << "\" ";
  }
  if (!text.empty()) {
    ss << ">" << text << "</" << tag << ">";
  } else {
    ss << "/>";
  }
}

string drawSVG(bool is_circular) {
  if (!tree)
    return "";

if (is_circular) {
  Layout positions(*tree);
  computePositionsCircular(*tree, positions, 200);

  stringstream ss;
  ss << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << 410
     << "\" height=\"" << 410 << "\">";

  for (auto node : tree->allNodes()) {
    auto [cx, cy] = positions[node];
    cx+=205;
    cy+=205;
    if (node->isLeaf()) {
      double ratio = 0.866; // equilateral triangle
      double sideLength = 40;
      stringstream points;
      points << cx << "," << cy << " ";
      points << cx - sideLength / 2 << "," << cy + sideLength * ratio << " ";
      points << cx + sideLength / 2 << "," << cy + sideLength * ratio;
      addSvgNode(ss, "polygon",
                 {
                     {"points", points.str()},
                     {"fill", "#ececec"},
                     {"stroke", "black"},
                     // {"dataLeaves", getLeaves(node)}
                 });
      addSvgNode(ss, "text",
                 {{"x", to_string(cx)},
                  {"y", to_string(cy + 0.69 * sideLength * ratio)},
                  {"text-anchor", "middle"},
                  {"dominant-baseline", "middle"},
                  {"text", labels[node]}});
    } else if (node->getNodeType() == PCNodeType::PNode) {
      for (auto child : node->children()) {
        auto [childCX, childCY] = positions[child];
        childCX+=205;
        childCY+=205;
        addSvgNode(ss, "line",
                   {
                       {"x1", to_string(cx)},
                       {"y1", to_string(cy)},
                       {"x2", to_string(childCX)},
                       {"y2", to_string(childCY)},
                       {"stroke", "black"},
                   });
      }
      addSvgNode(ss, "circle",
                 {
                     {"cx", to_string(cx)},
                     {"cy", to_string(cy)},
                     {"r", "15"},
                     {"fill", "#ececec"},
                     {"stroke", "black"},
                     // {"dataLeaves", getLeaves(node)},
                 });
      addSvgNode(ss, "text",
                 {
                     {"x", to_string(cx + 0.4)},
                     {"y", to_string(cy + 1)},
                     {"text-anchor", "middle"},
                     {"dominant-baseline", "middle"},
                     {"text", "P"},
                 });
    } else {
      for (auto child : node->children()) {
        auto [childCX, childCY] = positions[child];
        childCX+=205;
        childCY+=205;
        addSvgNode(ss, "line",
                   {
                       {"x1", to_string(cx)},
                       {"y1", to_string(cy)},
                       {"x2", to_string(childCX)},
                       {"y2", to_string(childCY)},
                       {"stroke", "black"},
                   });
      }
      addSvgNode(ss, "circle",
                 {
                     {"cx", to_string(cx)},
                     {"cy", to_string(cy)},
                     {"r", "15"},
                     {"fill", "#ececec"},
                     {"stroke", "black"},
                     // {"dataLeaves", getLeaves(node)},
                 });
      addSvgNode(ss, "circle",
                 {
                     {"cx", to_string(cx)},
                     {"cy", to_string(cy)},
                     {"r", "20"},
                     {"fill", "#ececec"},
                     {"stroke", "black"},
                     // {"dataLeaves", getLeaves(node)},
                 });
      addSvgNode(ss, "text",
                 {{"x", to_string(cx)},
                  {"y", to_string(cy + 1)},
                  {"text-anchor", "middle"},
                  {"dominant-baseline", "middle"},
                  {"text", "Q"}});
    }
  }

  ss << "</svg>";
  return ss.str();
}

  Layout positions(*tree);
  auto [width, height] =
      computePositionsLinear(*tree, positions, 80, 70, 0, -40);

  stringstream ss;
  ss << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << width
     << "\" height=\"" << (height - 80) << "\">";

  for (auto node : tree->allNodes()) {
    if (node == tree->getRootNode())
      continue;
    auto [cx, cy] = positions[node];
    if (node->isLeaf()) {
      double ratio = 0.866; // equilateral triangle
      double sideLength = 40;
      stringstream points;
      points << cx << "," << cy << " ";
      points << cx - sideLength / 2 << "," << cy + sideLength * ratio << " ";
      points << cx + sideLength / 2 << "," << cy + sideLength * ratio;
      addSvgNode(ss, "polygon",
                 {
                     {"points", points.str()},
                     {"fill", "#ececec"},
                     {"stroke", "black"},
                     // {"dataLeaves", getLeaves(node)}
                 });
      addSvgNode(ss, "text",
                 {{"x", to_string(cx)},
                  {"y", to_string(cy + 0.69 * sideLength * ratio)},
                  {"text-anchor", "middle"},
                  {"dominant-baseline", "middle"},
                  {"text", labels[node]}});
    } else if (node->getNodeType() == PCNodeType::PNode) {
      for (auto child : node->children()) {
        auto [childCX, childCY] = positions[child];
        addSvgNode(ss, "line",
                   {
                       {"x1", to_string(cx)},
                       {"y1", to_string(cy)},
                       {"x2", to_string(childCX)},
                       {"y2", to_string(childCY)},
                       {"stroke", "black"},
                   });
      }
      addSvgNode(ss, "circle",
                 {
                     {"cx", to_string(cx)},
                     {"cy", to_string(cy)},
                     {"r", "15"},
                     {"fill", "#ececec"},
                     {"stroke", "black"},
                     // {"dataLeaves", getLeaves(node)},
                 });
      addSvgNode(ss, "text",
                 {
                     {"x", to_string(cx + 0.4)},
                     {"y", to_string(cy + 1)},
                     {"text-anchor", "middle"},
                     {"dominant-baseline", "middle"},
                     {"text", "P"},
                 });
    } else {
      for (auto child : node->children()) {
        auto [childCX, childCY] = positions[child];
        addSvgNode(ss, "line",
                   {
                       {"x1", to_string(childCX)},
                       {"y1", to_string(cy)},
                       {"x2", to_string(childCX)},
                       {"y2", to_string(childCY)},
                       {"stroke", "black"},
                   });
      }
      auto buffer = 0.2 * 70;   // leafWidth
      auto myHeight = 0.3 * 80; // levelHeight
      double first_child_x = get<0>(positions[node->getChild1()]);
      double last_child_x = get<0>(positions[node->getChild2()]);
      if (first_child_x > last_child_x) {
        swap(first_child_x, last_child_x);
      }
      addSvgNode(
          ss, "rect",
          {
              {"x", to_string(first_child_x - buffer)},
              {"y", to_string(cy - myHeight / 2)},
              {"width", to_string((last_child_x - first_child_x) + 2 * buffer)},
              {"height", to_string(myHeight)},
              {"fill", "#ececec"},
              {"stroke", "black"},
              // {"dataLeaves", getLeaves(node)},
          });
      addSvgNode(ss, "text",
                 {{"x", to_string(cx)},
                  {"y", to_string(cy + 1)},
                  {"text-anchor", "middle"},
                  {"dominant-baseline", "middle"},
                  {"text", "Q"}});
    }
  }

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
