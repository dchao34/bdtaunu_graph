#include <iostream>
#include <vector>

#include <boost/graph/adjacency_list.hpp>

#include <pgstring_convert.h>
#include <PsqlReader.h>

template <typename T> 
void print(std::ostream &os, const std::vector<T> &v) {
  for (const auto &e : v) { os << e << " "; }
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

  // open database connection
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

  Graph g;

  std::vector<Vertex> vertex_map(n_vertices);
  for (int i = 0; i < n_vertices; ++i) {
    Vertex u = boost::add_vertex(g);
    vertex_map[i] = u;
    g[u].idx = i;
    g[u].lund_id = lund_id[i];
  }

  for (int i = 0; i < n_edges; ++i) {
    boost::add_edge(vertex_map[from_vertices[i]], 
                    vertex_map[to_vertices[i]], g);
  }

  /*std::cout << n_vertices << " ";
  std::cout << n_edges << std::endl;
  print(std::cout, lund_id); std::cout << std::endl;

  for (int i = 0; i < n_edges; ++i) {
    std::cout << from_vertices[i] << " " << to_vertices[i] << std::endl;
  }
  */

  psql.close_cursor();
  psql.close_connection();

  return 0;
}
