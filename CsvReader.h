#ifndef __CSV_READER_H__
#define __CSV_READER_H__

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <cassert>

#include <boost/tokenizer.hpp>

// class that reads the contents of a csv file. 
template <typename TokenizerFunction=boost::escaped_list_separator<char>>
class CsvReader {

  private:

    using TokenIterator = 
      typename boost::token_iterator_generator<TokenizerFunction>::type;

  public:

    // constructors
    // (1): assigns only the separator object. 
    // (2): does (1) and open()'s a file.
    CsvReader(TokenizerFunction sep = TokenizerFunction()) : sep_(sep) {};
    CsvReader(const std::string &fname, 
              TokenizerFunction sep = TokenizerFunction()) 
      : CsvReader<TokenizerFunction>(sep) { open(fname); }

    // open csv file for reading. 
    void open(const std::string &fname);

    // close currently open file.
    void close();

    // read in the next record. returns true if a next record is available. 
    bool next();

    // access the most recently read entry corresponding to column `key`.
    std::string& operator[](const std::string &key) { 
      return cache_[colname_idx_.at(key)];
    }

  private:
    TokenizerFunction sep_;

    std::ifstream fin_;
    std::string line_;

    std::unordered_map<std::string, size_t> colname_idx_;
    std::vector<std::string> cache_;
};

#include "CsvReaderImpl.h"

#endif
