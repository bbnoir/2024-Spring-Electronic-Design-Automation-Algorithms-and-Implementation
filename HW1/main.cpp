#include "include.hpp"
#include "SOP.hpp"
#include "DDManager.hpp"

using namespace std;

int robdd(SOP sop, string var_order)
{
  DDManager *ddm = new DDManager(var_order);
  ddm->robdd_build(sop, 0);
  int nodes = ddm->count_nodes();
  delete ddm;
  return nodes;
}

int main(int argc, char *argv[]) {
  if (argc != 3)
    assert("Usage: ./bdd <input_file> <output_file>");
  ifstream in_file(argv[1]);
  string expression;
  getline(in_file, expression);
  SOP sop(expression);
  int min_nodes = INT32_MAX;
  string var_order;
  while(getline(in_file, var_order)) {
    int nodes = robdd(sop, var_order);
    min_nodes = min(min_nodes, nodes);
  }
  in_file.close();
  ofstream out_file(argv[2]);
  out_file << min_nodes << endl;
  out_file.close();
  return 0;
}
