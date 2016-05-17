#ifndef _RECO_INDEXER_H_
#define _RECO_INDEXER_H_

#include <string>
#include <vector>
#include <unordered_map>

class RecoIndexer {

  public:

    // construct an indexer with the following properties:
    // + the block names are specified by `block_names`. 
    // + the ordering of the block names are the same as those in `block_names`.
    // + the maximum block sizes are specified in `max_block_sizes` assuming
    //   that the block order is the same as `block_names`.
    RecoIndexer(const std::vector<std::string> &block_names,
                const std::vector<int> &max_block_sizes);

    // set the block sizes. the ordering in `block_sizes` is assumed
    // to agree with those specied when `this` object was constructed. 
    void set_block_sizes(const std::vector<int> &block_sizes);

    // total size of the current global index
    int total_size() const;

    // return the current global start index for the block `block_name`
    int start_index(const std::string &block_name) const;

    // return the current block size for block `block_name`
    int block_size(const std::string &block_name) const;

    // return the global index given the 
    // local index `idx` within the block `block_name`
    int global_index(const std::string &block_name, int idx) const;

    // return true if any of the blocks are at full capacity
    bool has_full_block() const;

  private:

    // maps block names to cache indices
    std::unordered_map<std::string, int> name2blockidx_;

    // caches
    std::vector<int> max_block_size_;
    std::vector<int> start_index_;
    std::vector<int> block_size_;

};

inline int RecoIndexer::total_size() const {
  return start_index_.back() + block_size_.back();
}

inline int RecoIndexer::start_index(const std::string &block_name) const {
  return start_index_[name2blockidx_.at(block_name)];
}

inline int RecoIndexer::block_size(const std::string &block_name) const {
  return block_size_[name2blockidx_.at(block_name)];
}

#endif
