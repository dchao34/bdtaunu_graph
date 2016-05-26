#ifndef _TRUTH_MATCHER_H_
#define _TRUTH_MATCHER_H_

#include <vector>

#include <boost/graph/adjacency_list.hpp>


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

    using Vertex = boost::graph_traits<Graph>::vertex_descriptor;
    using VertexIter = boost::graph_traits<Graph>::vertex_iterator;

    using IntPropertyMap = boost::property_map<
      Graph, int VertexProperties::*>::type;

  public:

    TruthMatcher();
    ~TruthMatcher();

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

    Graph get_mc_graph() const;
    Graph get_reco_graph() const;

    // get property maps. 
    // prefer to have const... but need to think of a way. to fix!
    IntPropertyMap get_mc_idx_pm();
    IntPropertyMap get_mc_lund_id_pm();
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

  private:
    Graph mc_graph_;
    Graph reco_graph_;
                      
};

inline TruthMatcher::Graph TruthMatcher::get_mc_graph() const { return mc_graph_; }

inline TruthMatcher::IntPropertyMap TruthMatcher::get_mc_idx_pm() { 
  return get(&VertexProperties::idx_, mc_graph_); 
}

inline TruthMatcher::IntPropertyMap TruthMatcher::get_mc_lund_id_pm() { 
  return get(&VertexProperties::lund_id_, mc_graph_); 
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


#endif
