#include <iostream>
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
};

using Graph = boost::adjacency_list<boost::listS, boost::listS, 
                                    boost::bidirectionalS, VertexProperties>;

using IntPropertyMap = boost::property_map<Graph, int VertexProperties::*>::type;

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
  ParticleGraphWriter writer("../dat/pdt.dat");
  IntPropertyMap index_pm = get(&VertexProperties::idx, g);
  IntPropertyMap lund_pm = get(&VertexProperties::lund_id, g);
  writer.print(std::cout, g, lund_pm, index_pm);

  // close database connection
  psql.close_cursor();
  psql.close_connection();

  return 0;
}
