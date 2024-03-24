#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include "cudd.h"

using namespace std;

DdManager *ddm = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);

int build_robdd(string bool_func, string var_order)
{
  DdNode *f = Cudd_ReadLogicZero(ddm);
  Cudd_Ref(f);
  int n_vars = var_order.length();
  vector<DdNode *> vars(n_vars);
  for (int i = 0; i < n_vars; i++)
  {
    vars[i] = Cudd_bddIthVar(ddm, var_order[i] - 'a');
    Cudd_Ref(vars[i]);
  }
  DdNode *one = Cudd_ReadOne(ddm);
  Cudd_Ref(one);
  for (int i = 0; i < bool_func.length(); i++)
  {
    DdNode *tmp = one;
    while (bool_func[i] != '+' && i < bool_func.length())
    {
      char c = bool_func[i];
      DdNode *tmp_var;
      if (isupper(c))
        tmp_var = Cudd_Not(vars[c - 'A']);
      else
        tmp_var = vars[c - 'a'];
      tmp = Cudd_bddAnd(ddm, tmp, tmp_var);
      Cudd_Ref(tmp);
      i++;
    }
    f = Cudd_bddOr(ddm, f, tmp);
    Cudd_Ref(f);
    Cudd_RecursiveDeref(ddm, tmp);
  }
  Cudd_RecursiveDeref(ddm, one);
  for (int i = 0; i < n_vars; i++)
    Cudd_RecursiveDeref(ddm, vars[i]);
  f = Cudd_BddToAdd(ddm, f);
  return Cudd_DagSize(f);
}

int main(int argc, char *argv[]) {
  if (argc != 3)
  {
    std::cerr << "Usage: " << argv[0] << " <input file> <output file>" << std::endl;
    return 1;
  }
  ifstream in_file(argv[1]);
  string bool_func;
  in_file >> bool_func;
  bool_func.pop_back(); //remove period
  vector<string> var_order;
  string line;
  while (in_file >> line)
  {
    line.pop_back();
    var_order.push_back(line);
  }
  in_file.close();

  int min_nodes = INT32_MAX;

  for (int i = 0; i < var_order.size(); i++)
  {
    int num_nodes = build_robdd(bool_func, var_order[i]);
    min_nodes = min(min_nodes, num_nodes);
  }

  ofstream out_file(argv[2]);
  out_file << min_nodes;
  out_file.close();

  Cudd_Quit(ddm);
  return 0;
}
