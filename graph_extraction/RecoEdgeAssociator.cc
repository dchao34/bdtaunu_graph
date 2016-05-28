#include <stdexcept>

#include "RecoEdgeAssociator.h"

RecoEdgeAssociator::RecoEdgeAssociator(
  int max_mothers, int max_daughters) :
  max_mothers_(max_mothers), max_daughters_(max_daughters),
  curr_mothers_(),
  mother_lund_(),
  daulund_adjacency_(), 
  dauidx_adjacency_() {}


void RecoEdgeAssociator::associate_edges(
    int n_mothers, const std::vector<int> &mother_lund, 
    const std::vector<int> &ndaus_list, 
    const std::vector<std::vector<int>> &daulund_list, 
    const std::vector<std::vector<int>> &dauidx_list) {

  // setup mothers 
  if (n_mothers > max_mothers_) { 
    throw std::out_of_range(
        "RecoEdgeAssociator::associate_edges(): "
        "n_mothers exceeded than maximum. ");
  }
  curr_mothers_ = n_mothers;

  if (mother_lund.size() != static_cast<unsigned>(curr_mothers_)) { 
    throw std::length_error(
        "RecoEdgeAssociator::associate_edges(): "
        "mother_lund.size() not equal to n_mothers. ");
  }
  mother_lund_ = mother_lund;

  // setup daughter adjacency structure 
  if (ndaus_list.size() != static_cast<unsigned>(curr_mothers_)) { 
    throw std::length_error(
        "RecoEdgeAssociator::associate_edges(): "
        "ndaus_list.size() not equal to n_mothers. ");
  }
  if (daulund_list.size() != static_cast<unsigned>(max_daughters_)) { 
    throw std::length_error(
        "RecoEdgeAssociator::associate_edges(): "
        "daulund_list.size() not equal to max_daughters. ");
  }
  if (dauidx_list.size() != static_cast<unsigned>(max_daughters_)) { 
    throw std::length_error(
        "RecoEdgeAssociator::associate_edges(): "
        "daulund_list.size() not equal to max_daughters. ");
  }

  // consider checking for daulund_list and dauidx_list consistency?
  daulund_adjacency_ = std::vector<std::vector<int>>(curr_mothers_);
  dauidx_adjacency_ = std::vector<std::vector<int>>(curr_mothers_);
  for (int i = 0; i < curr_mothers_; ++i) {
    for (int j = 0; j < ndaus_list[i]; ++j) {
      daulund_adjacency_[i].push_back(daulund_list[j][i]);
      dauidx_adjacency_[i].push_back(dauidx_list[j][i]);
    }
  }

}
