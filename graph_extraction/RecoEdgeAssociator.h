#ifndef _RECO_EDGE_ASSOCIATOR_H_
#define _RECO_EDGE_ASSOCIATOR_H_

#include <vector>
#include <utility>

// class that provides a natural way to iterate over daughter candidate 
// information for a particle type of candidate. 
//
// example  
// -------
//
// a subset of the rows declared in the tuple maker could be as follows:
//
// candidate_lund     block_name      max_daughters     max_mothers
// B+                 B               4                 400
// B0                 B               4                 400
// e+                 l               3                 100
// mu+                l               2                 100
// gamma              gamma           0                 100
//
// such a declaration indicates, for example, that all reconstructed 
// B+ and B0 candidates, as well as their anti-particle type candidates, 
// are all stored in the "B" block. Furthermore, the "B" block cannot
// have more than 400 candidates, and each candidate cannot have more 
// more than 4 daughters. 
//
// for the reconstructed candidate block "B" bta tuple maker will save the 
// following information:
//
// + nB: integer. total number of mothers in the current record. 
//
// + Blund, Bndaus, Bd1idx, ..., BdNidx, Bd1lund, ..., BdNlund:
//
//   arrays of size 'max_mother' whose first 'nB' elements are valid. 
//   N is the largest of 'max_daughters' over rows with block name "B".
//   if N were zero, the Bd1idx, ..., BdNidx, Bd1lund, ..., BdNlund 
//   would not be saved. 
//
//   To access the information for candidate i, simply index into 
//   the corresponding array; e.g. Blund[i]. i is referred to as the local
//   index. you must be sure that i < nB. 
//   
//   Blund: lund id's of the current mothers.
//   Bndaus: number of daughters associated with the current mothers.
//   Bd1lund: lund id of first daughter of the current mothers. 
//            -1 if a mother does not have a first daughter. 
//   Bd1idx: local index of the first daughter within its own 
//           reconstruction block.
//           -1 if a mother does not have a first daughter. 
//   BdNlund: defined similarly as Bd1lund.
//   BdNidx: defined similarly as Bd1lund.
//
// usage
// -----
//
// for the example above, we can use the RecoEdgeAssociator as follows:
//
// 1. construct objects to manage the "B" block:
//    
//    RecoEdgeAssociator assoc(400, 4);
//
// 2. input a setting for the record. will compute the adjacencies as a 
//    side effect. 
//    
//    assoc.associate_edges(nb, blund, bndaus, 
//                          { bd1lund, bd2lund, bd3lund, bd4lund },
//                          { bd1idx, bd2idx, bd3idx, bd4idx });
//
// 3. the following operations and patterns are valid:
//
//    // get the number of mothers
//    int n_moths = assoc.n_mothers();
//
//    // get the number of daughters associated with the `i`th mother.
//    int n_daus = assoc.n_daughters(i);
//
//    // get the lund id of the `i`th mother
//    int mlund = assoc.mother_lund(i);
//
//    // get the lund id of the `j`th daughter of the `i`th mother. 
//    // you must make sure j < n_daus. 
//    int dlund = assoc.daughter_lund(i, j);
//
//    // get the local index of the `j`th daughter within its own block. 
//    int didx = assoc.daughter_idx(i, j);
//
//    // loop over the adjacency:
//    for (int i = 0; i < n_moths; ++i) {
//      for (int j = 0; j < n_daus; ++j) {
//        assoc.daughter_lund(i, j);
//        assoc.daughter_idx(i, j);
//      }
//    }
//
// 4. (bonus tip, not part of RecoEdgeAssociator)
//    to determine which block a daughter belongs to, you need to have ready
//    a separately defined map that associates candidate lund id's with blocks:
//
//    unordered_map<int, string> lund2block;
//    lund2block[511] = "B"  // 511 is B0
//    // ... etc... 
//
//
class RecoEdgeAssociator {

  public:

    // construct using maximum allowed candidates and daughters
    // allowed for the reconstruction block
    RecoEdgeAssociator(int max_mothers, 
                       int max_daughters);

    ~RecoEdgeAssociator() {}

    // compute adjacency out of the information 
    // dictated by those given in bta tuple maker.
    void associate_edges(
        int n_mothers, const std::vector<int> &mother_lund, 
        const std::vector<int> &ndaus_list, 
        const std::vector<std::vector<int>> &daulund_list, 
        const std::vector<std::vector<int>> &dauidx_list);

    // number of mothers in the current adjacency setting
    int n_mothers() const;

    // number of daughters for a given mother in the current adjacency setting
    int n_daughters(int moth_idx) const;

    // lund id for a given mother
    int mother_lund(int moth_idx) const;

    // lund id for the `j`th the daughter of the `i`th mother. 
    int daughter_lund(int i, int j) const;

    // local block index for the `j`th the daughter of the `i`th mother. 
    int daughter_idx(int moth_idx, int dau_idx) const;

    // return a pair as if by running { daughter_lund(i,j), daughter_idx(i,j) }
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
