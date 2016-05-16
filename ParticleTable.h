#ifndef _PARTICLE_TABLE_H_
#define _PARTICLE_TABLE_H_

#include <iostream>
#include <fstream>
#include <string>
#include <boost/bimap.hpp>

class ParticleTable {

  private:
    using bm_type = boost::bimap<std::string, int>;
    using left_type = bm_type::left_map;
    using right_type = bm_type::right_map;

  public:
    ParticleTable(const std::string &fname) 
      : table_(), name2id_(table_.left), id2name_(table_.right) {

      std::ifstream fin(fname);
      std::string name; int id;
      while (fin >> name >> id) {
        table_.insert(bm_type::value_type(name, id));
      }
    }

    int get(const std::string &name) const { return name2id_.at(name); }
    std::string get(int id) const { return id2name_.at(id); }

  private:
    bm_type table_;
    left_type &name2id_;
    right_type &id2name_;

};

#endif
