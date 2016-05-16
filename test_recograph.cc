#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <utility>
#include <unordered_map>
#include <initializer_list>
#include <map>
#include <stdexcept>
#include <algorithm>
#include <numeric>

#include "pgstring_convert.h"
#include "PsqlReader.h"

using namespace std;
using namespace boost;

std::string vector2pgstring(const std::vector<int> &v) {

  if (v.empty()) { return "{}"; }

  std::string s = "\"{";

  for (const auto &e : v) {
    s += std::to_string(e) + ",";
  }

  s.pop_back(); s += "}\"";
  return s;
}

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

      total_ = std::accumulate(block_sizes_.begin(), block_sizes_.end(), 0);

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

    int get_total_size() const {
      return total_;
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

    int total_;

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

  int eid;
  int ny, nb, nd, nc, nh, nl, ngamma;
  std::vector<int> ylund, blund, dlund, clund, hlund, llund, gammalund;
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


  PsqlReader psql; 
  psql.open_connection("dbname=testing");
  psql.open_cursor("framework_ntuples", 
      { "eid", 
        "ylund", "blund", "dlund", "clund", "hlund", "llund", "gammalund",
        "ny", "nb", "nd", "nc", "nh", "nl", "ngamma", 
        "yndaus", "bndaus", "dndaus", "cndaus", "hndaus", "lndaus", "gammandaus", 
        "yd1lund", "yd2lund", 
        "yd1idx", "yd2idx", 
        "bd1lund", "bd2lund", "bd3lund", "bd4lund", 
        "bd1idx", "bd2idx", "bd3idx", "bd4idx", 
        "dd1lund", "dd2lund", "dd3lund", "dd4lund", "dd5lund", 
        "dd1idx", "dd2idx", "dd3idx", "dd4idx", "dd5idx", 
        "cd1lund", "cd2lund", 
        "cd1idx", "cd2idx", 
        "hd1lund", "hd2lund", 
        "hd1idx", "hd2idx", 
        "ld1lund", "ld2lund", "ld3lund", 
        "ld1idx", "ld2idx", "ld3idx" },
      5000);

  int n_vertices, n_edges;
  std::vector<int> from, to;
  std::vector<int> lund_id;
  std::vector<int> y_reco_idx, b_reco_idx, d_reco_idx, c_reco_idx;
  std::vector<int> h_reco_idx, l_reco_idx, gamma_reco_idx;

  std::ofstream fout; fout.open("recograph_adjacency.csv");
  fout << "eid,n_vertices,n_edges,from,to,lund_id,";
  fout << "y_reco_idx,b_reco_idx,d_reco_idx,c_reco_idx,h_reco_idx,l_reco_idx,gamma_reco_idx";
  fout << std::endl;

  int n_records = 0;
  while (psql.next()) {
    ++n_records;

    n_vertices = 0; n_edges = 0;
    from.clear(); to.clear(); lund_id.clear();
    y_reco_idx.clear(), b_reco_idx.clear(); 
    d_reco_idx.clear(), c_reco_idx.clear();
    h_reco_idx.clear(), l_reco_idx.clear(), gamma_reco_idx.clear();

    pgstring_convert(psql.get("eid"), eid);

    pgstring_convert(psql.get("ny"), ny);
    pgstring_convert(psql.get("nb"), nb);
    pgstring_convert(psql.get("nd"), nd);
    pgstring_convert(psql.get("nc"), nc);
    pgstring_convert(psql.get("nh"), nh);
    pgstring_convert(psql.get("nl"), nl);
    pgstring_convert(psql.get("ngamma"), ngamma);

    pgstring_convert(psql.get("ylund"), ylund);
    pgstring_convert(psql.get("blund"), blund);
    pgstring_convert(psql.get("dlund"), dlund);
    pgstring_convert(psql.get("clund"), clund);
    pgstring_convert(psql.get("hlund"), hlund);
    pgstring_convert(psql.get("llund"), llund);
    pgstring_convert(psql.get("gammalund"), gammalund);

    pgstring_convert(psql.get("yndaus"), yndaus);
    pgstring_convert(psql.get("bndaus"), bndaus);
    pgstring_convert(psql.get("dndaus"), dndaus);
    pgstring_convert(psql.get("cndaus"), cndaus);
    pgstring_convert(psql.get("hndaus"), hndaus);
    pgstring_convert(psql.get("lndaus"), lndaus);
    pgstring_convert(psql.get("gammandaus"), gammandaus);

    pgstring_convert(psql.get("yd1lund"), yd1lund);
    pgstring_convert(psql.get("yd1idx"), yd1idx);
    pgstring_convert(psql.get("yd2lund"), yd2lund);
    pgstring_convert(psql.get("yd2idx"), yd2idx);

    pgstring_convert(psql.get("bd1lund"), bd1lund);
    pgstring_convert(psql.get("bd1idx"), bd1idx);
    pgstring_convert(psql.get("bd2lund"), bd2lund);
    pgstring_convert(psql.get("bd2idx"), bd2idx);
    pgstring_convert(psql.get("bd3lund"), bd3lund);
    pgstring_convert(psql.get("bd3idx"), bd3idx);
    pgstring_convert(psql.get("bd4lund"), bd4lund);
    pgstring_convert(psql.get("bd4idx"), bd4idx);

    pgstring_convert(psql.get("dd1lund"), dd1lund);
    pgstring_convert(psql.get("dd1idx"), dd1idx);
    pgstring_convert(psql.get("dd2lund"), dd2lund);
    pgstring_convert(psql.get("dd2idx"), dd2idx);
    pgstring_convert(psql.get("dd3lund"), dd3lund);
    pgstring_convert(psql.get("dd3idx"), dd3idx);
    pgstring_convert(psql.get("dd4lund"), dd4lund);
    pgstring_convert(psql.get("dd4idx"), dd4idx);
    pgstring_convert(psql.get("dd5lund"), dd5lund);
    pgstring_convert(psql.get("dd5idx"), dd5idx);

    pgstring_convert(psql.get("cd1lund"), cd1lund);
    pgstring_convert(psql.get("cd1idx"), cd1idx);
    pgstring_convert(psql.get("cd2lund"), cd2lund);
    pgstring_convert(psql.get("cd2idx"), cd2idx);

    pgstring_convert(psql.get("hd1lund"), hd1lund);
    pgstring_convert(psql.get("hd1idx"), hd1idx);
    pgstring_convert(psql.get("hd2lund"), hd2lund);
    pgstring_convert(psql.get("hd2idx"), hd2idx);

    pgstring_convert(psql.get("ld1lund"), ld1lund);
    pgstring_convert(psql.get("ld1idx"), ld1idx);
    pgstring_convert(psql.get("ld2lund"), ld2lund);
    pgstring_convert(psql.get("ld2idx"), ld2idx);
    pgstring_convert(psql.get("ld3lund"), ld3lund);
    pgstring_convert(psql.get("ld3idx"), ld3idx);

    reco_indexer.set_block_sizes({ny, nb, nd, nc, nh, nl, ngamma});

    n_vertices = reco_indexer.get_total_size();

    int y_start_index = reco_indexer.get_start_index("y");
    int b_start_index = reco_indexer.get_start_index("b");
    int d_start_index = reco_indexer.get_start_index("d");
    int c_start_index = reco_indexer.get_start_index("c");
    int h_start_index = reco_indexer.get_start_index("h");
    int l_start_index = reco_indexer.get_start_index("l");
    int gamma_start_index = reco_indexer.get_start_index("gamma");
    for (int i = 0; i < ny; ++i) { 
      y_reco_idx.push_back(y_start_index + i); 
      lund_id.push_back(ylund[i]);
    }
    for (int i = 0; i < nb; ++i) { 
      b_reco_idx.push_back(b_start_index + i); 
      lund_id.push_back(blund[i]);
    }
    for (int i = 0; i < nd; ++i) { 
      d_reco_idx.push_back(d_start_index + i); 
      lund_id.push_back(dlund[i]);
    }
    for (int i = 0; i < nc; ++i) { 
      c_reco_idx.push_back(c_start_index + i); 
      lund_id.push_back(clund[i]);
    }
    for (int i = 0; i < nh; ++i) { 
      h_reco_idx.push_back(h_start_index + i); 
      lund_id.push_back(hlund[i]);
    }
    for (int i = 0; i < nl; ++i) { 
      l_reco_idx.push_back(l_start_index + i); 
      lund_id.push_back(llund[i]);
    }
    for (int i = 0; i < ngamma; ++i) { 
      gamma_reco_idx.push_back(gamma_start_index + i); 
      lund_id.push_back(gammalund[i]);
    }


    if (ny >= 800 || nb >= 400 || nd >= 200 ||
        nc >= 100 || nh >= 100 || nl >= 100 || ngamma >= 100) {
      continue;
    }

    std::string block; int idx;

    for (int i = 0; i < ny; ++i) {
      y_dau_indexer.set_daughters(yndaus[i], 
          {yd1lund[i], yd2lund[i]}, 
          {yd1idx[i], yd2idx[i]});

      int u = y_start_index + i;
      for (int j = 0; j < yndaus[i]; ++j) {

        std::tie(block, idx) = y_dau_indexer.get_daughter_block_index(j);
        int v = reco_indexer.get_index(block, idx);

        from.push_back(u);
        to.push_back(v);
        ++n_edges;
      }
    }

    for (int i = 0; i < nb; ++i) {
      b_dau_indexer.set_daughters(bndaus[i], 
          {bd1lund[i], bd2lund[i], bd3lund[i], bd4lund[i]}, 
          {bd1idx[i], bd2idx[i], bd3idx[i], bd4idx[i]});

      int u = b_start_index + i;
      for (int j = 0; j < bndaus[i]; ++j) {

        std::tie(block, idx) = b_dau_indexer.get_daughter_block_index(j);
        int v = reco_indexer.get_index(block, idx);

        from.push_back(u);
        to.push_back(v);
        ++n_edges;
      }
    }

    for (int i = 0; i < nd; ++i) {
      d_dau_indexer.set_daughters(dndaus[i], 
          {dd1lund[i], dd2lund[i], dd3lund[i], dd4lund[i], dd5lund[i]}, 
          {dd1idx[i], dd2idx[i], dd3idx[i], dd4idx[i], dd5idx[i]});

      int u = d_start_index + i;
      for (int j = 0; j < dndaus[i]; ++j) {

        std::tie(block, idx) = d_dau_indexer.get_daughter_block_index(j);
        int v = reco_indexer.get_index(block, idx);

        from.push_back(u);
        to.push_back(v);
        ++n_edges;
      }
    }

    for (int i = 0; i < nc; ++i) {
      c_dau_indexer.set_daughters(cndaus[i], 
          {cd1lund[i], cd2lund[i]}, 
          {cd1idx[i], cd2idx[i]});

      int u = c_start_index + i;
      for (int j = 0; j < cndaus[i]; ++j) {

        std::tie(block, idx) = c_dau_indexer.get_daughter_block_index(j);
        int v = reco_indexer.get_index(block, idx);

        from.push_back(u);
        to.push_back(v);
        ++n_edges;
      }
    }

    for (int i = 0; i < nh; ++i) {
      h_dau_indexer.set_daughters(hndaus[i], 
          {hd1lund[i], hd2lund[i]}, 
          {hd1idx[i], hd2idx[i]});

      int u = h_start_index + i;
      for (int j = 0; j < hndaus[i]; ++j) {

        std::tie(block, idx) = h_dau_indexer.get_daughter_block_index(j);
        int v = reco_indexer.get_index(block, idx);

        from.push_back(u);
        to.push_back(v);
        ++n_edges;
      }
    }

    for (int i = 0; i < nl; ++i) {
      l_dau_indexer.set_daughters(lndaus[i], 
          {ld1lund[i], ld2lund[i], ld3lund[i]}, 
          {ld1idx[i], ld2idx[i], ld3idx[i]});

      int u = l_start_index + i;
      for (int j = 0; j < lndaus[i]; ++j) {

        std::tie(block, idx) = l_dau_indexer.get_daughter_block_index(j);
        int v = reco_indexer.get_index(block, idx);

        from.push_back(u);
        to.push_back(v);
        ++n_edges;
      }
    }

    fout << eid << ",";
    fout << n_vertices << ",";
    fout << n_edges << ",";
    fout << vector2pgstring(from) << ",";
    fout << vector2pgstring(to) << ",";
    fout << vector2pgstring(lund_id) << ",";
    fout << vector2pgstring(y_reco_idx) << ",";
    fout << vector2pgstring(b_reco_idx) << ",";
    fout << vector2pgstring(d_reco_idx) << ",";
    fout << vector2pgstring(c_reco_idx) << ",";
    fout << vector2pgstring(h_reco_idx) << ",";
    fout << vector2pgstring(l_reco_idx) << ",";
    fout << vector2pgstring(gamma_reco_idx);
    fout << std::endl;

  }
  std::cout << n_records << std::endl;

  fout.close();

  psql.close_cursor();
  psql.close_connection();

  return 0;
}
