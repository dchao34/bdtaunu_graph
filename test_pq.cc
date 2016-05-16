#include <iostream>
#include <string>

#include <libpq-fe.h>

int main() {
  PGconn *conn = PQconnectdb("dbname=testing");

  std::string query_stmt = 
    "SELECT mclen, mclund FROM framework_ntuples;";

  std::string s;

  PGresult *res = PQexec(conn, query_stmt.c_str());
  for (size_t i = 0; i < PQntuples(res); i++) {
    for (size_t j = 0; j < PQnfields(res); j++) {
      s = PQgetvalue(res, i, j);
      std::cout << s << " ";
      //printf("%-15s", PQgetvalue(res, i, j));
    }
    std::cout << std::endl;
  }
  PQclear(res);

  PQfinish(conn);
  return 0;
}
