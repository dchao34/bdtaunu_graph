CREATE MATERIALIZED VIEW mcreco AS 
SELECT
  eid, 
  mcgraph.n_vertices AS mc_n_vertices,
  mcgraph.n_edges AS mc_n_edges,
  mcgraph.from_vertices AS mc_from_vertices,
  mcgraph.to_vertices AS mc_to_vertices,
  mcgraph.lund_id AS mc_lund_id,
  recograph.n_vertices AS reco_n_vertices,
  recograph.n_edges AS reco_n_edges,
  recograph.from_vertices AS reco_from_vertices,
  recograph.to_vertices AS reco_to_vertices,
  recograph.lund_id AS reco_lund_id,
  h_reco_idx,
  l_reco_idx,
  gamma_reco_idx,
  y_reco_idx
FROM 
  mcgraph INNER JOIN recograph using (eid);

CREATE MATERIALIZED VIEW truth_match_info AS 
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
  framework_ntuples INNER JOIN mcreco using (eid);


