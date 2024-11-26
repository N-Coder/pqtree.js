#include "layout.h"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace std;
using namespace pc_tree;

Point computePositionsLinear(PCNode *node, double left, double top,
                             Layout &positions, double levelHeight,
                             double leafWidth) {
  if (node->isLeaf()) {
    if (node->isDetached()) { // root
      auto [child_width, child_heigh] =
          computePositionsLinear(node->getOnlyChild(), left, top + levelHeight,
                                 positions, levelHeight, leafWidth);
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
    auto [child_width, child_heigh] =
        computePositionsLinear(child, left + width, top + levelHeight,
                               positions, levelHeight, leafWidth);
    width += child_width;
    height = max(height, child_heigh);
  }
  positions[node] = {left + width / 2, top};
  cout << node->index() << " " << width << ' ' << height << endl;
  return {width, height + levelHeight};
}

void computePositionsCircular(PCTree &tree, Layout &positions, double radius) {
  PCTreeNodeArray<int> indices(tree);
  size_t last_col = tree.getNodeCount();
  vector<vector<double>> matrix_x(tree.getNodeCount(),
                                  vector<double>(last_col + 1));
  vector<vector<double>> matrix_y(tree.getNodeCount(),
                                  vector<double>(last_col + 1));
  int cnt = tree.getLeafCount();
  for (auto node : tree.innerNodes()) {
    indices[node] = cnt;
    matrix_x[cnt][cnt] = node->getDegree() * -1.0;
    matrix_y[cnt][cnt] = node->getDegree() * -1.0;
    if (node != tree.getRootNode()) {
      int parent_idx = indices[node->getParent()];
      matrix_x[parent_idx][cnt] = 1;
      matrix_x[cnt][parent_idx] = 1;
      matrix_y[parent_idx][cnt] = 1;
      matrix_y[cnt][parent_idx] = 1;
    }
    cnt++;
  }
  assert(cnt == tree.getNodeCount());

  cnt = 0;
  double angle = (2 * M_PI) / tree.getLeafCount();
  for (auto leaf : tree.getLeaves()) {
    double a = angle * cnt;
    // positions[leaf] = {sin(a) * radius, cos(a) * radius};
    indices[leaf] = cnt;
    matrix_x[cnt][cnt] = 1;
    matrix_y[cnt][cnt] = 1;
    matrix_x[cnt][last_col] = sin(a) * radius; // get<0>(positions[leaf]);
    matrix_y[cnt][last_col] = cos(a) * radius; // get<1>(positions[leaf]);
    int parent_idx = indices[leaf->getParent()];
    matrix_x[parent_idx][cnt] = 1;
    matrix_x[cnt][parent_idx] = 1;
    matrix_y[parent_idx][cnt] = 1;
    matrix_y[cnt][parent_idx] = 1;
    cnt++;
  }
  assert(cnt == tree.getLeafCount());

  vector<double> result_x;
  vector<double> result_y;

  for (int row = 0; row < tree.getNodeCount(); row++) {
    for (int col = 0; col < tree.getNodeCount(); col++) {
      cout << matrix_x[row][col] << " ";
    }
    cout << "= " << matrix_x[row][last_col] << " -> " << result_x[row] << endl;
  }
  cout << endl;

  for (int row = 0; row < tree.getNodeCount(); row++) {
    for (int col = 0; col < tree.getNodeCount(); col++) {
      cout << matrix_y[row][col] << " ";
    }
    cout << "= " << matrix_y[row][last_col] << " -> " << result_y[row] << endl;
  }
  cout << endl;

  gaussianElimination(matrix_x, result_x);
  gaussianElimination(matrix_y, result_y);

  for (int row = 0; row < tree.getNodeCount(); row++) {
    for (int col = 0; col < tree.getNodeCount(); col++) {
      cout << matrix_x[row][col] << " ";
    }
    cout << "= " << matrix_x[row][last_col] << " -> " << result_x[row] << endl;
  }
  cout << endl;

  for (int row = 0; row < tree.getNodeCount(); row++) {
    for (int col = 0; col < tree.getNodeCount(); col++) {
      cout << matrix_y[row][col] << " ";
    }
    cout << "= " << matrix_y[row][last_col] << " -> " << result_y[row] << endl;
  }
  cout << endl;

  cout << tree << endl;
  for (auto node : tree.allNodes()) {
    size_t idx = indices[node];
    positions[node] = {result_x[idx], result_y[idx]};
    cout << (node->isLeaf() ? "L" : "N") << " " << node->index() << "@" << idx
         << ": " << result_x[idx] << ", " << result_y[idx] << endl;
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
