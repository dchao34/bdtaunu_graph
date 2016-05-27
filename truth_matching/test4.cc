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
        "gamma_reco_idx", "gammamcidx"});


  int eid;
  int mc_n_vertices, mc_n_edges;
  std::vector<int> mc_from_vertices, mc_to_vertices, mc_lund_id;
  int reco_n_vertices, reco_n_edges;
  std::vector<int> reco_from_vertices, reco_to_vertices, reco_lund_id;
  std::vector<int> h_reco_idx, hmcidx;
  std::vector<int> l_reco_idx, lmcidx;
  std::vector<int> gamma_reco_idx, gammamcidx;

  psql.next();

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

  // print
  TruthMatchGraphPrinter tm_printer("../dat/pdt.dat");
  tm_printer.print(std::cout, tm.get_reco_graph(), tm.get_matching(), 
                   tm.get_reco_idx_pm(), tm.get_reco_lund_id_pm());

  // close database connection
  psql.close_cursor();
  psql.close_connection();
  return 0;
}
