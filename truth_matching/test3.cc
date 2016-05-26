#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <queue>
#include <stack>
#include <cmath>
#include <cassert>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/property_map/property_map.hpp>

#include <pgstring_convert.h>
#include <PsqlReader.h>

#include "ParticleGraph.h"
#include "ParticleGraphWriter.h"


struct VertexProperties {
  int idx;
  int lund_id;
};

bool is_terminal_particle(int lund_id) {
  switch (abs(lund_id)) {
    case 11:
    case 13:
    case 211:
    case 321:
    case 22:
    case 2212:
    case 2112:
      return true;
  }
  return false;
}

bool is_undetectable_particle(int lund_id) {
  switch (abs(lund_id)) {
    case 12:
    case 14:
    case 15:
    case 16:
    case 311:
      return true;
  }
  return false;
}

bool is_acceptable_photon_mother(int lund_id) {
  switch (abs(lund_id)) {
    case 111:
    case 413:
    case 423:
      return true;
  }
  return false;
}

template <typename Vertex, typename Graph, typename LundIdPropertyMap>
void remove_subtrees(Vertex s, Graph &g, LundIdPropertyMap lund_pm) {

  std::vector<Vertex> to_remove;
  std::unordered_set<Vertex> visited;

  std::queue<Vertex> q; 
  visited.insert(s); q.push(s);

  while (!q.empty()) {

    Vertex u = q.front(); q.pop();

    if (is_terminal_particle(get(lund_pm, u))) {
      typename boost::graph_traits<Graph>::out_edge_iterator oe, oe_end, next;
      std::tie(oe, oe_end) = out_edges(u, g);
      for (next = oe; oe != oe_end; oe = next) {
        ++next;
        Vertex v = target(*oe, g);
        label_for_removal(v, g, to_remove);
      }
    } else {
      typename boost::graph_traits<Graph>::out_edge_iterator oe, oe_end;
      std::tie(oe, oe_end) = out_edges(u, g);
      for (; oe != oe_end; ++oe) {
        Vertex v = target(*oe, g);
        if (visited.find(v) == visited.end()) {
          visited.insert(v); q.push(v);
        }
      }
    }
  }

  for (auto u : to_remove) {
    clear_vertex(u, g);
    remove_vertex(u, g);
  }
}

template < typename Vertex, typename Graph >
void label_for_removal(Vertex r, Graph &g, std::vector<Vertex> &to_remove) {

  std::unordered_set<Vertex> visited;

  std::queue<Vertex> q; 
  visited.insert(r); q.push(r);

  while (!q.empty()) {

    Vertex u = q.front(); q.pop();

    typename boost::graph_traits<Graph>::out_edge_iterator oe, oe_end;
    std::tie(oe, oe_end) = out_edges(u, g);
    for (; oe != oe_end; ++oe) {
      Vertex v = target(*oe, g);
      if (visited.find(v) == visited.end()) {
        visited.insert(v); q.push(v);
      }
    }

    to_remove.push_back(u);
  }
}

using Graph = boost::adjacency_list<boost::listS, boost::listS, 
                                    boost::bidirectionalS, VertexProperties>;
using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
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

  typename boost::graph_traits<Graph>::vertex_iterator vi, vi_end, next;
  typename boost::graph_traits<Graph>::in_edge_iterator ie, ie_end;
  typename boost::graph_traits<Graph>::out_edge_iterator oe, oe_end;

  // remove subtrees
  // ---------------

  Vertex s;
  for (std::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
    if (g[*vi].idx == 2) { 
      s = *vi;
      break;
    }
  }
  if (vi == vi_end) {
    throw std::runtime_error("couldn't find mc_idx 2. ");
  }

  IntPropertyMap lund_pm = get(&VertexProperties::lund_id, g);
  remove_subtrees(s, g, lund_pm);

  // rip vertices 
  // ------------

  // e+e-
  std::unordered_map<int, bool> to_rip;
  to_rip[0] = true;
  to_rip[1] = true;

  // undetectale particles
  IntPropertyMap index_pm = get(&VertexProperties::idx, g);
  lund_pm = get(&VertexProperties::lund_id, g);

  std::tie(vi, vi_end) = vertices(g);
  for (; vi != vi_end; ++vi) {
    if (is_undetectable_particle(get(lund_pm, *vi))) {
      to_rip[get(index_pm, *vi)] = true;
    }
  }

  // spurious photons
  std::tie(vi, vi_end) = vertices(g);
  for (; vi != vi_end; ++vi) {

    if (get(lund_pm, *vi) == 22) {

      std::tie(ie, ie_end) = in_edges(*vi, g);
      Vertex u = source(*ie, g);
      if (!is_acceptable_photon_mother(get(lund_pm, u))) {
        to_rip[get(index_pm, *vi)] = true;
      }

    }
  }


  // rip
  std::tie(vi, vi_end) = vertices(g);
  for (next = vi; vi != vi_end; vi = next) {
    ++next;

    if (to_rip[g[*vi].idx]) {

      std::tie(ie, ie_end) = in_edges(*vi, g);
      for (; ie != ie_end; ++ie) {
        auto u = source(*ie, g);

        std::tie(oe, oe_end) = out_edges(*vi, g);
        for (; oe != oe_end; ++oe) {
          auto v = target(*oe, g);
          add_edge(u, v, g);
        }
      }

      clear_vertex(*vi, g);
      remove_vertex(*vi, g);
    }
  }

  // print 
  ParticleGraphWriter writer("../dat/pdt.dat");
  index_pm = get(&VertexProperties::idx, g);
  lund_pm = get(&VertexProperties::lund_id, g);
  writer.print(std::cout, g, lund_pm, index_pm);

  // close database connection
  psql.close_cursor();
  psql.close_connection();

  return 0;
}
