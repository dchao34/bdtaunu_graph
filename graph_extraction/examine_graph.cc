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

void print_usage(std::ostream &os) {
  os << "usage: ./examine_graph adjacency_fname.csv record_index" << std::endl;
  os << "adjacency_fname: first line is the title. requires these fields: ";
  os << "n_vertices,n_edges,from_vertices,to_vertices,lund_id " << std::endl;
  os << "record_index: range from 0 to the total number of records. " << std::endl;
}

int main(int argc, char **argv) {

  // read command line
  if (argc < 2 || argc > 3) { print_usage(std::cerr); return 1; }

  std::string fname(argv[1]);
  size_t row_index = 0;
  if (argc == 3) { row_index = stoull(argv[2]); }

  // read the csv file
  CsvReader<> csv(fname); 
  bool valid_record = true;
  for (size_t i = 0; i <= row_index && (valid_record=csv.next()); ++i) ;

  if (!valid_record) { 
    std::cerr << "file does not contain at least ";
    std::cerr << (row_index+1) << " record(s)... " << std::endl;
    return 1;
  }

  // load the columns
  int n_vertices, n_edges;
  vector<int> from, to, lund_id;
  pgstring_convert(csv["n_vertices"], n_vertices);
  pgstring_convert(csv["n_edges"], n_edges);
  pgstring_convert(csv["from_vertices"], from);
  pgstring_convert(csv["to_vertices"], to);
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
  boost::write_graphviz(std::cout, g, 
                        particle_writer<NameMap>(lundmap, "../dat/pdt.dat"),
                        default_writer(), default_writer(), 
                        index);

  return 0;
}
