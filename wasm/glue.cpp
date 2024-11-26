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

const double leafWidth = 70;
const double levelHeight = 80;

int numLeaves(PCNode *node) {
  if (node->isLeaf())
    return 1;
  int count = 0;
  for (auto child : node->children()) {
    count += numLeaves(child);
  }
  return count;
}
void drawPQNodeSVG(stringstream &ss, PCNode *node, double cx, double cy,
                   double &line_x, double &max_y);

string drawSVG(bool is_circular) {
  if (!tree)
    return "";
  stringstream ss;
  ss << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"500\" "
        "height=\"300\">";

  auto root = tree->getRootNode()->getChild1();
  auto overallWidth = numLeaves(root) * leafWidth;
  double computedHeight = 0;
  double dummy = 0;
  drawPQNodeSVG(ss, root, overallWidth / 2, 20, dummy, computedHeight);
  computedHeight += 5;

  ss << "</svg>";
  return ss.str();
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

void drawPQNodeSVG(stringstream &ss, PCNode *node, double cx, double cy,
                   double &line_x, double &max_y) {
  if (node->isLeaf()) {
    double ratio = 0.866; // equilateral triangle
    double sideLength = 40;
    stringstream points;
    points << cx << "," << cy << " ";
    points << cx - sideLength / 2 << "," << cy + sideLength * ratio << " ";
    points << cx + sideLength / 2 << "," << cy + sideLength * ratio;
    addSvgNode(
        ss, "polygon",
        {
            {"points", points.str()}, {"fill", "#ececec"}, {"stroke", "black"},
            // {"dataLeaves", getLeaves(node)}
        });
    addSvgNode(ss, "text",
               {{"x", to_string(cx)},
                {"y", to_string(cy + 0.69 * sideLength * ratio)},
                {"textAnchor", "middle"},
                {"dominantBaseline", "middle"},
                {"text", labels[node]}});
    max_y = max(cy + sideLength * ratio, max_y);
    return;
  } // else if (node->getNodeType() == PCNodeType::CNode) {

  auto myWidth = numLeaves(node) * leafWidth;
  auto left = cx - myWidth / 2;
  for (auto child : node->children()) {
    auto childWidth = numLeaves(child) * leafWidth;
    auto childCX = left + childWidth / 2;
    double childCY;
    if (child->isLeaf()) {
      childCY = cy + 0.5 * levelHeight;
    } else {
      childCY = cy + levelHeight;
    }
    drawPQNodeSVG(ss, child, childCX, childCY, childCX, max_y);
    addSvgNode(ss, "line",
               {
                   {"x1", to_string(cx)},
                   {"y1", to_string(cy)},
                   {"x2", to_string(childCX)},
                   {"y2", to_string(childCY)},
                   {"stroke", "black"},
               });
    left += childWidth;
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
                 {"textAnchor", "middle"},
                 {"dominantBaseline", "middle"},
                 {"text", "P"},
             });
  // } else if (node->getNodeType() == PCNodeType::PNode) {
  //   auto myWidth = numLeaves(node) * leafWidth;
  //   auto left = cx - myWidth / 2;
  //   vector<double> childCXs;
  //   for (auto child : node->children()) {
  //     auto childWidth = numLeaves(child) * leafWidth;
  //     auto childCX = left + childWidth / 2;
  //     childCXs.push_back(childCX);
  //     double childCY;
  //     if (child->isLeaf()) {
  //       childCY = cy + 0.5 * levelHeight;
  //     } else {
  //       childCY = cy + levelHeight;
  //     }
  //     auto line = addSvgNode(ss, "line",
  //                            {
  //                                {"x1", to_string(cx)},
  //                                {"y1", to_string(cy)},
  //                                {"x2", to_string(childCX)},
  //                                {"y2", to_string(childCY)},
  //                                {"stroke", "black"},
  //                            });
  //     drawPQNodeSVG(ss, child, childCX, childCY, line);
  //     left += childWidth;
  //   }
  //   auto myWidth = childCXs.back() - childCXs[0];
  //   auto buffer = 0.2 * leafWidth;
  //   auto myHeight = 0.3 * levelHeight;
  //   addSvgNode(ss, "rect",
  //              {
  //                  {"x", childCXs[0] - buffer},
  //                  {"y", cy - myHeight / 2},
  //                  {"width", myWidth + 2 * buffer},
  //                  {"height", myHeight},
  //                  {"fill", "#ececec"},
  //                  {"stroke", "black"},
  //                  {"dataLeaves", getLeaves(node)},
  //              });
  //   auto center = childCXs[0] + myWidth / 2;
  //   addSvgNode(ss, "text",
  //              {{"x", center},
  //               {"y", cy + 1},
  //               {"textAnchor", "middle"},
  //               {"dominantBaseline", "middle"},
  //               {"text", "Q"}});
  //   line_x = center;
  // }
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
