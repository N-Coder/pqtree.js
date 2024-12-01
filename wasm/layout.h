#pragma once

#include <pctree/PCTree.h>

#include <vector>

using Point = std::tuple<double, double>;
using Layout = pc_tree::PCTreeNodeArray<Point>;

/**
 * Computes all node positions in the subtree starting at node, placed
 * left-top aligned to the given coordinates. Returns the size of the subtree.
 */
Point computePositionsLinear(pc_tree::PCNode* node, double left, double top, Layout& positions,
		pc_tree::PCTreeNodeArray<double>* subtree_widths, double levelHeight, double leafWidth);

inline Point computePositionsLinear(pc_tree::PCTree& tree, Layout& positions,
		pc_tree::PCTreeNodeArray<double>* subtree_widths, double levelHeight, double leafWidth,
		double left = 0, double top = 0) {
	return computePositionsLinear(tree.getRootNode(), left, top, positions, subtree_widths,
			levelHeight, leafWidth);
}

void gaussianElimination(std::vector<std::vector<double>>& augmentedMatrix,
		std::vector<double>& result);

double computeCircularWeight(pc_tree::PCTree& tree, pc_tree::PCTreeNodeArray<double>& weights,
		double inner_weight, double leaf_weight, std::function<double(double, double)> fold);

inline double computeCircularWeightByHeight(pc_tree::PCTree& tree,
		pc_tree::PCTreeNodeArray<double>& weights) {
	return computeCircularWeight(tree, weights, 0, 1,
			[](double c, double n) { return std::max(c + 1, n); });
}

inline double computeCircularWeightBySubtreeSize(pc_tree::PCTree& tree,
		pc_tree::PCTreeNodeArray<double>& weights, double inner_weight = 1) {
	return computeCircularWeight(tree, weights, inner_weight, 1,
			[](double c, double n) { return c + n; });
}

/**
 * Computes all node positions using Tutte's method with all leaves
 * positioned on a circle of the given radius, optionally using edge weights.
 *
 * W. T. Tutte:
 * How to Draw a Graph. Proceedings of the London Mathematical Society 1963
 * https://doi.org/10.1112/plms/s3-13.1.743
 *
 * Alvin Chiu, David Eppstein, Michael T. Goodrich:
 * Manipulating Weights to Improve Stress-Graph Drawings of 3-Connected Planar
 * Graphs. GD 2023 https://doi.org/10.1007/978-3-031-49275-4_10
 */
void computePositionsCircular(pc_tree::PCTree& tree, Layout& positions, double radius,
		pc_tree::PCTreeNodeArray<double>* weights = nullptr, double off_x = 0, double off_y = 0);
