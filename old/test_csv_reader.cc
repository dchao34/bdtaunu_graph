#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <fstream>

#include "CsvReader.h"

using namespace std;

int main(){

  CsvReader<> csv;
  csv.next();
  csv.open("testing.csv");
  csv.next();
  cout << csv["mclen"] << endl;
  cout << csv["mclund"] << endl;
  csv.next();
  cout << csv["mclen"] << endl;
  cout << csv["mclund"] << endl;
  csv.close();

  cout << endl;
  cout << "break " << endl;
  cout << endl;

  csv.open("testing.csv");
  csv.next();
  cout << csv["mclen"] << endl;
  cout << csv["mclund"] << endl;
  csv.next();
  cout << csv["mclen"] << endl;
  cout << csv["mclund"] << endl;
  csv.close();

  cout << endl;
  cout << "break " << endl;
  cout << endl;
  
  csv.open("testing.csv");
  int n_records = 0;
  while (csv.next()) { ++n_records; }
  cout << "number of records: " << n_records << endl; 
  csv.close();

  return 0;
}

