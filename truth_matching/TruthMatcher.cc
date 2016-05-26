#include <algorithm>
#include <unordered_set>
#include <cmath>
#include <queue>

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
  pruned_mc_graph_.clear();
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

  construct_pruned_mc_graph();

}

void TruthMatcher::construct_pruned_mc_graph() {

  // start with a copy of the original mc_graph 
  pruned_mc_graph_ = mc_graph_;

  // remove subtrees for final states reachable from the decay root
  remove_final_state_subtrees(pruned_mc_graph_);

}

void TruthMatcher::remove_final_state_subtrees(Graph &g) {

  // find the decay root. this is the first daughter of the e+e- collision
  Vertex decay_root;

  VertexIter vi, vi_end;
  for (std::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
    if (g[*vi].idx_ == 2) { decay_root = *vi; break; }
  }

  if (vi == vi_end) {
    throw std::runtime_error(
        "TruthMatcher::remove_final_state_subtrees(): " 
        "couldn't find mc_idx 2. ");
  }

  // BFS for the final states and label their subtrees for removal.
  // labelling and then removing avoids iterator invalidation. 
  std::vector<Vertex> to_remove;
  IntPropertyMap lund_pm = get(&VertexProperties::lund_id_, g);

  std::unordered_set<Vertex> visited;

  std::queue<Vertex> q; 
  visited.insert(decay_root); q.push(decay_root);
  while (!q.empty()) {

    Vertex u = q.front(); q.pop();

    // if `u` is considered a final state, then label all its daughter
    // subtrees for removal
    if (is_final_state(get(lund_pm, u))) {

      OutEdgeIter oe, oe_end;
      for (std::tie(oe, oe_end) = out_edges(u, g); 
           oe != oe_end; ++oe) {
        Vertex v = target(*oe, g);
        label_for_removal(v, g, to_remove);
      }

    // if not a final state then continue exploring its daughters
    } else {

      OutEdgeIter oe, oe_end;
      for (std::tie(oe, oe_end) = out_edges(u, g); 
           oe != oe_end; ++oe) {
        Vertex v = target(*oe, g);
        if (visited.find(v) == visited.end()) {
          visited.insert(v); q.push(v);
        }
      }

    }
  }

  // remove those vertices that were labelled
  for (auto u : to_remove) {
    clear_vertex(u, g);
    remove_vertex(u, g);
  }
}

// push vertices in the subtree of `r` into `to_remove`.
// perform BFS and push all reachable vertices
void TruthMatcher::label_for_removal(Vertex r, Graph &g, std::vector<Vertex> &to_remove) {

  std::unordered_set<Vertex> visited;

  std::queue<Vertex> q; 
  visited.insert(r); q.push(r);

  while (!q.empty()) {

    Vertex u = q.front(); q.pop();

    OutEdgeIter oe, oe_end;
    std::tie(oe, oe_end) = out_edges(u, g);
    for (; oe != oe_end; ++oe) {
      Vertex v = target(*oe, g);
      if (visited.find(v) == visited.end()) {
        visited.insert(v); q.push(v);
      }
    }

    to_remove.push_back(u);
  }
}

// decide if a lund id is considered a final state
bool TruthMatcher::is_final_state(int lund_id) {
  switch (abs(lund_id)) {
    case 11:
    case 13:
    case 211:
    case 321:
    case 22:
    case 2212:
    case 2112:
      return true;
  }
  return false;
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
