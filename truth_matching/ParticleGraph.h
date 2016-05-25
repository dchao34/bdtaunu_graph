#ifndef _PARTICLE_GRAPH_H_
#define _PARTICLE_GRAPH_H_

#include <iostream>
#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>


template <typename AdjacencyListGraph>
void construct_graph(
    AdjacencyListGraph &g, 
    int n_vertices, int n_edges,
    const std::vector<int> &from_vertices, 
    const std::vector<int> &to_vertices) {

  using Vertex = typename boost::graph_traits<AdjacencyListGraph>::vertex_descriptor;

  // check for argument consistency
  if (from_vertices.size() != n_edges) {
    throw std::invalid_argument(
        "construct_graph(): n_vertices and from_vertices.size() must agree. "
    );
  }

  if (to_vertices.size() != from_vertices.size()) {
    throw std::invalid_argument(
        "construct_graph(): from_vertices.size() must agree with to_vertices.size(). "
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
    g[u].idx = i;
  }

  // insert edges
  for (int i = 0; i < n_edges; ++i) {
    boost::add_edge(vertex_map[from_vertices[i]], 
                    vertex_map[to_vertices[i]], g);
  }
}


template <typename AdjacencyListGraph>
void populate_lund_id(
    AdjacencyListGraph &g, 
    const std::vector<int> &lund_id) {

  // check for argument consistency
  if (lund_id.size() != num_vertices(g)) {
    throw std::invalid_argument(
        "populate_lund_id(): lund_id.size() must agree with num_vertices(g). "
    );
  }

  // populate attributes
  typename boost::graph_traits<AdjacencyListGraph>::vertex_iterator vi, vi_end;
  for (std::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
    g[*vi].lund_id = lund_id[g[*vi].idx];
  }

}


#endif
