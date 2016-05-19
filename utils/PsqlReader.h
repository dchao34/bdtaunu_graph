#ifndef _PSQL_READER_H_
#define _PSQL_READER_H_

#include <string>
#include <vector>
#include <unordered_map>

#include <libpq-fe.h>

// class that reads a set of columns from a table in 
// some database and delivers it memory. 
// i.e. performs 'SELECT col1,...,colN FROM table_name'
class PsqlReader {
  public: 

    PsqlReader();
    ~PsqlReader();

    // open a connection to the database. `conninfo` is the same 
    // format as that expected in PQconnect in libpq. 
    void open_connection(const std::string &conninfo);

    // close the database connecion. 
    void close_connection();

    // open a cursor to read from a table in the current database connection. 
    // + table_name: table to read from.
    // + colnames: vector of column names to read. 
    // + max_rows: maximum number of rows per fetch. 
    // + cursor_name: name of the cursor. 
    void open_cursor(const std::string &table_name, 
                     const std::vector<std::string> &colnames,
                     size_t max_rows=10000, 
                     const std::string &cursor_name = "myportal");

    // close the cursor
    void close_cursor();

    // fetch the next available row. returns false if no new 
    // rows were able to be fetched. 
    bool next();

    // extract the contents in text form in the column `colname`. 
    const std::string& get(const std::string &colname) const;

  private:
    void reset_pgresult(PGresult **res);
    void reset_pgconn(PGconn **conn);

  private:
    PGconn *conn_;
    PGresult *res_;
    PGresult *qres_;

    std::string cursor_name_;

    size_t curr_idx_, curr_max_;
    size_t max_rows_;

    std::unordered_map<std::string, size_t> name2idx_;
    std::vector<std::string> cache_;
};


inline void PsqlReader::close_connection() { 
  reset_pgconn(&conn_); 
}

inline const std::string& PsqlReader::get(const std::string &colname) const {
  return cache_[name2idx_.at(colname)];
}

// use this to reset PGconn* objects
inline void PsqlReader::reset_pgconn(PGconn **conn) {
  if (*conn) { PQfinish(*conn); }
  *conn = nullptr;
}

// use this to reset PGresult* objects
inline void PsqlReader::reset_pgresult(PGresult **res) {
  if (*res) { PQclear(*res); }
  *res = nullptr;
}

#endif
