#ifndef _PARTICLE_GRAPH_WRITER_H_
#define  _PARTICLE_GRAPH_WRITER_H_

#include <iostream>
#include <vector>
#include <unordered_map>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

#include <ParticleTable.h>

// vertex or edge writer for arbitrary quantities 
template <typename PropertyMapT>
class BasicGraphWriter {

  public:
    BasicGraphWriter(PropertyMapT pm) : pm_(pm) {}

    void set_property(
        const std::string &name, 
        const std::string &value) {
      properties_[name] = value; 
    }

    template <typename VertexOrEdgeT>
      void operator()(std::ostream& out, const VertexOrEdgeT &v) const {
        out << "[";
        out << "label=\"" << get(pm_, v) << "\"";
        std::string s;
        for (const auto &p : properties_) {
          s += ",";
          s += p.first + "=";
          s += "\"" + p.second + "\"";
        }
        out << s;
        out << "]";
      }

  protected:
    PropertyMapT pm_;
    std::unordered_map<std::string, std::string> properties_;

};


// convenience function to create basic graph writer
template <typename PropertyMapT>
inline BasicGraphWriter<PropertyMapT> 
make_basic_graph_writer(PropertyMapT pm) {
  return BasicGraphWriter<PropertyMapT>(pm);
}


// special vertex writer for lund id
template <typename LundIdPropertyMapT>
class LundIdWriter {

  public:
    LundIdWriter(LundIdPropertyMapT lund_pm, 
                 const std::string &fname, 
                 bool do_name_lookup = true) 
      : lund_pm_(lund_pm), pdt_(fname), do_name_lookup_(do_name_lookup) {}

    void set_property(
        const std::string &name, 
        const std::string &value) {
      properties_[name] = value; 
    }

    template <typename VertexT>
      void operator()(std::ostream& out, const VertexT &v) const {
        out << "[";

        if (do_name_lookup_) {
          out << "label=\"" << pdt_.get(get(lund_pm_, v)) << "\"";
        } else {
          out << "label=\"" << get(lund_pm_, v) << "\"";
        }

        std::string s;
        for (const auto &p : properties_) {
          s += ",";
          s += p.first + "=";
          s += "\"" + p.second + "\"";
        }

        out << s;
        out << "]";
      }

  private:
    LundIdPropertyMapT lund_pm_;
    ParticleTable pdt_;
    bool do_name_lookup_;

    std::unordered_map<std::string, std::string> properties_;
};


// convenience function to create lund id writer
template <typename LundIdPropertyMapT>
inline LundIdWriter<LundIdPropertyMapT> 
make_lund_id_writer(
    LundIdPropertyMapT pm, 
    const std::string &fname, 
    bool do_name_lookup=true) {

  return LundIdWriter<LundIdPropertyMapT>(pm, fname, do_name_lookup);

}


// print graph to output stream. 
template <typename GraphT, 
          typename VertexIndexPropertyMapT, 
          typename VertexWriterT=boost::default_writer, 
          typename EdgeWriterT=boost::default_writer, 
          typename GraphWriterT=boost::default_writer> 
void print_graph(
    std::ostream &os, 
    GraphT g, 
    VertexIndexPropertyMapT index_pm, 
    VertexWriterT v_wtr=VertexWriterT(), 
    EdgeWriterT e_wtr=EdgeWriterT(), 
    GraphWriterT g_wtr=GraphWriterT()) {
  boost::write_graphviz(os, g, v_wtr, e_wtr, g_wtr, index_pm);
}


#endif
