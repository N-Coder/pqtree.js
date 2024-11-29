#include "layout.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace std;
using namespace pc_tree;

Point computePositionsLinear(PCNode *node, double left, double top,
                             Layout &positions,
                             PCTreeNodeArray<double> *subtree_widths,
                             double levelHeight, double leafWidth) {
  if (node->isLeaf()) {
    if (node->isDetached()) { // root
      auto [child_width, child_heigh] = computePositionsLinear(
          node->getOnlyChild(), left, top + levelHeight, positions,
          subtree_widths, levelHeight, leafWidth);
      positions[node] = {left + child_width / 2, top};
      return {child_width, child_heigh + levelHeight};
    } else { // ordinary leaf
      positions[node] = {left + leafWidth / 2, top};
      return {leafWidth, levelHeight};
    }
  }
  double width = 0;
  double height = 0;
  for (auto child : node->children()) {
    auto [child_width, child_heigh] = computePositionsLinear(
        child, left + width, top + levelHeight, positions, subtree_widths,
        levelHeight, leafWidth);
    width += child_width;
    height = max(height, child_heigh);
  }
  positions[node] = {left + width / 2, top};
  if (subtree_widths)
    (*subtree_widths)[node] = width;
  // cout << node->index() << " " << width << ' ' << height << endl;
  return {width, height + levelHeight};
}

double computeCircularWeight(PCTree &tree, PCTreeNodeArray<double> &weights,
                             double inner_weight, double leaf_weight,
                             function<double(double, double)> fold) {
  vector<PCNode *> nodes(tree.allNodes(), FilteringPCTreeDFS());
  std::reverse(
      nodes.begin(),
      nodes.end()); // TODO still goes towards root, not away from leaves
  double max_weight = 0;
  for (auto node : nodes) {
    if (node->isLeaf()) {
      weights[node] = leaf_weight;
    } else {
      double w = inner_weight;
      for (auto child : node->children()) {
        w = fold(w, weights[child]);
      }
      weights[node] = w;
      max_weight = max(max_weight, w);
    }
  }
  return max_weight;
}

void computePositionsCircular(PCTree &tree, Layout &positions, double radius,
                              PCTreeNodeArray<double> *weights, double off_x,
                              double off_y) {
  PCTreeNodeArray<int> indices(tree);
  size_t last_col = tree.getNodeCount();
  vector<vector<double>> matrix_x(tree.getNodeCount(),
                                  vector<double>(last_col + 1));
  vector<vector<double>> matrix_y(tree.getNodeCount(),
                                  vector<double>(last_col + 1));

  auto add_to_matrix = [&matrix_x, &matrix_y](size_t a, size_t b,
                                              double weight) {
    matrix_x[a][b] += weight;
    matrix_x[b][a] += weight;
    matrix_x[a][a] -= weight;
    matrix_x[b][b] -= weight;

    matrix_y[a][b] += weight;
    matrix_y[b][a] += weight;
    matrix_y[a][a] -= weight;
    matrix_y[b][b] -= weight;
  };

  auto cnt = tree.getLeafCount();
  for (auto node : tree.innerNodes()) {
    indices[node] = cnt;
    if (node != tree.getRootNode())
      add_to_matrix(cnt, indices[node->getParent()],
                    weights ? (*weights)[node] : 1);
    cnt++;
  }
  assert(cnt == tree.getNodeCount());

  cnt = 0;
  double angle = (2 * M_PI) / tree.getLeafCount();
  for (auto leaf : tree.currentLeafOrder()) {
    double a = angle * cnt;
    // positions[leaf] = {sin(a) * radius, cos(a) * radius};
    indices[leaf] = cnt;
    matrix_x[cnt][cnt] = 1;
    matrix_y[cnt][cnt] = 1;
    matrix_x[cnt][last_col] = sin(a) * radius; // get<0>(positions[leaf]);
    matrix_y[cnt][last_col] = cos(a) * radius; // get<1>(positions[leaf]);

    int parent_idx = indices[leaf->getParent()];
    double weight = weights ? (*weights)[leaf] : 1;
    matrix_x[parent_idx][cnt] += weight;
    matrix_x[parent_idx][parent_idx] -= weight;
    matrix_y[parent_idx][cnt] += weight;
    matrix_y[parent_idx][parent_idx] -= weight;

    cnt++;
  }
  assert(cnt == tree.getLeafCount());

  vector<double> result_x;
  vector<double> result_y;

  // auto dump_result = [&tree, last_col](vector<vector<double>> &matrix,
  //                                      vector<double> &result) {
  //   for (int row = 0; row < tree.getNodeCount(); row++) {
  //     for (int col = 0; col < tree.getNodeCount(); col++) {
  //       cout << matrix[row][col] << " ";
  //     }
  //     cout << "= " << matrix[row][last_col] << " -> " << result[row] << "\n";
  //   }
  //   cout << endl;
  // };

  // dump_result(matrix_x, result_x);
  // dump_result(matrix_y, result_y);
  gaussianElimination(matrix_x, result_x);
  gaussianElimination(matrix_y, result_y);
  // dump_result(matrix_x, result_x);
  // dump_result(matrix_y, result_y);

  // cout << tree << endl;
  for (auto node : tree.allNodes()) {
    size_t idx = indices[node];
    positions[node] = {result_x[idx] + off_x, result_y[idx] + off_y};
    // cout << (node->isLeaf() ? "L" : "N") << " " << node->index() << "@" <<
    // idx
    //      << ": " << result_x[idx] << ", " << result_y[idx] << endl;
  }
}

void gaussianElimination(vector<vector<double>> &augmentedMatrix,
                         vector<double> &result) {
  int n = augmentedMatrix.size();

  // Forward elimination
  for (int i = 0; i < n; ++i) {
    // Find the maximum element in the current column
    int maxRow = i;
    for (int k = i + 1; k < n; ++k) {
      if (fabs(augmentedMatrix[k][i]) > fabs(augmentedMatrix[maxRow][i])) {
        maxRow = k;
      }
    }

    // Swap the rows
    swap(augmentedMatrix[i], augmentedMatrix[maxRow]);

    // Make all rows below the current one 0 in the current column
    for (int k = i + 1; k < n; ++k) {
      double factor = augmentedMatrix[k][i] / augmentedMatrix[i][i];
      augmentedMatrix[k][i] = 0;         // The current column becomes 0
      for (int j = i + 1; j <= n; ++j) { // Include the augmented column
        augmentedMatrix[k][j] -= factor * augmentedMatrix[i][j];
      }
    }
  }

  // Back substitution
  result.assign(n, 0);
  for (int i = n - 1; i >= 0; --i) {
    result[i] = augmentedMatrix[i][n]; // Start with the constant term
    for (int j = i + 1; j < n; ++j) {
      result[i] -= augmentedMatrix[i][j] * result[j];
    }
    result[i] /= augmentedMatrix[i][i];
  }
}
