#include <algorithm>

#include "TruthMatcher.h"

TruthMatcher::TruthMatcher() {}

TruthMatcher::~TruthMatcher() {}


void TruthMatcher::set_graph(
    int mc_n_vertices, int mc_n_edges, 
    const std::vector<int> &mc_from_vertices, 
    const std::vector<int> &mc_to_vertices, 
    const std::vector<int> &mc_lund_id, 
    int reco_n_vertices, int reco_n_edges, 
    const std::vector<int> &reco_from_vertices, 
    const std::vector<int> &reco_to_vertices, 
    const std::vector<int> &reco_lund_id, 
    const std::vector<std::vector<int>> &fs_reco_idx,
    const std::vector<std::vector<int>> &fs_matched_idx) {

  clear_cache();

  construct_mc_graph(
      mc_n_vertices, mc_n_edges,
      mc_from_vertices, mc_to_vertices,
      mc_lund_id
  );

  construct_reco_graph(
      reco_n_vertices, reco_n_edges,
      reco_from_vertices, reco_to_vertices,
      reco_lund_id, 
      fs_reco_idx, fs_matched_idx
  );


}


void TruthMatcher::clear_cache() {
  mc_graph_.clear();
  reco_graph_.clear();
}


void TruthMatcher::construct_mc_graph(
    int n_vertices, int n_edges,
    const std::vector<int> &from_vertices, 
    const std::vector<int> &to_vertices, 
    const std::vector<int> &lund_id) {

  construct_graph(
      mc_graph_, n_vertices, n_edges, 
      from_vertices, to_vertices);

  populate_lund_id(mc_graph_, lund_id);

}

void TruthMatcher::construct_reco_graph(
    int n_vertices, int n_edges,
    const std::vector<int> &from_vertices, 
    const std::vector<int> &to_vertices, 
    const std::vector<int> &lund_id, 
    const std::vector<std::vector<int>> &fs_reco_idx,
    const std::vector<std::vector<int>> &fs_matched_idx) {

  construct_graph(
      reco_graph_, n_vertices, n_edges, 
      from_vertices, to_vertices);

  populate_lund_id(reco_graph_, lund_id);

  populate_reco_matched_idx(reco_graph_, n_vertices, 
      fs_reco_idx, fs_matched_idx);

}


void TruthMatcher::construct_graph(
    Graph &g, 
    int n_vertices, int n_edges,
    const std::vector<int> &from_vertices, 
    const std::vector<int> &to_vertices) {

  // check for argument consistency
  if (from_vertices.size() != n_edges) {
    throw std::invalid_argument(
        "TruthMatcher::construct_graph(): n_vertices and " 
        "from_vertices.size() must agree. "
    );
  }

  if (to_vertices.size() != from_vertices.size()) {
    throw std::invalid_argument(
        "TruthMatcher::construct_graph(): from_vertices.size() "
        "must agree with to_vertices.size(). "
    );
  }

  // clear the graph 
  g.clear();

  // establish a mapping between vertex index and vertex descriptors
  std::vector<Vertex> vertex_map(n_vertices);

  // insert vertices and bind internal properties
  for (int i = 0; i < n_vertices; ++i) {
    Vertex u = boost::add_vertex(g);
    vertex_map[i] = u;
    g[u].idx_ = i;
  }

  // insert edges
  for (int i = 0; i < n_edges; ++i) {
    boost::add_edge(vertex_map[from_vertices[i]], 
                    vertex_map[to_vertices[i]], g);
  }
}


void TruthMatcher::populate_lund_id(
    Graph &g, 
    const std::vector<int> &lund_id) {

  // check for argument consistency
  if (lund_id.size() != num_vertices(g)) {
    throw std::invalid_argument(
        "TruthMatcher::populate_lund_id(): lund_id.size() "
        "must agree with num_vertices(g). "
    );
  }

  // populate attributes
  VertexIter vi, vi_end;
  for (std::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
    g[*vi].lund_id_ = lund_id[g[*vi].idx_];
  }

}


void TruthMatcher::populate_reco_matched_idx(
    Graph &g, 
    int n_vertices,
    const std::vector<std::vector<int>> &fs_reco_idx,
    const std::vector<std::vector<int>> &fs_matched_idx) {

  // concatenate final state matched mc indices
  // ------------------------------------------

  std::vector<int> concat_fs_reco_idx;
  for (const auto &vi : fs_reco_idx) {
    std::copy(vi.begin(), vi.end(), std::back_inserter(concat_fs_reco_idx));
  }

  std::vector<int> concat_fs_matched_idx;
  for (const auto &vi : fs_matched_idx) {
    std::copy(vi.begin(), vi.end(), std::back_inserter(concat_fs_matched_idx));
  }

  if (concat_fs_matched_idx.size() != concat_fs_reco_idx.size()) {
    throw std::runtime_error(
        "TruthMatcher::populate_matched_idx(): fs_reco_idx " 
        "and fs_matched_idx must have the same total elements. "
    );
  }

  // populating matched indices of the entire reco graph
  // ---------------------------------------------------

  if (n_vertices != num_vertices(g)) {
    throw std::runtime_error(
        "TruthMatcher::populate_matched_idx(): n_vertices "
        "must agree with num_vertices(g). "
    );
  }

  // determine values for every reco index
  std::vector<int> matched_idx(n_vertices, -1);
  for (size_t i = 0; i < concat_fs_reco_idx.size(); ++i) {
    if (concat_fs_matched_idx[i] >= 0) { 
      matched_idx[concat_fs_reco_idx[i]] = concat_fs_matched_idx[i];
    }
  }

  // fill attributes
  VertexIter vi, vi_end;
  for (std::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
    g[*vi].matched_idx_ = matched_idx[g[*vi].idx_];
  }

}
