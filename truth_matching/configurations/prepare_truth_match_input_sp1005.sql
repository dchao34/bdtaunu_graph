CREATE MATERIALIZED VIEW truth_match_input_sp1005 AS 
SELECT
  eid, 
  mc_n_vertices,
  mc_n_edges,
  mc_from_vertices,
  mc_to_vertices,
  mc_lund_id,
  reco_n_vertices,
  reco_n_edges,
  reco_from_vertices,
  reco_to_vertices,
  reco_lund_id,
  h_reco_idx,
  l_reco_idx,
  gamma_reco_idx,
  hmcidx,
  lmcidx,
  gammamcidx, 
  y_reco_idx
FROM 
  framework_ntuples_sp1005 INNER JOIN graph_sp1005 using (eid);
