#pragma once

#include <pctree/PCTree.h>
#include <vector>

using Point = std::tuple<double, double>;
using Layout = pc_tree::PCTreeNodeArray<Point>;

/**
 * Computes all node positions in the subtree starting at node, placed
 * left-top aligned to the given coordinates. Returns the width of the subtree.
 */
Point computePositionsLinear(pc_tree::PCNode *node, double left, double top,
                              Layout &positions, double levelHeight,
                              double leafWidth);

inline Point computePositionsLinear(pc_tree::PCTree &tree, Layout &positions,
                              double levelHeight, double leafWidth, double left=0, double top=0) {
  return computePositionsLinear(tree.getRootNode(), left, top, positions,
                                levelHeight, leafWidth);
}

void gaussianElimination(std::vector<std::vector<double>> &augmentedMatrix,
                         std::vector<double> &result);

/**
 * Computes all node positions using Tutte's method with all leaves positioned
 * on a circle of the given radius.
 */
void computePositionsCircular(pc_tree::PCTree &tree, Layout &positions,
                              double radius);
