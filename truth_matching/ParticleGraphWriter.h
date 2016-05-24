#ifndef _PARTICLE_GRAPH_WRITER_H_
#define  _PARTICLE_GRAPH_WRITER_H_

#include <iostream>
#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

#include <ParticleTable.h>

template <class LundIdPropertyMap>
class particle_writer {

public:
  particle_writer(LundIdPropertyMap lundpm, const std::string &fname) 
    : lundpm_(lundpm), pdt_(fname) {
  }

  template <class VertexOrEdge>
    void operator()(std::ostream& out, const VertexOrEdge &v) const {
      out << "[label=\"" << pdt_.get(get(lundpm_, v)) << "\"]";
    }

private:
  LundIdPropertyMap lundpm_;
  ParticleTable pdt_;
};


class ParticleGraphWriter {
  public:
    ParticleGraphWriter(std::string pdt_fname) : pdt_fname_(pdt_fname) {}

    template <typename Graph, 
              typename LundIdPropertyMap, 
              typename VertexIndexPropertyMap>
    void print(std::ostream &os, Graph g, 
               LundIdPropertyMap lund_pm, 
               VertexIndexPropertyMap index_pm) {

      boost::write_graphviz(os, g, 
        particle_writer<LundIdPropertyMap>(lund_pm, pdt_fname_),
        boost::default_writer(), 
        boost::default_writer(), 
        index_pm);
    }

  private:
    std::string pdt_fname_;
};

#endif
