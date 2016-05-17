#include <iostream>
#include <vector>
#include <fstream>
#include <utility>
#include <unordered_map>
#include <map>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

#include "pgstring_convert.h"
#include "CsvReader.h"
#include "ParticleTable.h"

template <typename T> 
void print(std::ostream &os, const std::vector<T> &v) {
  for (const auto &e : v) { os << e << " "; }
}

using namespace std;
using namespace boost;

struct ParticleProperties {
  int idx;
  int lund_id;
};

template <class Name>
class particle_writer {

public:
  particle_writer(Name name, const std::string &fname) 
    : name_(name), pdt_(fname) {
  }

  template <class VertexOrEdge>
    void operator()(std::ostream& out, const VertexOrEdge &v) const {
      out << "[label=\"" << pdt_.get(get(name_, v)) << "\"]";
    }

private:
  Name name_;
  ParticleTable pdt_;
};

int main() {

  // read the csv file
  int n_vertices, n_edges;
  vector<int> from, to, lund_id;
  CsvReader<> csv("recograph_adjacency.csv"); csv.next();
  //CsvReader<> csv("mcgraph_adjacency.csv"); csv.next();
  pgstring_convert(csv["n_vertices"], n_vertices);
  pgstring_convert(csv["n_edges"], n_edges);
  pgstring_convert(csv["from"], from);
  pgstring_convert(csv["to"], to);
  pgstring_convert(csv["lund_id"], lund_id);

  // define graph type
  typedef adjacency_list<listS, listS, bidirectionalS, ParticleProperties> Graph;
  typedef graph_traits<Graph>::vertex_descriptor Vertex;

  // instantiate empty graph
  Graph g;

  // add vertices
  map<int, Vertex> idx2vtx;
  for (int i = 0; i < n_vertices; ++i) {
    Vertex v = add_vertex(g);
    idx2vtx[i] = v;
  }

  // add edges
  for (int i = 0; i < n_edges; ++i) {
    add_edge(idx2vtx[from[i]], idx2vtx[to[i]], g);
  }

  // attach particle properties
  for (const auto &p : idx2vtx) {
    g[p.second].idx = p.first;
    g[p.second].lund_id = lund_id[p.first];
  }

  // print 
  typedef property_map<Graph, int ParticleProperties::*>::type NameMap;
  NameMap index = get(&ParticleProperties::idx, g);
  NameMap lundmap = get(&ParticleProperties::lund_id, g);
  boost::write_graphviz(std::cout, g, //make_label_writer(lundmap), 
                        particle_writer<NameMap>(lundmap, "cache/pdt.dat"),
                        default_writer(), default_writer(), 
                        index);

  return 0;
}
