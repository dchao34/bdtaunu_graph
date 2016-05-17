#ifndef _RECO_EDGE_ASSOCIATOR_H_
#define _RECO_EDGE_ASSOCIATOR_H_

#include <vector>
#include <utility>

class RecoEdgeAssociator {

  public:

    // maximum allowed candidates and daughters
    // allowed for the reconstruction block
    RecoEdgeAssociator(int max_mothers, 
                       int max_daughters);

    ~RecoEdgeAssociator() {}

    void associate_edges(
        int n_mothers, const std::vector<int> &mother_lund, 
        const std::vector<int> &ndaus_list, 
        const std::vector<std::vector<int>> &daulund_list, 
        const std::vector<std::vector<int>> &dauidx_list);

    int n_mothers() const;
    int n_daughters(int moth_idx) const;

    int mother_lund(int moth_idx) const;
    int daughter_lund(int moth_idx, int dau_idx) const;
    int daughter_idx(int moth_idx, int dau_idx) const;

    std::pair<int,int> daughter_info(int moth_idx, int dau_idx) const;

  private:

    int max_mothers_, max_daughters_;
    int curr_mothers_;

    std::vector<int> mother_lund_;
    std::vector<std::vector<int>> daulund_adjacency_;
    std::vector<std::vector<int>> dauidx_adjacency_;
};

inline int RecoEdgeAssociator::n_mothers() const { 
  return curr_mothers_; 
}

inline int RecoEdgeAssociator::n_daughters(int moth_idx) const { 
  return daulund_adjacency_[moth_idx].size();
}

inline int RecoEdgeAssociator::mother_lund(int moth_idx) const {
  return mother_lund_.at(moth_idx);
}

inline int 
RecoEdgeAssociator::daughter_lund(int moth_idx, int dau_idx) const {
  return daulund_adjacency_.at(moth_idx).at(dau_idx);
}

inline int 
RecoEdgeAssociator::daughter_idx(int moth_idx, int dau_idx) const {
  return dauidx_adjacency_.at(moth_idx).at(dau_idx);
}

inline std::pair<int,int> 
RecoEdgeAssociator::daughter_info(int moth_idx, int dau_idx) const {
  return { daughter_lund(moth_idx, dau_idx), 
           daughter_idx(moth_idx, dau_idx) };
}

#endif
