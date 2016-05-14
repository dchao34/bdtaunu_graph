#include <iostream>
#include <vector>
#include <fstream>
#include <utility>
#include <unordered_map>

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

struct Particle {
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

  int mclen;
  vector<int> mclund, daulen, dauidx;
  CsvReader<> csv("testing.csv"); csv.next();
  pgstring_convert(csv["mclen"], mclen);
  pgstring_convert(csv["mclund"], mclund);
  pgstring_convert(csv["daulen"], daulen);
  pgstring_convert(csv["dauidx"], dauidx);

  typedef adjacency_list<listS, listS, bidirectionalS, Particle> Graph;
  typedef graph_traits<Graph>::vertex_descriptor Vertex;

  Graph g;

  std::unordered_map<int, Vertex> verts;
  for (int i = 0; i < mclen; ++i) {
    Vertex v = add_vertex(g);
    verts[i] = v;
    g[v].idx = i;
    g[v].lund_id = mclund[i];
  }

  for (int i = 0; i < mclen; ++i) {
    if (daulen[i] <= 0 || dauidx[i] <= 0) { continue; }
    for (int j = dauidx[i]; j < dauidx[i]+daulen[i]; ++j) {
      add_edge(verts[i], verts[j], g);
    }
  }

  typedef property_map<Graph, int Particle::*>::type NameMap;
  NameMap name = get(&Particle::lund_id, g);
  NameMap index = get(&Particle::idx, g);
  boost::write_graphviz(std::cout, g, make_label_writer(name), 
                        default_writer(), default_writer(), 
                        index);

  return 0;
}
