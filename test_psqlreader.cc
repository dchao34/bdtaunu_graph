#include <iostream>
#include <string>

#include "PsqlReader.h"

int main() {
  
  PsqlReader psql;
  psql.open_connection("dbname=testing");
  psql.open_cursor(
      "framework_ntuples", 
      { "eid", "mclen", "mclund" }, 5000);

  int n_records = 0;
  while (psql.next()) {
    ++n_records;
    std::cout << psql.get("mclund") << std::endl;
  }
  std::cout << n_records << std::endl;

  psql.close_cursor();
  psql.close_connection();

  return 0;
}
