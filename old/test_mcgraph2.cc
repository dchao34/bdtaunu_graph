#include <iostream>
#include <vector>
#include <fstream>
#include <utility>
#include <unordered_map>
#include <map>

#include "pgstring_convert.h"
#include "CsvReader.h"

template <typename T> 
void print(std::ostream &os, const std::vector<T> &v) {
  for (const auto &e : v) { os << e << " "; }
}

int main() {

  int eid;
  int mclen;
  std::vector<int> mclund, daulen, dauidx;

  CsvReader<> csv("mcgraph.csv"); csv.next();
  pgstring_convert(csv["eid"], eid);
  pgstring_convert(csv["mclen"], mclen);
  pgstring_convert(csv["mclund"], mclund);
  pgstring_convert(csv["daulen"], daulen);
  pgstring_convert(csv["dauidx"], dauidx);

  std::vector<int> from_vertices, to_vertices;

  for (int i = 0; i < mclen; ++i) {
    if (daulen[i] <= 0 || dauidx[i] <= 0) { continue; }
    for (int j = dauidx[i]; j < dauidx[i]+daulen[i]; ++j) {
      from_vertices.push_back(i);
      to_vertices.push_back(j);
    }
  }

  print(std::cout, from_vertices); std::cout << std::endl;
  print(std::cout, to_vertices); std::cout << std::endl;

  return 0;
}
