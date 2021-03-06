BEGIN;

CREATE TABLE truth_match_sp998 (
  eid integer,
  pruned_mc_from_vertices integer[],
  pruned_mc_to_vertices integer[],
  matching integer[],
  y_match_status integer[],
  exist_matched_y integer
);

\copy truth_match_sp998 FROM 'truth_match_sp998.csv' WITH CSV HEADER;

DROP MATERIALIZED VIEW truth_match_input_sp998;

CREATE INDEX ON truth_match_sp998 (eid);

COMMIT;
