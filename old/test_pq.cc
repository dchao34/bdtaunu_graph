#include <iostream>
#include <string>

#include <libpq-fe.h>

static void exit_nicely(PGconn *conn) {
  PQfinish(conn);
  exit(1);
}

int main() {

  PGconn *conn = PQconnectdb("dbname=testing");
  if (PQstatus(conn) != CONNECTION_OK) {
    std::cerr << "Connection to database failed: ";
    std::cerr << PQerrorMessage(conn) << std::endl;
    exit_nicely(conn);
  }

  PGresult *res = PQexec(conn, "BEGIN");
  PQclear(res);

  std::string select_stmt = "SELECT mclen, mclund FROM framework_ntuples;";

  res = PQexec(conn, 
      ("DECLARE myportal CURSOR FOR " + select_stmt).c_str());
  PQclear(res);

  res = PQexec(conn, "FETCH FORWARD 2 in myportal;");

  for (size_t i = 0; i < PQntuples(res); i++) {
    std::cout << PQgetvalue(res, i, 0) << std::endl;
  }

  PQclear(res);

  res = PQexec(conn, "FETCH FORWARD 2 in myportal;");

  for (size_t i = 0; i < PQntuples(res); i++) {
    std::cout << PQgetvalue(res, i, 0) << std::endl;
  }

  PQclear(res);

  res = PQexec(conn, "CLOSE myportal;");
  PQclear(res);

  res = PQexec(conn, "END");
  PQclear(res);

  PQfinish(conn);

  return 0;
}
