#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <utility>
#include <unordered_map>
#include <initializer_list>
#include <map>
#include <stdexcept>

#include "pgstring_convert.h"
#include "CsvReader.h"

using namespace std;
using namespace boost;

class RecoIndexer {

  public:
    RecoIndexer(const std::vector<std::string> &block_names) 
      : block_sizes_(block_names.size()), 
        start_indices_(block_names.size()) {
      for (int i = 0; i < block_names.size(); ++i) { 
        name2block_[block_names[i]] = i;
      }
    }

    void set_block_sizes(const std::vector<int> &block_sizes) {

      if (block_sizes.size() != block_sizes_.size()) { 
        throw std::length_error(
            "RecoIndexer::set_block_size(): block size mismatch. ");
      }

      block_sizes_ = block_sizes;
      for (int i = 1; i < block_sizes.size(); ++i) {
        start_indices_[i] = start_indices_[i-1] + block_sizes[i-1];
      }

    }

    int get_start_index(const std::string &block_name) {
      return start_indices_[name2block_[block_name]];
    }

    int get_block_size(const std::string &block_name) {
      return block_sizes_[name2block_[block_name]];
    }

    int get_index(const std::string &block_name, int idx) {
      int block_idx = name2block_[block_name];
      if (idx >= block_sizes_[block_idx]) {
        throw std::out_of_range(
            "RecoIndexer::get_index(): index exceeded maximum. ");
      }
      return start_indices_[block_idx] + idx;
    }

  private:
    std::unordered_map<std::string, int> name2block_;
    std::vector<int> block_sizes_;
    std::vector<int> start_indices_;

};

int main() {

  RecoIndexer reco_indexer({"y", "b", "d", "c", "h", "l", "gamma"});

  // read the csv file
  int ny, nb, nd, nc, nh, nl, ngamma;

  CsvReader<> csv("recograph.csv"); 

  int n_records = 0;
  while (csv.next() && n_records < 2) {
    ++n_records;

    pgstring_convert(csv["ny"], ny);
    pgstring_convert(csv["nb"], nb);
    pgstring_convert(csv["nd"], nd);
    pgstring_convert(csv["nc"], nc);
    pgstring_convert(csv["nh"], nh);
    pgstring_convert(csv["nl"], nl);
    pgstring_convert(csv["ngamma"], ngamma);

    std::cout << ny << " ";
    std::cout << nb << " ";
    std::cout << nd << " ";
    std::cout << nc << " ";
    std::cout << nh << " ";
    std::cout << nl << " ";
    std::cout << ngamma << " ";
    std::cout << std::endl;

    reco_indexer.set_block_sizes({ny, nb, nd, nc, nh, nl, ngamma});

    std::cout << reco_indexer.get_start_index("y") << " ";
    std::cout << reco_indexer.get_start_index("b") << " ";
    std::cout << reco_indexer.get_start_index("d") << " ";
    std::cout << reco_indexer.get_start_index("c") << " ";
    std::cout << reco_indexer.get_start_index("h") << " ";
    std::cout << reco_indexer.get_start_index("l") << " ";
    std::cout << reco_indexer.get_start_index("gamma") << " ";
    std::cout << std::endl;

    std::cout << reco_indexer.get_block_size("y") << " ";
    std::cout << reco_indexer.get_block_size("b") << " ";
    std::cout << reco_indexer.get_block_size("d") << " ";
    std::cout << reco_indexer.get_block_size("c") << " ";
    std::cout << reco_indexer.get_block_size("h") << " ";
    std::cout << reco_indexer.get_block_size("l") << " ";
    std::cout << reco_indexer.get_block_size("gamma") << " ";
    std::cout << std::endl;

    std::cout << reco_indexer.get_index("y", 0) << " ";
    std::cout << reco_indexer.get_index("b", 0) << " ";
    std::cout << reco_indexer.get_index("d", 0) << " ";
    std::cout << reco_indexer.get_index("c", 0) << " ";
    std::cout << reco_indexer.get_index("h", 0) << " ";
    std::cout << reco_indexer.get_index("l", 0) << " ";
    std::cout << reco_indexer.get_index("gamma", 0) << " ";
    std::cout << std::endl;
    std::cout << std::endl;
  }

  return 0;
}
