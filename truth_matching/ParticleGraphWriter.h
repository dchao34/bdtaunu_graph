#ifndef _PARTICLE_GRAPH_WRITER_H_
#define  _PARTICLE_GRAPH_WRITER_H_

#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/property_map/property_map.hpp>

#include <ParticleTable.h>

// vertex or writer for arbitrary quantities 
template <typename PropertyMapT>
class BasicVertexWriter {

  public:
    BasicVertexWriter(PropertyMapT pm) : pm_(pm) {}

    void set_property(
        const std::string &name, 
        const std::string &value) {
      properties_[name] = value; 
    }

    template <typename VertexT>
      void operator()(std::ostream& out, const VertexT &v) const {
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
inline BasicVertexWriter<PropertyMapT> 
make_basic_graph_writer(PropertyMapT pm) {
  return BasicVertexWriter<PropertyMapT>(pm);
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


// TruthMatchGraphPrinter
// ----------------------

class TruthMatchGraphPrinter {

  private:

  // the helper classes are for implementation. skip to API for usage. 

  // vertex writer class. models boost graph library's PropertyWriter.
  template <typename LundIdPropertyMapT, typename VertexIndexPropertyMapT>
  class TruthMatchVertexWriter {

    public:

      TruthMatchVertexWriter(
          const std::unordered_set<int> &matched_indices, 
          LundIdPropertyMapT lund_pm, 
          VertexIndexPropertyMapT idx_pm, 
          const ParticleTable &pdt,
          const std::unordered_map<std::string, std::string> &vertex_properties,
          const std::unordered_map<std::string, std::string> &matched_vertex_properties)
          : matched_indices_(matched_indices), 
            lund_pm_(lund_pm), idx_pm_(idx_pm), pdt_(pdt), 
            vertex_properties_(vertex_properties),
            matched_vertex_properties_(matched_vertex_properties) {}

      template <typename VertexT>
        void operator()(std::ostream& out, const VertexT &v) const {
          out << "[";
          out << "label=\"" << pdt_.get(get(lund_pm_, v)) << "\"";

          std::string s;
          if (matched_indices_.find(get(idx_pm_, v)) != matched_indices_.end()) {
            assemble_property_string(s, matched_vertex_properties_);
          } else {
            assemble_property_string(s, vertex_properties_);
          }

          out << s;
          out << "]";

        }

    private:
      void assemble_property_string(std::string &s, 
          const std::unordered_map<std::string, std::string> &vertex_properties) const {
        s.clear();
        for (const auto &p : vertex_properties) {
          s += ",";
          s += p.first + "=";
          s += "\"" + p.second + "\"";
        }
      }

    private:
      const std::unordered_set<int> &matched_indices_;
      LundIdPropertyMapT lund_pm_;
      VertexIndexPropertyMapT idx_pm_;
      const ParticleTable &pdt_;
      const std::unordered_map<std::string, std::string> &vertex_properties_;
      const std::unordered_map<std::string, std::string> &matched_vertex_properties_;

  };

  // edge writer class. models boost graph library's PropertyWriter.
  template <typename MatchedEdgePropertyMapT>
  class TruthMatchEdgeWriter {

    public:
      TruthMatchEdgeWriter(const MatchedEdgePropertyMapT &edge_pm, 
          const std::unordered_map<std::string, std::string> &edge_properties,
          const std::unordered_map<std::string, std::string> &matched_edge_properties)
        : edge_pm_(edge_pm),
          edge_properties_(edge_properties), 
          matched_edge_properties_(matched_edge_properties) {}

      template <typename EdgeT>
        void operator()(std::ostream& out, const EdgeT &e) const {
          out << "[";

          std::string s;
          if (get(edge_pm_, e)) {
            assemble_property_string(s, matched_edge_properties_);
          } else {
            assemble_property_string(s, edge_properties_);
          }

          out << s;
          out << "]";

        }
    private:
      void assemble_property_string(std::string &s, 
          const std::unordered_map<std::string, std::string> &edge_properties) const {
        s.clear();
        for (const auto &p : edge_properties) {
          s += p.first + "=";
          s += "\"" + p.second + "\"";
          s += ",";
        }
        s.pop_back();
      }

    private: 
      const MatchedEdgePropertyMapT &edge_pm_;
      const std::unordered_map<std::string, std::string> &edge_properties_;
      const std::unordered_map<std::string, std::string> &matched_edge_properties_;
  };

  // API
  // ---

  public:

    // constructor takes the particle name lookup table
    TruthMatchGraphPrinter(const std::string &fname);

    // print method. input:
    // + os: stream to print to.
    // + g: reconstruction graph to print.
    // + matched_indices: graph vertex indices that are matched. 
    //                    a positive value means there's a match.
    // + idx_pm: vertex index property map.
    // + lund_id_pm: lund id property map.
    template <typename GraphT, 
              typename VertexIndexPropertyMapT,
              typename LundIdPropertyMapT>
    void print(std::ostream &os, GraphT g, 
                const std::vector<int> matched_indices_v,
                VertexIndexPropertyMapT idx_pm, 
                LundIdPropertyMapT lund_id_pm);

    // set graphviz properties for non-matched vertices
    void set_vertex_property(
        const std::string &name, const std::string &value) {
      vertex_properties_[name] = value; 
    }

    // set graphviz properties for matched vertices
    void set_matched_vertex_property(
        const std::string &name, const std::string &value) {
      matched_vertex_properties_[name] = value; 
    }

    // set graphviz properties for non-matched edges
    void set_edge_property(
        const std::string &name, const std::string &value) {
      edge_properties_[name] = value; 
    }

    // set graphviz properties for matched edges
    void set_matched_edge_property(
        const std::string &name, const std::string &value) {
      matched_edge_properties_[name] = value; 
    }

  private:
    ParticleTable pdt_;
    std::unordered_map<std::string, std::string> vertex_properties_;
    std::unordered_map<std::string, std::string> matched_vertex_properties_;
    std::unordered_map<std::string, std::string> edge_properties_;
    std::unordered_map<std::string, std::string> matched_edge_properties_;
};



// load the particle name lookup table and initialize default properties
TruthMatchGraphPrinter::TruthMatchGraphPrinter(const std::string &fname) : pdt_(fname) {
  vertex_properties_["color"] = "red";

  matched_vertex_properties_["color"] = "red";
  matched_vertex_properties_["style"] = "filled";
  matched_vertex_properties_["fillcolor"] = "lightskyblue";
  matched_vertex_properties_["penwidth"] = "3";

  edge_properties_["color"] = "grey";
  matched_edge_properties_["penwidth"] = "3";
}


// main printing method
template <typename GraphT, 
          typename VertexIndexPropertyMapT,
          typename LundIdPropertyMapT>
void TruthMatchGraphPrinter::print(
    std::ostream &os, GraphT g, 
    const std::vector<int> matched_indices_v,
    VertexIndexPropertyMapT idx_pm, 
    LundIdPropertyMapT lund_id_pm) {

  // initialize the set of matched vertices 
  std::unordered_set<int> matched_indices_s;
  for (int i = 0; i < matched_indices_v.size(); ++i) {
    if (matched_indices_v[i] >= 0) {
      matched_indices_s.insert(i);
    }
  }

  // initialize the set of matched edges. these are edges where both 
  // its source and target are matched vertices
  using EdgeT = typename boost::graph_traits<GraphT>::edge_descriptor;
  std::map<EdgeT, bool> edge2bool;
  boost::associative_property_map< std::map<EdgeT, bool> >
        matched_edge_pm(edge2bool);

  using EdgeIterT = typename boost::graph_traits<GraphT>::edge_iterator;
  EdgeIterT ei, ei_end;
  for (std::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
    if (matched_indices_s.find(g[source(*ei, g)].idx_) != matched_indices_s.end() &&
        matched_indices_s.find(g[target(*ei, g)].idx_) != matched_indices_s.end()) {
      edge2bool[*ei] = true;
    } else {
      edge2bool[*ei] = false;
    }
  }
  
  // draw the graph by delegating the work to the respective writers. 
  boost::write_graphviz(os, g, 

      TruthMatchVertexWriter<LundIdPropertyMapT, VertexIndexPropertyMapT>(
        matched_indices_s,
        lund_id_pm, idx_pm, pdt_, 
        vertex_properties_, matched_vertex_properties_),

      TruthMatchEdgeWriter<decltype(matched_edge_pm)>(
        matched_edge_pm, edge_properties_, matched_edge_properties_),

      boost::default_writer(), idx_pm);

  return;
}


#endif
