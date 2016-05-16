#ifndef _PSQL_READER_H_
#define _PSQL_READER_H_

#include <string>
#include <vector>
#include <unordered_map>

#include <libpq-fe.h>

class PsqlReader {
  public: 

    PsqlReader();
    ~PsqlReader();

    void open_connection(const std::string &conninfo);
    void close_connection();

    void open_cursor(const std::string &table_name, 
                     const std::vector<std::string> &colnames,
                     size_t max_rows, 
                     const std::string &cursor_name = "myportal");

    void close_cursor();

    bool next();

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
