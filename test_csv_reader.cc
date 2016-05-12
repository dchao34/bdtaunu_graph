#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <fstream>

#include <boost/tokenizer.hpp>

using namespace std;
using namespace boost;

class CsvReader {
  public:

    CsvReader() = default;
    CsvReader(const string &fname) { open(fname); }

    ~CsvReader() { close(); }

    void open(const string &fname) {

      close();

      fin_.open(fname);
      getline(fin_, line_);

      typedef token_iterator_generator<escaped_list_separator<char>>::type Iter;
      Iter beg = make_token_iterator<string>(line_.begin(), line_.end(), sep_);
      Iter end = make_token_iterator<string>(line_.end(), line_.end(), sep_); 

      for (size_t idx = 0; beg != end; ++beg, ++idx) { 
        colname_idx_[*beg] = idx;
        cache_.push_back("");
      }

    }

    void close() { 
      fin_.close(); 
      colname_idx_.clear(); 
      cache_.clear(); 
    }

    bool next() {
      if (!fin_.good()) { return false; }

      getline(fin_, line_);

      typedef token_iterator_generator<escaped_list_separator<char>>::type Iter;
      Iter beg = make_token_iterator<string>(line_.begin(), line_.end(), sep_);
      Iter end = make_token_iterator<string>(line_.end(), line_.end(), sep_); 

      for (size_t i = 0; beg != end; ++beg, ++i) { cache_[i] = *beg;}

      return true;
    }

    string& operator[](const string &key) { 
      return cache_[colname_idx_.at(key)];
    }

  private:
    ifstream fin_;
    string line_;

    escaped_list_separator<char> sep_;

    unordered_map<string, size_t> colname_idx_;
    vector<string> cache_;
};

int main(){

  CsvReader csv;
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

  return 0;
}

