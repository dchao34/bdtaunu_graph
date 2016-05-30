BEGIN;

CREATE TABLE truth_match_sp1237 (
  eid integer,
  pruned_mc_from_vertices integer[],
  pruned_mc_to_vertices integer[],
  matching integer[],
  y_match_status integer[],
  exist_matched_y integer
);

\copy truth_match_sp1237 FROM 'truth_match_sp1237.csv' WITH CSV HEADER;

DROP MATERIALIZED VIEW truth_match_input_sp1237;

COMMIT;
