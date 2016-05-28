#include <iostream>
#include <vector>

#include <PsqlReader.h>
#include <pgstring_convert.h>

#include "ParticleGraphWriter.h"
#include "TruthMatcher.h"

int main() {

  // open database connection and populate fields
  PsqlReader psql;
  psql.open_connection("dbname=testing");
  psql.open_cursor("truth_match_info", 
      { "eid", 
        "mc_n_vertices", "mc_n_edges", 
        "mc_from_vertices", "mc_to_vertices", "mc_lund_id",
        "reco_n_vertices", "reco_n_edges", 
        "reco_from_vertices", "reco_to_vertices", "reco_lund_id", 
        "h_reco_idx", "hmcidx", 
        "l_reco_idx", "lmcidx", 
        "gamma_reco_idx", "gammamcidx", 
        "y_reco_idx"});

  int eid;
  int mc_n_vertices, mc_n_edges;
  std::vector<int> mc_from_vertices, mc_to_vertices, mc_lund_id;
  int reco_n_vertices, reco_n_edges;
  std::vector<int> reco_from_vertices, reco_to_vertices, reco_lund_id;
  std::vector<int> h_reco_idx, hmcidx;
  std::vector<int> l_reco_idx, lmcidx;
  std::vector<int> gamma_reco_idx, gammamcidx;
  std::vector<int> y_reco_idx;

  // open output file and write title line
  std::ofstream fout; fout.open("truth_match.csv");
  fout << "eid,pruned_mcgraph_from_vertices,pruned_mcgraph_to_vertices,";
  fout << "matching,y_match_status,exist_matched_y" << std::endl;

  // main loop
  size_t n_records = 0;
  while (psql.next()) {

    ++n_records;

    // load record information
    pgstring_convert(psql.get("eid"), eid);
    pgstring_convert(psql.get("mc_n_vertices"), mc_n_vertices);
    pgstring_convert(psql.get("mc_n_edges"), mc_n_edges);
    pgstring_convert(psql.get("mc_from_vertices"), mc_from_vertices);
    pgstring_convert(psql.get("mc_to_vertices"), mc_to_vertices);
    pgstring_convert(psql.get("mc_lund_id"), mc_lund_id);
    pgstring_convert(psql.get("reco_n_vertices"), reco_n_vertices);
    pgstring_convert(psql.get("reco_n_edges"), reco_n_edges);
    pgstring_convert(psql.get("reco_from_vertices"), reco_from_vertices);
    pgstring_convert(psql.get("reco_to_vertices"), reco_to_vertices);
    pgstring_convert(psql.get("reco_lund_id"), reco_lund_id);
    pgstring_convert(psql.get("h_reco_idx"), h_reco_idx);
    pgstring_convert(psql.get("hmcidx"), hmcidx);
    pgstring_convert(psql.get("l_reco_idx"), l_reco_idx);
    pgstring_convert(psql.get("lmcidx"), lmcidx);
    pgstring_convert(psql.get("gamma_reco_idx"), gamma_reco_idx);
    pgstring_convert(psql.get("gammamcidx"), gammamcidx);
    pgstring_convert(psql.get("y_reco_idx"), y_reco_idx);

    
    // compute truth match
    TruthMatcher tm;
    tm.set_graph(
        mc_n_vertices, mc_n_edges,
        mc_from_vertices, mc_to_vertices,
        mc_lund_id, 
        reco_n_vertices, reco_n_edges,
        reco_from_vertices, reco_to_vertices,
        reco_lund_id, 
        { h_reco_idx, l_reco_idx, gamma_reco_idx }, 
        { hmcidx, lmcidx, gammamcidx }
    );

    // compute from and to vertices of pruned mc graph
    std::vector<int> from_vertices, to_vertices;
    TruthMatcher::Graph pruned_mcgraph = tm.get_pruned_mc_graph();
    boost::graph_traits<TruthMatcher::Graph>::edge_iterator ei, ei_end;
    for (std::tie(ei, ei_end) = edges(pruned_mcgraph); ei != ei_end; ++ei) {
      from_vertices.push_back(
          pruned_mcgraph[source(*ei, pruned_mcgraph)].idx_);
      to_vertices.push_back(
          pruned_mcgraph[target(*ei, pruned_mcgraph)].idx_);
    }

    // get matching result
    std::vector<int> matching = tm.get_matching();

    // get y matched status and set indicator
    int exist_matched_y = 0;
    std::vector<int> y_match_status(y_reco_idx.size(), -1);
    for (int i = 0; i < y_reco_idx.size(); ++i) {
      if (matching[y_reco_idx[i]] >= 0) {
        y_match_status[i] = 1;
        exist_matched_y = 1;
      }
    }

    // write a line
    fout << eid << ",";
    fout << vector2pgstring(from_vertices) << ",";
    fout << vector2pgstring(to_vertices) << ",";
    fout << vector2pgstring(matching) << ",";
    fout << vector2pgstring(y_match_status) << ",";
    fout << exist_matched_y;
    fout << std::endl;
  }

  // close file
  fout.close();

  // close database connection
  psql.close_cursor();
  psql.close_connection();

  std::cout << "processed " << n_records << " rows. " << std::endl;

  return 0;
}
