#include <iostream>
#include <string>
#include <vector>
#include "pgstring_convert.h"

using namespace std;
using namespace boost;

int main(){

  string s = "{11,-11,70553,-511,-511,413,-12,11,411,15,-16,421,211,22,22,22,211,211,-321,-20213,16,111,211,-321,211,1022,211,2112,-213,111,22,22,211,22,111,-211,22,22,22,22,13,-14}";
  
  vector<int> s_int;
  pgstring_convert(s, s_int);
  for (const auto &e : s_int) {
    cout << e << " ";
  }
  cout << endl;

  s = "{11.1,-11.3,70553,-511,-511,413,-12,11,411,15,-16,421,211,22,22,22,211,211,-321,-20213,16,111,211,-321,211,1022,211,2112,-213,111,22,22,211,22,111,-211,22,22,22,22,13,-14.321}";
  
  vector<float> s_float;
  pgstring_convert(s, s_float);
  for (const auto &e : s_float) {
    cout << e << " ";
  }
  cout << endl;

  vector<double> s_double;
  pgstring_convert(s, s_double);
  for (const auto &e : s_double) {
    cout << e << " ";
  }
  cout << endl;

  return 0;
}

