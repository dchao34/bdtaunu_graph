#include <iostream>
#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

#include <pgstring_convert.h>
#include <PsqlReader.h>
#include "ParticleGraphWriter.h"

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


struct VertexProperties {
  int idx;
  int lund_id;
};

using Graph = boost::adjacency_list<boost::listS, boost::listS, 
                                    boost::bidirectionalS, VertexProperties>;
using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
using Edge = typename boost::graph_traits<Graph>::edge_descriptor;

int main() {

  // open database connection and populate fields
  PsqlReader psql;
  psql.open_connection("dbname=testing");
  psql.open_cursor(
      "mcgraph", 
      { "eid", "n_vertices", "n_edges", 
        "from_vertices", "to_vertices", "lund_id" });


  int eid;
  int n_vertices, n_edges;
  std::vector<int> to_vertices, from_vertices, lund_id;

  psql.next();

  pgstring_convert(psql.get("eid"), eid);
  pgstring_convert(psql.get("n_vertices"), n_vertices);
  pgstring_convert(psql.get("n_edges"), n_edges);
  pgstring_convert(psql.get("from_vertices"), from_vertices);
  pgstring_convert(psql.get("to_vertices"), to_vertices);
  pgstring_convert(psql.get("lund_id"), lund_id);

  // build and analyze graph
  Graph g;
  construct_graph(g, n_vertices, n_edges, from_vertices, to_vertices);
  populate_lund_id(g, lund_id);


  // print 
  typedef boost::property_map<Graph, int VertexProperties::*>::type NameMap;
  NameMap index = get(&VertexProperties::idx, g);
  NameMap lundmap = get(&VertexProperties::lund_id, g);
  ParticleGraphWriter writer("../dat/pdt.dat");
  writer.print(std::cout, g, lundmap, index);

  // close database connection
  psql.close_cursor();
  psql.close_connection();

  return 0;
}
