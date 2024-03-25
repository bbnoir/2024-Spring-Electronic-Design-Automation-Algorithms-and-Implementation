#pragma once
#include "include.hpp"

using namespace std;

class Product
{
public:
  string product;

  Product() : product("") {}
  Product(string product) : product(product) {}

  bool is_one()
  {
    return product == "1";
  }

  bool is_zero()
  {
    return product == "0";
  }

  void pos_cofactor(const char &var)
  {
    if (is_zero() || is_one())
      return;
    if (product.find(toupper(var)) != string::npos)
      product = "0";
    else if (product.find(var) != string::npos)
      product.erase(product.find(var), 1);
      if (product.empty())
        product = "1";
  }

  void neg_cofactor(const char &var)
  {
    if (is_zero() || is_one())
      return;
    if (product.find(var) != string::npos)
      product = "0";
    else if (product.find(toupper(var)) != string::npos)
      product.erase(product.find(toupper(var)), 1);
      if (product.empty())
        product = "1";
  }
};

class SOP
{
public:
  vector<Product> sop;

  SOP(vector<Product> sop) : sop(sop) {}
  SOP(string expression)
  {
    string product = "";
    for (char c : expression)
    {
      if (c == '.')
      {
        sop.emplace_back(Product(product));
        break;
      }
      else if (c == '+')
      {
        sop.push_back(Product(product));
        product = "";
      }
      else
      {
        product += c;
      }
    }
  }

  bool is_one()
  {
    return sop[0].is_one();
  }

  bool is_zero()
  {
    return sop[0].is_zero();
  }

  SOP pos_cofactor(const char &var)
  {
    vector<Product> new_sop;
    for (Product p : sop)
    {
      Product new_p = p;
      new_p.pos_cofactor(var);
      if (new_p.is_one())
      {
        new_sop.clear();
        new_sop.emplace_back(new_p);
        break;
      }
      if (!new_p.is_zero())
        new_sop.emplace_back(new_p);
    }
    if (new_sop.empty())
      new_sop.emplace_back("0");
    return SOP(new_sop);
  }

  SOP neg_cofactor(const char &var)
  {
    vector<Product> new_sop;
    for (Product p : sop)
    {
      Product new_p = p;
      new_p.neg_cofactor(var);
      if (new_p.is_one())
      {
        new_sop.clear();
        new_sop.emplace_back(new_p);
        break;
      }
      if (!new_p.is_zero())
        new_sop.emplace_back(new_p);
    }
    if (new_sop.empty())
      new_sop.emplace_back("0");
    return SOP(new_sop);
  }
};