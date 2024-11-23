#include <pctree/PCTree.h>
#include <string>

using namespace std;
using namespace pc_tree;
extern "C" {

char *Process(char *input) {
  // get prefix of input until ";"
  string prefix = "";
  int i = 0;
  while (input[i] != ';') {
    prefix += input[i];
    i++;
  }
  i++; // skip the semicolon
  int U = atoi(prefix.c_str());
  std::vector<PCNode *> leaves;
  PCTree tree(U + 1, &leaves);
  bool valid = true;
  vector<PCNode *> s;
  while (valid && i < strlen(input)) {
    s.clear();
    for (int j = 0; j <= U; j++) {
      if (input[i + j] == '1') {
        s.push_back(leaves[j]);
      }
    }
    i += U + 1;
    valid = valid && tree.makeConsecutive(s.begin(), s.end());
  }
  string str;
  if (valid) {
    std::stringstream ss;
    ss << tree;
    str = ss.str();
    // std::replace(str.begin(), str.end(), ':', ' ');
    // std::replace(str.begin(), str.end(), ',', ' ');
  } else {
    str = "Impossible";
  }
  char *cstr = new char[str.length() + 1];
  strcpy(cstr, str.c_str());
  return cstr;
}
}
