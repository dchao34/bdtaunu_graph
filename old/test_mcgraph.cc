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
  particle_writer(Name name) : name_(name) {}

  template <class VertexOrEdge>
    void operator()(std::ostream& out, const VertexOrEdge &v) const {
      out << "[label=\"" << get(name_, v) << "\"]";
    }

private:
  Name name_;
};

int main() {

  // read the csv file
  int mclen;
  vector<int> mclund, daulen, dauidx;
  CsvReader<> csv("mcgraph.csv"); csv.next();
  pgstring_convert(csv["mclen"], mclen);
  pgstring_convert(csv["mclund"], mclund);
  pgstring_convert(csv["daulen"], daulen);
  pgstring_convert(csv["dauidx"], dauidx);

  // define graph type
  typedef adjacency_list<listS, listS, bidirectionalS, ParticleProperties> Graph;
  typedef graph_traits<Graph>::vertex_descriptor Vertex;

  // instantiate empty graph
  Graph g;

  // add vertices
  map<int, Vertex> idx2vtx;
  for (int i = 0; i < mclen; ++i) {
    Vertex v = add_vertex(g);
    idx2vtx[i] = v;
    //g[v].idx = i;
    //g[v].lund_id = mclund[i];
  }

  // add edges
  for (int i = 0; i < mclen; ++i) {
    if (daulen[i] <= 0 || dauidx[i] <= 0) { continue; }
    for (int j = dauidx[i]; j < dauidx[i]+daulen[i]; ++j) {
      add_edge(idx2vtx[i], idx2vtx[j], g);
    }
  }

  // attach particle properties
  for (const auto &p : idx2vtx) {
    g[p.second].idx = p.first;
    g[p.second].lund_id = mclund[p.first];
  }

  // print 
  typedef property_map<Graph, int ParticleProperties::*>::type NameMap;
  NameMap name = get(&ParticleProperties::lund_id, g);
  NameMap index = get(&ParticleProperties::idx, g);
  boost::write_graphviz(std::cout, g, make_label_writer(name), 
                        default_writer(), default_writer(), 
                        index);

  return 0;
}
