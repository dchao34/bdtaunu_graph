#include <stdexcept>

#include "PsqlReader.h"

PsqlReader::PsqlReader() 
  : conn_(nullptr), res_(nullptr), qres_(nullptr) {};

PsqlReader::~PsqlReader() {
  reset_pgconn(&conn_);
  reset_pgresult(&res_);
  reset_pgresult(&qres_);
}

void PsqlReader::open_connection(const std::string &conninfo) {
  conn_ = PQconnectdb(conninfo.c_str());
  if (PQstatus(conn_) != CONNECTION_OK) {
    throw std::runtime_error(
      std::string("Connection to database failed: ") + PQerrorMessage(conn_));
  }
}

void PsqlReader::open_cursor(
    const std::string &table_name, 
    const std::vector<std::string> &colnames,
    size_t max_rows, 
    const std::string &cursor_name) {

  if (colnames.size() == 0) { 
    throw std::invalid_argument(
        "PsqlReader::open_cursor(): you must select at least 1 column. ");
  }

  if (max_rows == 0) { 
    throw std::invalid_argument(
        "PsqlReader::open_cursor(): you must load at least 1 row. ");
  }

  cursor_name_ = cursor_name;

  // open a transaction. required for cursors. 
  res_ = PQexec(conn_, "BEGIN");
  if (PQresultStatus(res_) != PGRES_COMMAND_OK) {
    throw std::runtime_error(
        std::string("BEGIN command failed: ") + PQerrorMessage(conn_));
  }
  reset_pgresult(&res_);


  // assemble query statement
  std::string query_stmt;
  for (const auto &col : colnames) { query_stmt += col + ","; }
  query_stmt.pop_back();
  query_stmt = "SELECT " + query_stmt + " FROM " + table_name;

  // declare cursor
  res_ = PQexec(conn_,
     ("DECLARE " + cursor_name_ + " CURSOR FOR " + query_stmt).c_str());
  if (PQresultStatus(res_) != PGRES_COMMAND_OK) {
    throw std::runtime_error(
        std::string("DECLARE CURSOR failed: ") + PQerrorMessage(conn_));
  }
  reset_pgresult(&res_);

  // initialize the column map and caches
  for (size_t i = 0; i < colnames.size(); ++i) {
    name2idx_[colnames[i]] = i;
  }
  cache_ = std::vector<std::string>(colnames.size());

  // initialize buffer to empty. first call to next() will replenish. 
  max_rows_ = max_rows;
  curr_max_ = max_rows_;
  curr_idx_ = curr_max_;

}

void PsqlReader::close_cursor() {

  // clear query result set
  reset_pgresult(&qres_);

  // close cursor
  res_ = PQexec(conn_, ("CLOSE " + cursor_name_).c_str());
  if (PQresultStatus(res_) != PGRES_COMMAND_OK) {
    throw std::runtime_error(
        std::string("CLOSE cursor failed: ") + PQerrorMessage(conn_));
  }
  reset_pgresult(&res_);

  // close transaction
  res_ = PQexec(conn_, "END");
  if (PQresultStatus(res_) != PGRES_COMMAND_OK) {
    throw std::runtime_error(
        std::string("END command failed: ") + PQerrorMessage(conn_));
  }
  reset_pgresult(&res_);

}

bool PsqlReader::next() {

  // non-empty buffer
  if (curr_idx_ != curr_max_) {

    // copy columns contents into the cache
    for (size_t i = 0; i < cache_.size(); ++i) {
      cache_[i] = PQgetvalue(qres_, curr_idx_, i);
    }

    ++curr_idx_;

    return true;

  // empty buffer. replenish and try again. 
  } else {

    reset_pgresult(&qres_);

    // empty store
    if (curr_max_ != max_rows_) {  return false; }

    // fetch records from store and set the buffer state to 
    // indicate the (possible) availability of new records 
    qres_ = PQexec(conn_, ("FETCH FORWARD " 
         + std::to_string(max_rows_) + " in myportal").c_str());
    curr_max_ = PQntuples(qres_);
    curr_idx_ = 0;

    return next();

  }
}

