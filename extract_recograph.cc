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
#include "RecoIndexer.h"
#include "RecoEdgeAssociator.h"

using namespace std;
using namespace boost;


// convenience function to determine reconstruction adjacencies 
// for given block
void add_edges(
    const std::string &block_name,
    const RecoIndexer &reco_indexer,
    const RecoEdgeAssociator &edge_assoc,
    const std::unordered_map<int, std::string> &lund2block,
    std::vector<int> &from, std::vector<int> &to, 
    int &n_edges) {

  for (int i = 0; i < edge_assoc.n_mothers(); ++i) {

    int u = reco_indexer.global_index(block_name, i);

    for (int j = 0; j < edge_assoc.n_daughters(i); ++j) {

      int v = reco_indexer.global_index(
          lund2block.at(edge_assoc.daughter_lund(i, j)), 
          edge_assoc.daughter_idx(i, j));

      from.push_back(u);
      to.push_back(v);
      ++n_edges;
    }
  }
}


// output csv formatting functions
void write_title_line(std::ostream &os) {
  os << "eid,n_vertices,n_edges,from,to,lund_id,"
        "y_reco_idx,b_reco_idx,d_reco_idx,c_reco_idx,"
        "h_reco_idx,l_reco_idx,gamma_reco_idx";
  os << std::endl;
}

void write_record_line(
  std::ostream &os, size_t eid, 
  int n_vertices, int n_edges,
  const std::vector<int> &from, const std::vector<int> &to,
  const std::vector<int> &lund_id, 
  const std::vector<int> &y_reco_idx, const std::vector<int> &b_reco_idx, 
  const std::vector<int> &d_reco_idx, const std::vector<int> &c_reco_idx, 
  const std::vector<int> &h_reco_idx, const std::vector<int> &l_reco_idx, 
  const std::vector<int> &gamma_reco_idx) {

  os << eid << ",";
  os << n_vertices << ",";
  os << n_edges << ",";
  os << vector2pgstring(from) << ",";
  os << vector2pgstring(to) << ",";
  os << vector2pgstring(lund_id) << ",";
  os << vector2pgstring(y_reco_idx) << ",";
  os << vector2pgstring(b_reco_idx) << ",";
  os << vector2pgstring(d_reco_idx) << ",";
  os << vector2pgstring(c_reco_idx) << ",";
  os << vector2pgstring(h_reco_idx) << ",";
  os << vector2pgstring(l_reco_idx) << ",";
  os << vector2pgstring(gamma_reco_idx);
  os << std::endl;
}


// reads the framework ntuple reconstruction adjacencies to produce 
// the more natural adjacency list representation
int main() {

  // 1. setup data structures. see the BtaTupleMaker block to 
  //    decide how to initialize these structures. 

  // lund id to reconstruction block mapping. 
  std::unordered_map<int, std::string> lund2block;
  lund2block.insert({70553, "y"});
  lund2block.insert({521, "b"});
  lund2block.insert({-521, "b"});
  lund2block.insert({511, "b"});
  lund2block.insert({-511, "b"});
  lund2block.insert({413, "d"});
  lund2block.insert({-413, "d"});
  lund2block.insert({423, "d"});
  lund2block.insert({-423, "d"});
  lund2block.insert({421, "d"});
  lund2block.insert({-421, "d"});
  lund2block.insert({411, "d"});
  lund2block.insert({-411, "d"});
  lund2block.insert({310, "c"});
  lund2block.insert({213, "c"});
  lund2block.insert({-213, "c"});
  lund2block.insert({111, "c"});
  lund2block.insert({321, "h"});
  lund2block.insert({-321, "h"});
  lund2block.insert({211, "h"});
  lund2block.insert({-211, "h"});
  lund2block.insert({11, "l"});
  lund2block.insert({-11, "l"});
  lund2block.insert({13, "l"});
  lund2block.insert({-13, "l"});
  lund2block.insert({22, "gamma"});

  // global indexer for all reconstructed particles
  RecoIndexer reco_indexer({"y", "b", "d", "c", "h", "l", "gamma"}, 
                           {800, 400, 200, 100, 100, 100, 100});

  // data structure that vastly simplifies how 
  // reconstruction edges are associated
  RecoEdgeAssociator y_assoc(800, 2);
  RecoEdgeAssociator b_assoc(400, 4);
  RecoEdgeAssociator d_assoc(200, 5);
  RecoEdgeAssociator c_assoc(100, 2);
  RecoEdgeAssociator h_assoc(100, 2);
  RecoEdgeAssociator l_assoc(100, 3);

  // 2. declare the data to compute
  int n_vertices, n_edges;
  std::vector<int> from, to;
  std::vector<int> lund_id;
  std::vector<int> y_reco_idx, b_reco_idx, d_reco_idx, c_reco_idx;
  std::vector<int> h_reco_idx, l_reco_idx, gamma_reco_idx;

  // 3. open output file
  std::ofstream fout; 
  fout.open("recograph_adjacency.csv");
  write_title_line(fout);

  // 4. open postgres reader and declare the required input data
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
        "ld1idx", "ld2idx", "ld3idx" });

  // initialize location to save downloaded data
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

  // main loop
  int n_records = 0;
  while (psql.next()) {
    ++n_records;

    // 6. download all the data
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


    // 7. update data structures
    reco_indexer.set_block_sizes({ny, nb, nd, nc, nh, nl, ngamma});
    y_assoc.associate_edges(ny, ylund, yndaus, 
      { yd1lund, yd2lund }, { yd1idx, yd2idx });
    b_assoc.associate_edges(nb, blund, bndaus, 
      { bd1lund, bd2lund, bd3lund, bd4lund }, 
      { bd1idx, bd2idx, bd3idx, bd4idx });
    d_assoc.associate_edges(nd, dlund, dndaus, 
      { dd1lund, dd2lund, dd3lund, dd4lund, dd5lund }, 
      { dd1idx, dd2idx, dd3idx, dd4idx, dd5idx });
    c_assoc.associate_edges(nc, clund, cndaus, 
      { cd1lund, cd2lund }, { cd1idx, cd2idx });
    h_assoc.associate_edges(nh, hlund, hndaus, 
      { hd1lund, hd2lund }, { hd1idx, hd2idx });
    l_assoc.associate_edges(nl, llund, lndaus, 
      { ld1lund, ld2lund, ld3lund }, { ld1idx, ld2idx, ld3idx});

    // 8. skip problematic records

    // bta tuple maker known to have bugs when candidate block is full
    if (reco_indexer.has_full_block()) { continue; }

    // 9. compute quantities of interest 

    // clear cache
    n_vertices = 0; n_edges = 0;
    from.clear(); to.clear(); 
    lund_id.clear();
    y_reco_idx.clear(), b_reco_idx.clear(); 
    d_reco_idx.clear(), c_reco_idx.clear();
    h_reco_idx.clear(), l_reco_idx.clear(); 
    gamma_reco_idx.clear();

    // compute n_vertices
    n_vertices = reco_indexer.total_size();

    // compute local to global index mappings
    for (int i = 0; i < ny; ++i) { y_reco_idx.push_back(reco_indexer.global_index("y", i)); }
    for (int i = 0; i < nb; ++i) { b_reco_idx.push_back(reco_indexer.global_index("b", i)); }
    for (int i = 0; i < nd; ++i) { d_reco_idx.push_back(reco_indexer.global_index("d", i)); }
    for (int i = 0; i < nc; ++i) { c_reco_idx.push_back(reco_indexer.global_index("c", i)); }
    for (int i = 0; i < nh; ++i) { h_reco_idx.push_back(reco_indexer.global_index("h", i)); }
    for (int i = 0; i < nl; ++i) { l_reco_idx.push_back(reco_indexer.global_index("l", i)); }
    for (int i = 0; i < ngamma; ++i) { gamma_reco_idx.push_back(reco_indexer.global_index("gamma", i)); }

    // compute global lund id
    lund_id = std::vector<int>(n_vertices);
    for (int i = 0; i < ny; ++i) { lund_id[reco_indexer.global_index("y", i)] = ylund[i]; }
    for (int i = 0; i < nb; ++i) { lund_id[reco_indexer.global_index("b", i)] = blund[i]; }
    for (int i = 0; i < nd; ++i) { lund_id[reco_indexer.global_index("d", i)] = dlund[i]; }
    for (int i = 0; i < nc; ++i) { lund_id[reco_indexer.global_index("c", i)] = clund[i]; }
    for (int i = 0; i < nh; ++i) { lund_id[reco_indexer.global_index("h", i)] = hlund[i]; }
    for (int i = 0; i < nl; ++i) { lund_id[reco_indexer.global_index("l", i)] = llund[i]; }
    for (int i = 0; i < ngamma; ++i) { lund_id[reco_indexer.global_index("gamma", i)] = gammalund[i]; }


    // compute edge count and edge adjacency 
    add_edges("y", reco_indexer, y_assoc, lund2block, from, to, n_edges);
    add_edges("b", reco_indexer, b_assoc, lund2block, from, to, n_edges);
    add_edges("d", reco_indexer, d_assoc, lund2block, from, to, n_edges);
    add_edges("c", reco_indexer, c_assoc, lund2block, from, to, n_edges);
    add_edges("h", reco_indexer, h_assoc, lund2block, from, to, n_edges);
    add_edges("l", reco_indexer, l_assoc, lund2block, from, to, n_edges);


    // 10. write to file
    write_record_line(fout, eid, 
        n_vertices, n_edges, from, to, lund_id,
        y_reco_idx, b_reco_idx, d_reco_idx, c_reco_idx,
        h_reco_idx, l_reco_idx, gamma_reco_idx);

  }

  std::cout << "processed " << n_records << "rows. " << std::endl;

  // 11. close file and postgres connection
  fout.close();
  psql.close_cursor();
  psql.close_connection();

  return 0;
}

