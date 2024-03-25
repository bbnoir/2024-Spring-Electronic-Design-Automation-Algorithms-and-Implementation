#pragma once
#include "include.hpp"
#include "SOP.hpp"

using namespace std;

struct BDDNode
{
  char var;
  BDDNode *high, *low;

  BDDNode(char var, BDDNode *low, BDDNode *high) : var(var), low(low), high(high) {}
};

class DDManager
{
public:
  string var_order;
  unordered_map<string, BDDNode *> table;
  
  DDManager(string var_order) : var_order(var_order) {}

  BDDNode* old_or_new(char var, BDDNode *low, BDDNode *high)
  {
    string key;
    if (var == '0' || var == '1')
      key = var;
    else
      key = to_string(var) + to_string(low->var) + to_string(high->var);
    if (table.find(key) != table.end())
      return table[key];
    return table[key] = new BDDNode(var, low, high);
  }

  BDDNode* robdd_build(SOP sop, int idx)
  {
    if (sop.is_one())
      return old_or_new('1', nullptr, nullptr);
    else if (sop.is_zero())
      return old_or_new('0', nullptr, nullptr);
    else
    {
      BDDNode *high = robdd_build(sop.pos_cofactor(var_order[idx]), idx + 1);
      BDDNode *low = robdd_build(sop.neg_cofactor(var_order[idx]), idx + 1);
      if (high == low)
        return high;
      else
        return old_or_new(var_order[idx], low, high);
    }
  }


  int count_nodes()
  {
    return table.size();
  }
};