#include "RecoIndexer.h"

RecoIndexer::RecoIndexer(
    const std::vector<std::string> &block_names, 
    const std::vector<int> &max_block_sizes) 
  : name2blockidx_(),
    max_block_size_(max_block_sizes), 
    start_index_(max_block_sizes.size()), 
    block_size_(max_block_sizes.size()) {

  if (block_names.size() == 0) { 
    throw std::length_error(
        "RecoIndexer::RecoIndexer(): must have non-zero blocks. ");
  }

  if (block_names.size() != max_block_sizes.size()) { 
    throw std::length_error(
        "RecoIndexer::RecoIndexer(): block_names and max_block_sizes " 
        "must have the same size. ");
  }

  // cache order depends on the order of the block_names argument. 
  for (size_t i = 0; i < block_names.size(); ++i) { 
    name2blockidx_[block_names[i]] = i;
  }
}

void RecoIndexer::set_block_sizes(const std::vector<int> &block_sizes) {

  if (block_sizes.size() != block_size_.size()) { 
    throw std::length_error(
        "RecoIndexer::set_block_sizes(): block size mismatch. ");
  }

  // update the block size cache
  block_size_ = block_sizes;

  // update starting indices
  start_index_[0] = 0;
  for (size_t i = 1; i < block_sizes.size(); ++i) {
    start_index_[i] = start_index_[i-1] + block_size_[i-1];
  }

}


int RecoIndexer::global_index(const std::string &block_name, int idx) const {
  int block_idx = name2blockidx_.at(block_name);
  if (idx >= block_size_[block_idx]) {
    throw std::out_of_range(
        "RecoIndexer::global_index(): index exceeded maximum. ");
  }
  return start_index_[block_idx] + idx;
}

bool RecoIndexer::has_full_block() const {
  for (size_t i = 0; i < block_size_.size(); ++i) {
    if (block_size_[i] >= max_block_size_[i]) { return true; }
  }
  return false;
}
