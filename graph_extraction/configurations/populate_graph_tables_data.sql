BEGIN;

CREATE TABLE graph_data (
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
);

\copy graph_data FROM 'recograph_adjacency_data.csv' WITH CSV HEADER;

CREATE INDEX ON graph_data (eid);

ALTER TABLE graph_data RENAME COLUMN n_vertices TO reco_n_vertices;
ALTER TABLE graph_data RENAME COLUMN n_edges TO reco_n_edges;
ALTER TABLE graph_data RENAME COLUMN from_vertices TO reco_from_vertices;
ALTER TABLE graph_data RENAME COLUMN to_vertices TO reco_to_vertices;
ALTER TABLE graph_data RENAME COLUMN lund_id TO reco_lund_id;

COMMIT;
