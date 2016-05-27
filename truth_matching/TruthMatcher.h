#ifndef _TRUTH_MATCHER_H_
#define _TRUTH_MATCHER_H_

#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>

// determine whether a lund id is considered to be 
// final state for the purpose of truth matching
bool is_final_state(int lund_id);

// determine whether a lund id is considered to be 
// undetectable for the purpose of truth matching
bool is_undetectable_particle(int lund_id);

// determine whether a lund id is considered to be 
// a valid mother for a photon in truth matching
bool is_acceptable_photon_mother(int lund_id);


// class that performs truth matching by solving subgraph isomorphism. 
class TruthMatcher {

  public:

    struct VertexProperties {
      int idx_;
      int lund_id_;
      int matched_idx_;
    };

    using Graph = boost::adjacency_list<
      boost::listS, boost::listS,
      boost::bidirectionalS, VertexProperties>;

    using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
    using VertexIter = typename boost::graph_traits<Graph>::vertex_iterator;
    using InEdgeIter = typename boost::graph_traits<Graph>::in_edge_iterator;
    using OutEdgeIter = typename boost::graph_traits<Graph>::out_edge_iterator;

    using IntPropertyMap = boost::property_map<
      Graph, int VertexProperties::*>::type;


  public:

    // default constructor initializes the truth matcher to a 
    // state ready to accept inputs
    TruthMatcher();

    ~TruthMatcher();

    // load graph information and compute the matching. once set, 
    // you can call the get methods to access the results. 
    //
    // Input: 
    //
    // vertices assume a fixed ordering indexed by integers in
    // the range [0, n_vertices-1]. 
    // `mc` and `reco` prefixed quantities are associated with the 
    // monte carlo and reconstruction graph respectively. 
    // + (mc|reco)_n_vertices: number of vertices in the graph
    // + (mc|reco)_n_edges: number of edges in the graph
    // + (mc|reco)_from_vertices: edge source vertices
    // + (mc|reco)_to_vertices: edge target vertices
    // + (mc|reco)_lund_id: lund id of the vertices
    //
    // + fs_reco_idx: vector of vectors. each vector element 
    //   is a list of reco indices corresponding to a particular
    //   kind of final state particle. 
    //
    // + fs_matched_idx: vector of vectors. each vector element 
    //   is a list of indices that the corresponding final state 
    //   matches to based on the detector hit. 
    void set_graph(
        int mc_n_vertices, int mc_n_edges, 
        const std::vector<int> &mc_from_vertices, 
        const std::vector<int> &mc_to_vertices, 
        const std::vector<int> &mc_lund_id, 
        int reco_n_vertices, int reco_n_edges, 
        const std::vector<int> &reco_from_vertices, 
        const std::vector<int> &reco_to_vertices, 
        const std::vector<int> &reco_lund_id, 
        const std::vector<std::vector<int>> &fs_reco_idx,
        const std::vector<std::vector<int>> &fs_matched_idx
    );

    // get a referece to the mc graph. 
    Graph get_mc_graph() const;

    // get a referece to the pruned mc graph; that is, the graph
    // is the target of matching. 
    Graph get_pruned_mc_graph() const;

    // get a referece to the reconstructed graph. 
    Graph get_reco_graph() const;

    // get the result of the matching. value of element `i` indicates the 
    // matched index of reconstructed particle `i`. 
    const std::vector<int>& get_matching() const;

    // get property maps. 
    // prefer to have const... but need to think of a way. to fix!
    IntPropertyMap get_mc_idx_pm();
    IntPropertyMap get_mc_lund_id_pm();
    IntPropertyMap get_pruned_mc_idx_pm();
    IntPropertyMap get_pruned_mc_lund_id_pm();
    IntPropertyMap get_reco_idx_pm();
    IntPropertyMap get_reco_lund_id_pm();
    IntPropertyMap get_reco_matched_idx_pm();

  private:
    void clear_cache();

    void construct_mc_graph(
        int n_vertices, int n_edges,
        const std::vector<int> &from_vertices, 
        const std::vector<int> &to_vertices, 
        const std::vector<int> &lund_id);

    void construct_reco_graph(
        int n_vertices, int n_edges,
        const std::vector<int> &from_vertices, 
        const std::vector<int> &to_vertices, 
        const std::vector<int> &lund_id, 
        const std::vector<std::vector<int>> &fs_reco_idx,
        const std::vector<std::vector<int>> &fs_matched_idx);

    void construct_graph(Graph &g, 
        int n_vertices, int n_edges,
        const std::vector<int> &from_vertices, 
        const std::vector<int> &to_vertices);

    void populate_lund_id(Graph &g, 
        const std::vector<int> &lund_id);

    void populate_reco_matched_idx(Graph &g, int n_vertices,
        const std::vector<std::vector<int>> &fs_reco_idx,
        const std::vector<std::vector<int>> &fs_matched_idx);

    void construct_pruned_mc_graph();
    void remove_final_state_subtrees(Graph &g);
    void label_for_removal(Vertex, Graph&, std::vector<Vertex>&);
    void rip_irrelevant_particles(Graph &g);

    void compute_matching();

  private:
    Graph mc_graph_;
    Graph reco_graph_;

    Graph pruned_mc_graph_;
    std::unordered_map<int, Vertex> pruned_mcidx2vtx_;

    std::vector<int> matching_;
                      
};

class TruthMatchDfsVisitor : public boost::default_dfs_visitor {

  public:
    using Graph = TruthMatcher::Graph;
    using Vertex = TruthMatcher::Vertex;
    using InEdgeIter = TruthMatcher::InEdgeIter;
    using OutEdgeIter = TruthMatcher::OutEdgeIter;

  public: 
    TruthMatchDfsVisitor(std::vector<int> &matching, const Graph &mc_graph,
        const std::unordered_map<int, Vertex> &mcidx2vtx) : 
        matching_(matching), mc_graph_(mc_graph), mcidx2vtx_(mcidx2vtx) {}

    void finish_vertex(Vertex u, const Graph &reco_graph);

  private:
    std::vector<int> &matching_;
    const Graph &mc_graph_;
    const std::unordered_map<int, Vertex> &mcidx2vtx_;

};

inline TruthMatcher::Graph 
TruthMatcher::get_mc_graph() const { return mc_graph_; }

inline TruthMatcher::IntPropertyMap TruthMatcher::get_mc_idx_pm() { 
  return get(&VertexProperties::idx_, mc_graph_); 
}

inline TruthMatcher::IntPropertyMap TruthMatcher::get_mc_lund_id_pm() { 
  return get(&VertexProperties::lund_id_, mc_graph_); 
}

inline TruthMatcher::Graph 
TruthMatcher::get_pruned_mc_graph() const { return pruned_mc_graph_; }

inline TruthMatcher::IntPropertyMap 
TruthMatcher::get_pruned_mc_idx_pm() { 
  return get(&VertexProperties::idx_, pruned_mc_graph_); 
}

inline TruthMatcher::IntPropertyMap 
TruthMatcher::get_pruned_mc_lund_id_pm() { 
  return get(&VertexProperties::lund_id_, pruned_mc_graph_); 
}

inline TruthMatcher::Graph TruthMatcher::get_reco_graph() const { return reco_graph_; }

inline TruthMatcher::IntPropertyMap TruthMatcher::get_reco_idx_pm() { 
  return get(&VertexProperties::idx_, reco_graph_); 
}

inline TruthMatcher::IntPropertyMap TruthMatcher::get_reco_lund_id_pm() { 
  return get(&VertexProperties::lund_id_, reco_graph_); 
}

inline TruthMatcher::IntPropertyMap TruthMatcher::get_reco_matched_idx_pm() { 
  return get(&VertexProperties::matched_idx_, reco_graph_); 
}

inline const std::vector<int>& TruthMatcher::get_matching() const {
  return matching_;
}

#endif
