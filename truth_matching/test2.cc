#include <iostream>
#include <fstream>
#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

#include <pgstring_convert.h>
#include <PsqlReader.h>

#include "ParticleGraph.h"
#include "ParticleGraphWriter.h"


struct VertexProperties {
  int idx;
  int lund_id;
  int mc_idx;
};

using Graph = boost::adjacency_list<boost::listS, boost::listS, 
                                    boost::bidirectionalS, VertexProperties>;
using IntPropertyMap = boost::property_map<Graph, int VertexProperties::*>::type;

void fill_reco_mcidx(
    std::vector<int> &target_mc_idx, 
    const std::vector<int> &reco_idx, 
    const std::vector<int> &mc_idx) {

  if (reco_idx.size() != mc_idx.size()) {
    throw std::invalid_argument(
        "fill_reco_mcidx(): reco_idx.size() must be equal to mc_idx.size()");
  }

  for (int i = 0; i < mc_idx.size(); ++i) {
    if (reco_idx[i] >= target_mc_idx.size()) {
      throw std::runtime_error(
          "fill_reco_mcidx(): reco_idx[i] must be less than target_mc_idx.size().");
    }

    if (mc_idx[i] >= 0) { 
      target_mc_idx[reco_idx[i]] = mc_idx[i];
    }
  }
}


template <typename AdjacencyListGraph>
void populate_reco_mc_idx(
    AdjacencyListGraph &g, 
    const std::vector<int> &reco_mc_idx) {

  // check for argument consistency
  if (reco_mc_idx.size() != num_vertices(g)) {
    throw std::invalid_argument(
        "populate_reco_mc_idx(): reco_mc_idx.size() must agree with num_vertices(g). "
    );
  }

  // populate attributes
  typename boost::graph_traits<AdjacencyListGraph>::vertex_iterator vi, vi_end;
  for (std::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
    g[*vi].mc_idx = reco_mc_idx[g[*vi].idx];
  }

}

int main() {

  // open database connection and populate fields
  PsqlReader psql;
  psql.open_connection("dbname=testing");
  psql.open_cursor(
      "truth_match_info", 
      { "eid", "reco_n_vertices", "reco_n_edges", 
        "reco_from_vertices", "reco_to_vertices", "reco_lund_id", 
        "h_reco_idx", "l_reco_idx", "gamma_reco_idx",
        "hmcidx", "lmcidx", "gammamcidx" });


  int reco_n_vertices, reco_n_edges;
  std::vector<int> reco_from_vertices, reco_to_vertices, reco_lund_id;
  std::vector<int> h_reco_idx, l_reco_idx, gamma_reco_idx;
  std::vector<int> hmcidx, lmcidx, gammamcidx;

  psql.next();

  pgstring_convert(psql.get("reco_n_vertices"), reco_n_vertices);
  pgstring_convert(psql.get("reco_n_edges"), reco_n_edges);
  pgstring_convert(psql.get("reco_from_vertices"), reco_from_vertices);
  pgstring_convert(psql.get("reco_to_vertices"), reco_to_vertices);
  pgstring_convert(psql.get("reco_lund_id"), reco_lund_id);
  pgstring_convert(psql.get("h_reco_idx"), h_reco_idx);
  pgstring_convert(psql.get("l_reco_idx"), l_reco_idx);
  pgstring_convert(psql.get("gamma_reco_idx"), gamma_reco_idx);
  pgstring_convert(psql.get("hmcidx"), hmcidx);
  pgstring_convert(psql.get("lmcidx"), lmcidx);
  pgstring_convert(psql.get("gammamcidx"), gammamcidx);

  // build and analyze graph
  Graph g;
  construct_graph(g, reco_n_vertices, reco_n_edges, reco_from_vertices, reco_to_vertices);
  populate_lund_id(g, reco_lund_id);

  std::vector<int> mc_idx(reco_n_vertices, -1);
  fill_reco_mcidx(mc_idx, h_reco_idx, hmcidx);
  fill_reco_mcidx(mc_idx, l_reco_idx, lmcidx);
  fill_reco_mcidx(mc_idx, gamma_reco_idx, gammamcidx);
  populate_reco_mc_idx(g, mc_idx);


  // print 
  ParticleGraphWriter writer("../dat/pdt.dat");
  IntPropertyMap index_pm = get(&VertexProperties::idx, g);
  IntPropertyMap mc_idx_pm = get(&VertexProperties::mc_idx, g);
  writer.print(std::cout, g, mc_idx_pm, index_pm, false);

  // close database connection
  psql.close_cursor();
  psql.close_connection();

  return 0;
}
