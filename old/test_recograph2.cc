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

std::unordered_map<int, std::string> create_block_map() {
  std::unordered_map<int, std::string> block_map;
  block_map.insert({70553, "y"});
  block_map.insert({521, "b"});
  block_map.insert({-521, "b"});
  block_map.insert({511, "b"});
  block_map.insert({-511, "b"});
  block_map.insert({413, "d"});
  block_map.insert({-413, "d"});
  block_map.insert({423, "d"});
  block_map.insert({-423, "d"});
  block_map.insert({421, "d"});
  block_map.insert({-421, "d"});
  block_map.insert({411, "d"});
  block_map.insert({-411, "d"});
  block_map.insert({310, "c"});
  block_map.insert({213, "c"});
  block_map.insert({-213, "c"});
  block_map.insert({111, "c"});
  block_map.insert({321, "h"});
  block_map.insert({-321, "h"});
  block_map.insert({211, "h"});
  block_map.insert({-211, "h"});
  block_map.insert({11, "l"});
  block_map.insert({-11, "l"});
  block_map.insert({13, "l"});
  block_map.insert({-13, "l"});
  block_map.insert({22, "gamma"});
  return block_map;
}



class DaughterIndexer {

  public: 
    DaughterIndexer(int max_daughters, 
                    const std::unordered_map<int, std::string> &lund2block) 
      : max_daus_(max_daughters),
        lund2block_(lund2block),
        dau_lund_(max_daughters),
        dau_idx_(max_daughters) {}

    void set_daughters(int n_daus, 
                       const std::vector<int> &lund,
                       const std::vector<int> &idx) {
      if (n_daus > max_daus_) {
        throw std::overflow_error(
            "DaughterIndexer::set_daughters(): exceeded maximum daughters. ");
      }

      if (lund.size() != max_daus_ || idx.size() != max_daus_) {
        throw std::length_error(
            "DaughterIndexer::set_daughters(): daughter vector length mismatch. ");
      }

      n_daus_ = n_daus;
      dau_lund_ = lund;
      dau_idx_ = idx;
    }

    int get_n_daus() const { return n_daus_; }

    std::pair<std::string, int> get_daughter_block_index(int i) {

      if (i >= n_daus_) { 
        throw std::out_of_range(
            "DaughterIndexer::get_daughter_block_index(): "
            "query index exceeded total daughters. ");
      }

      return { lund2block_[dau_lund_[i]], dau_idx_[i] };

    }

  private:
    int max_daus_;
    std::unordered_map<int, std::string> lund2block_;

    int n_daus_;
    std::vector<int> dau_lund_;
    std::vector<int> dau_idx_;

};

int main() {

  RecoIndexer reco_indexer({"y", "b", "d", "c", "h", "l", "gamma"});

  std::unordered_map<int, std::string> lund2block = create_block_map();
  DaughterIndexer y_dau_indexer(2, lund2block);
  DaughterIndexer b_dau_indexer(4, lund2block);
  DaughterIndexer d_dau_indexer(5, lund2block);
  DaughterIndexer c_dau_indexer(2, lund2block);
  DaughterIndexer h_dau_indexer(2, lund2block);
  DaughterIndexer l_dau_indexer(3, lund2block);

  // read the csv file
  int ny, nb, nd, nc, nh, nl, ngamma;
  std::vector<int> yndaus, bndaus, dndaus, cndaus, hndaus, lndaus, gammandaus;
  std::vector<int> yd1lund, yd2lund;
  std::vector<int> yd1idx, yd2idx;
  std::vector<int> bd1lund, bd2lund, bd3lund, bd4lund;
  std::vector<int> bd1idx, bd2idx, bd3idx, bd4idx;
  std::vector<int> dd1lund, dd2lund, dd3lund, dd4lund, dd5lund;
  std::vector<int> dd1idx, dd2idx, dd3idx, dd4idx, dd5idx;
  std::vector<int> cd1lund, cd2lund;
  std::vector<int> cd1idx, cd2idx;
  std::vector<int> hd1lund, hd2lund;
  std::vector<int> hd1idx, hd2idx;
  std::vector<int> ld1lund, ld2lund, ld3lund;
  std::vector<int> ld1idx, ld2idx, ld3idx;


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

    pgstring_convert(csv["yndaus"], yndaus);
    pgstring_convert(csv["bndaus"], bndaus);
    pgstring_convert(csv["dndaus"], dndaus);
    pgstring_convert(csv["cndaus"], cndaus);
    pgstring_convert(csv["hndaus"], hndaus);
    pgstring_convert(csv["lndaus"], lndaus);
    pgstring_convert(csv["gammandaus"], gammandaus);

    pgstring_convert(csv["yd1lund"], yd1lund);
    pgstring_convert(csv["yd1idx"], yd1idx);
    pgstring_convert(csv["yd2lund"], yd2lund);
    pgstring_convert(csv["yd2idx"], yd2idx);

    pgstring_convert(csv["bd1lund"], bd1lund);
    pgstring_convert(csv["bd1idx"], bd1idx);
    pgstring_convert(csv["bd2lund"], bd2lund);
    pgstring_convert(csv["bd2idx"], bd2idx);
    pgstring_convert(csv["bd3lund"], bd3lund);
    pgstring_convert(csv["bd3idx"], bd3idx);
    pgstring_convert(csv["bd4lund"], bd4lund);
    pgstring_convert(csv["bd4idx"], bd4idx);

    pgstring_convert(csv["dd1lund"], dd1lund);
    pgstring_convert(csv["dd1idx"], dd1idx);
    pgstring_convert(csv["dd2lund"], dd2lund);
    pgstring_convert(csv["dd2idx"], dd2idx);
    pgstring_convert(csv["dd3lund"], dd3lund);
    pgstring_convert(csv["dd3idx"], dd3idx);
    pgstring_convert(csv["dd4lund"], dd4lund);
    pgstring_convert(csv["dd4idx"], dd4idx);
    pgstring_convert(csv["dd5lund"], dd5lund);
    pgstring_convert(csv["dd5idx"], dd5idx);

    pgstring_convert(csv["cd1lund"], cd1lund);
    pgstring_convert(csv["cd1idx"], cd1idx);
    pgstring_convert(csv["cd2lund"], cd2lund);
    pgstring_convert(csv["cd2idx"], cd2idx);

    pgstring_convert(csv["hd1lund"], hd1lund);
    pgstring_convert(csv["hd1idx"], hd1idx);
    pgstring_convert(csv["hd2lund"], hd2lund);
    pgstring_convert(csv["hd2idx"], hd2idx);

    pgstring_convert(csv["ld1lund"], ld1lund);
    pgstring_convert(csv["ld1idx"], ld1idx);
    pgstring_convert(csv["ld2lund"], ld2lund);
    pgstring_convert(csv["ld2idx"], ld2idx);
    pgstring_convert(csv["ld3lund"], ld3lund);
    pgstring_convert(csv["ld3idx"], ld3idx);

    std::string block; int idx;

    for (int i = 0; i < ny; ++i) {
      y_dau_indexer.set_daughters(yndaus[i], 
          {yd1lund[i], yd2lund[i]}, 
          {yd1idx[i], yd2idx[i]});

      for (int j = 0; j < yndaus[i]; ++j) {
        std::tie(block, idx) = y_dau_indexer.get_daughter_block_index(j);
        std::cout << "(" << block << ", " << idx << ") ";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;

    for (int i = 0; i < nb; ++i) {
      b_dau_indexer.set_daughters(bndaus[i], 
          {bd1lund[i], bd2lund[i], bd3lund[i], bd4lund[i]}, 
          {bd1idx[i], bd2idx[i], bd3idx[i], bd4idx[i]});

      for (int j = 0; j < bndaus[i]; ++j) {
        std::tie(block, idx) = b_dau_indexer.get_daughter_block_index(j);
        std::cout << "(" << block << ", " << idx << ") ";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;

    for (int i = 0; i < nd; ++i) {
      d_dau_indexer.set_daughters(dndaus[i], 
          {dd1lund[i], dd2lund[i], dd3lund[i], dd4lund[i], dd5lund[i]}, 
          {dd1idx[i], dd2idx[i], dd3idx[i], dd4idx[i], dd5idx[i]});

      for (int j = 0; j < dndaus[i]; ++j) {
        std::tie(block, idx) = d_dau_indexer.get_daughter_block_index(j);
        std::cout << "(" << block << ", " << idx << ") ";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << std::endl;

  }

  return 0;
}
