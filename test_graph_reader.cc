#include <iostream>
#include <vector>
#include <fstream>

#include "pgstring_convert.h"
#include "CsvReader.h"

template <typename T> 
void print(std::ostream &os, const std::vector<T> &v) {
  for (const auto &e : v) { os << e << " "; }
}

int main() {

  int mclen;
  std::vector<int> mclund, daulen, dauidx;

  CsvReader<> csv("testing.csv");

  size_t n_records = 0;
  while (csv.next() && n_records < 3) {

    pgstring_convert(csv["mclen"], mclen);
    pgstring_convert(csv["mclund"], mclund);
    pgstring_convert(csv["daulen"], daulen);
    pgstring_convert(csv["dauidx"], dauidx);

    std::cout << mclen << std::endl;
    print(std::cout, mclund); std::cout << std::endl;
    print(std::cout, daulen); std::cout << std::endl;
    print(std::cout, dauidx); std::cout << std::endl;

    std::cout << std::endl;

    ++n_records;
  }

  return 0;
}
