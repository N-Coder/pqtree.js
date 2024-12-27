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

int main(int argc, char** argv) {
	if (argc < 2) {
		cerr << "Usage: " << argv[0] << " <spec> [is_circular]" << endl;
		return 2;
	}
	string spec = argv[1];
	bool is_circular = (argc == 3);
	if (argc > 3) {
		cerr << "Usage: " << argv[0] << " <spec> [is_circular]" << endl;
		return 2;
	}

	unique_ptr<PCTree> tree;
	try {
		tree = make_unique<PCTree>(spec, true);
	} catch (const invalid_argument& e) {
		cerr << "!" << e.what() << endl;
		return 1;
	}
	cout << tree << endl;
	if (!is_circular) {
		tree->changeRoot(tree->newNode(PCNodeType::Leaf, tree->getRootNode()));
		cout << tree << endl;
	}
	cout << endl;
	vector<vector<PCNode*>> restr;
	tree->getRestrictions(restr, is_circular ? nullptr : tree->getRootNode());

	vector<PCNode*> leaves(tree->getLeaves().begin(), tree->getLeaves().end());
	if (!is_circular) {
		leaves.erase(std::remove(leaves.begin(), leaves.end(), tree->getRootNode()), leaves.end());
	}
	sort(leaves.begin(), leaves.end(), [](PCNode* a, PCNode* b) { return a->index() < b->index(); });

	PCTreeNodeSet<> cons(*tree);
	for (const auto& row : restr) {
		cons.clear();
		for (auto leaf : row) {
			cons.insert(leaf);
		}
		for (auto leaf : leaves) {
			cout << (cons.isMember(leaf) ? "1" : "0");
		}
		cout << endl;
	}
	cout << endl;

	if (is_circular) {
		tree->uniqueID(cout, uid_utils::leafToID, uid_utils::compareNodesByID) << endl;
	} else {
		tree->uniqueIDRooted(
				cout,
				[](std::ostream& os, PCNode* n, int pos) {
					if (n->isLeaf() && n->getParent() != nullptr) {
						os << n->index();
					}
				},
				uid_utils::compareNodesByID, false)
				<< endl;
	}

	return 0;
}
