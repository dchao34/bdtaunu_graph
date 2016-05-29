BEGIN;

CREATE TEMPORARY TABLE mcgraph (
  eid integer, 
  n_vertices integer,
  n_edges integer,
  from_vertices integer[],
  to_vertices integer[],
  lund_id integer[]
) ON COMMIT DROP;

CREATE TEMPORARY TABLE recograph (
  eid integer, 
  n_vertices integer,
  n_edges integer,
  from_vertices integer[],
  to_vertices integer[],
  lund_id integer[], 
  y_reco_idx integer[],
  b_reco_idx integer[],
  d_reco_idx integer[],
  c_reco_idx integer[],
  h_reco_idx integer[],
  l_reco_idx integer[],
  gamma_reco_idx integer[]
) ON COMMIT DROP;

\copy mcgraph FROM 'mcgraph_adjacency.csv' WITH CSV HEADER;
\copy recograph FROM 'recograph_adjacency.csv' WITH CSV HEADER;

CREATE INDEX ON mcgraph (eid);
CREATE INDEX ON recograph (eid);

CREATE TABLE graph AS 
SELECT 
  m.eid, 
  m.n_vertices AS mc_n_vertices,
  m.n_edges AS mc_n_edges,
  m.from_vertices AS mc_from_vertices,
  m.to_vertices AS mc_to_vertices,
  m.lund_id AS mc_lund_id,
  r.n_vertices AS reco_n_vertices,
  r.n_edges AS reco_n_edges,
  r.from_vertices AS reco_from_vertices,
  r.to_vertices AS reco_to_vertices,
  r.lund_id AS reco_lund_id,
  y_reco_idx,
  b_reco_idx,
  d_reco_idx,
  c_reco_idx,
  h_reco_idx,
  l_reco_idx,
  gamma_reco_idx
FROM 
  mcgraph AS m INNER JOIN recograph AS r USING (eid);

CREATE INDEX ON graph (eid);

COMMIT;
