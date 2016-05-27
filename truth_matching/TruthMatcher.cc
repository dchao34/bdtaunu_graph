#include <algorithm>
#include <unordered_set>
#include <cmath>
#include <queue>
#include <map>
#include <iostream>

#include <boost/property_map/property_map.hpp>

#include "TruthMatcher.h"

// decide if a lund id is considered a final state
bool is_final_state(int lund_id) {
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


// decide if a particle is considered undetectable for truth matching
bool is_undetectable_particle(int lund_id) {
  switch (abs(lund_id)) {
    case 12:
    case 14:
    case 15:
    case 16:
    case 311:
      return true;
  }
  return false;
}

// decide if a particle is a valid photon mother for the purpose
// of truth matching. 
bool is_acceptable_photon_mother(int lund_id) {
  switch (abs(lund_id)) {
    case 111:
    case 413:
    case 423:
      return true;
  }
  return false;
}

// clear data structures
void TruthMatcher::clear_cache() {

  mc_graph_.clear();
  reco_graph_.clear();

  pruned_mc_graph_.clear();
  pruned_mcidx2vtx_.clear();

  matching_.clear();
}

TruthMatcher::TruthMatcher() { clear_cache(); }


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

  // clear all data structures 
  clear_cache();

  // construct the mc graph as well as the pruned version 
  construct_mc_graph(
      mc_n_vertices, mc_n_edges,
      mc_from_vertices, mc_to_vertices,
      mc_lund_id
  );

  // construct the reco graph
  construct_reco_graph(
      reco_n_vertices, reco_n_edges,
      reco_from_vertices, reco_to_vertices,
      reco_lund_id, 
      fs_reco_idx, fs_matched_idx
  );

  // compute the matching
  compute_matching();

}




void TruthMatcher::construct_mc_graph(
    int n_vertices, int n_edges,
    const std::vector<int> &from_vertices, 
    const std::vector<int> &to_vertices, 
    const std::vector<int> &lund_id) {

  // build the graph
  construct_graph(
      mc_graph_, n_vertices, n_edges, 
      from_vertices, to_vertices);

  // attach internal properties
  populate_lund_id(mc_graph_, lund_id);

  // construct a pruned version
  construct_pruned_mc_graph();

}

// the pruned mc graph is the target of graph matching. the un-pruned
// mc graph has too many artifacts and spurious particles
void TruthMatcher::construct_pruned_mc_graph() {

  // start with a copy of the original mc_graph 
  pruned_mc_graph_ = mc_graph_;

  // remove subtrees for final states reachable from the decay root
  remove_final_state_subtrees(pruned_mc_graph_);

  // rip out particles that are not relevant for truth matching
  rip_irrelevant_particles(pruned_mc_graph_);

  // populate the index to vertex mapping. needed later 
  // for computing the matching
  pruned_mcidx2vtx_.clear();
  VertexIter vi, vi_end;
  for (std::tie(vi, vi_end) = vertices(pruned_mc_graph_); 
       vi != vi_end; ++vi) {
    pruned_mcidx2vtx_[pruned_mc_graph_[*vi].idx_] = *vi;
  }

}


void TruthMatcher::rip_irrelevant_particles(Graph &g) {

  // set of particles that need to be ripped. keyed by mc idx_.
  std::unordered_set<int> to_rip;

  // stage the incoming e+ and e- particles for ripping
  // their mc indices are 0 and 1 by construction
  to_rip.insert(0);
  to_rip.insert(1);

  // stage undetectable particles for ripping
  IntPropertyMap index_pm = get(&VertexProperties::idx_, g);
  IntPropertyMap lund_pm = get(&VertexProperties::lund_id_, g);

  VertexIter vi, vi_end;
  for (std::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
    if (is_undetectable_particle(get(lund_pm, *vi))) {
      to_rip.insert(get(index_pm, *vi));
    }
  }

  // stage photons that do not descend from acceptable mothers for ripping
  for (std::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {

    if (get(lund_pm, *vi) == 22) {

      InEdgeIter ie, ie_end;
      std::tie(ie, ie_end) = in_edges(*vi, g);

      Vertex u = source(*ie, g);
      if (!is_acceptable_photon_mother(get(lund_pm, u))) {
        to_rip.insert(get(index_pm, *vi));
      }

      // note: assume only one mother! 
      assert(++ie == ie_end);

    }
  }

  // rip vertices by contracting with its mother. in the case of 
  // no mothers, the procedure is equivalent to just removing 
  // the vertex and outgoing edges. 
  //
  // be careful with iterator invalidation
  VertexIter next; std::tie(vi, vi_end) = vertices(g);
  for (next = vi; vi != vi_end; vi = next) {
    ++next;

    if (to_rip.find(g[*vi].idx_) != to_rip.end()) {

      // add edges from mother to daughters
      InEdgeIter ie, ie_end;
      for (std::tie(ie, ie_end) = in_edges(*vi, g); ie != ie_end; ++ie) {
        Vertex u = source(*ie, g);

        OutEdgeIter oe, oe_end;
        for (std::tie(oe, oe_end) = out_edges(*vi, g); oe != oe_end; ++oe) {
          Vertex v = target(*oe, g);
          add_edge(u, v, g);
        }
      }

      // clear and remove the vertex
      clear_vertex(*vi, g);
      remove_vertex(*vi, g);
    }
  }
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

void TruthMatcher::construct_reco_graph(
    int n_vertices, int n_edges,
    const std::vector<int> &from_vertices, 
    const std::vector<int> &to_vertices, 
    const std::vector<int> &lund_id, 
    const std::vector<std::vector<int>> &fs_reco_idx,
    const std::vector<std::vector<int>> &fs_matched_idx) {

  // build the graph itself
  construct_graph(
      reco_graph_, n_vertices, n_edges, 
      from_vertices, to_vertices);

  // attach internal properties: lund id
  populate_lund_id(reco_graph_, lund_id);

  // attach internal properties: final state matched index. 
  // note that these are the matched indices given from babar. they are, 
  // after slight modifications, used as the base case of our dfs_visitor.
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

  // determine values for every reco index. note that 
  // composite particles get -1. 
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

void TruthMatcher::compute_matching() {

  // initialize result to the empty matching; i.e. all -1. 
  matching_ = std::vector<int>(num_vertices(reco_graph_), -1);

  // create color map for dfs
  std::map<Vertex, boost::default_color_type> color_map;
  boost::associative_property_map< 
    std::map<Vertex, boost::default_color_type> > color_pm(color_map);

  VertexIter vi, vi_end;
  for (std::tie(vi, vi_end) = vertices(reco_graph_); vi != vi_end; ++vi) {
    color_map[*vi] = boost::white_color;
  }

  // create visitor
  TruthMatchDfsVisitor vis(matching_, pruned_mc_graph_, pruned_mcidx2vtx_);

  // compute matching by dfs
  boost::depth_first_search(reco_graph_, visitor(vis).color_map(color_pm));
}


void TruthMatchDfsVisitor::finish_vertex(
    Vertex u, const Graph &reco_graph) {

  // for final states, just lookup the answer stored at the node. 
  //
  // be careful though: while it is true that a non-negative matched index 
  // indicates a match to the mc graph, it need not be a match in 
  // the pruned mc graph. 
  if (is_final_state(reco_graph[u].lund_id_)) {
    if (reco_graph[u].matched_idx_ >= 0 &&
        mcidx2vtx_.find(reco_graph[u].matched_idx_) != mcidx2vtx_.end()) {
      matching_[reco_graph[u].idx_] = reco_graph[u].matched_idx_;
    }


  // for composite states, it can match only to the common mother of all the 
  // particles that its daughters match to. this amounts to the following 
  // criteria:
  // 1. all daughters must match to some particle in the (pruned) mc graph
  // 2. all matched daughters in the mc graph must share the same mother. 
  // 3. that mother must have the same lund id as the composite particle
  // 4. that mother must have the same number of daughter as the composite. 
  } else {

    // 1. check that all daughters match to a particle in the mc graph.
    // put the matched mc particles daughter_matched_mcvtx
    std::vector<Vertex> daughter_matched_mcvtx;

    OutEdgeIter oe, oe_end; 
    for (std::tie(oe, oe_end) = out_edges(u, reco_graph); 
        oe != oe_end; ++oe) {

      Vertex v = target(*oe, reco_graph);
      int daughter_matched_mc_index = matching_[reco_graph[v].idx_];

      // fail if any daughters don't match
      if (daughter_matched_mc_index < 0) { return; }

      daughter_matched_mcvtx.push_back(
          mcidx2vtx_.at(daughter_matched_mc_index));
    }

    if (daughter_matched_mcvtx.size() <= 0) {
      throw std::runtime_error(
          "TruthMatchDfsVisitor::finish_vertex(): composite particle has no "
          "daughters. "
      );
    }


    // 2. check that all matched mc daughters have a common mother
    // cache the result of the first daughter, and compare all others
    // against it. 

    // cache the mother `m` of the first matched daughter. fail if no 
    // mothers exist. `m` is the common mother if it exists. 
    InEdgeIter ie, ie_end;
    std::tie(ie, ie_end) = in_edges(
        *daughter_matched_mcvtx.begin(), mc_graph_);
    if (ie == ie_end) { return; }

    Vertex m = source(*ie, reco_graph);
    for (auto it = daughter_matched_mcvtx.begin()+1; 
         it != daughter_matched_mcvtx.end(); ++it) {
      std::tie(ie, ie_end) = in_edges(*it, mc_graph_);

      // fail if no mother or if it does not agree with the mother 
      // of the first daughter
      if (ie == ie_end) { return; }
      if (source(*ie, reco_graph) != m) { return; }
    }

    // 3. check for mother lund. fail if it does not agree with the 
    // lund id of the composite reco particle 
    if (mc_graph_[m].lund_id_ != reco_graph[u].lund_id_) { return; }

    // 4. check the number of daughters descending from the common mother
    size_t n_mc_daughters = 0;
    for (std::tie(oe, oe_end) = out_edges(m, mc_graph_); oe != oe_end; ++oe) {
      n_mc_daughters++;
    }

    // fail if it does not have the same number of daughters
    // as the composite particle
    if (n_mc_daughters != daughter_matched_mcvtx.size()) { return; }

    // success. cache the result
    matching_[reco_graph[u].idx_] = mc_graph_[m].idx_;

  }

}

