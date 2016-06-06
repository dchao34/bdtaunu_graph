BEGIN;

CREATE TABLE truth_match (
  eid integer,
  pruned_mc_from_vertices integer[],
  pruned_mc_to_vertices integer[],
  matching integer[],
  y_match_status integer[],
  exist_matched_y integer
);

\copy truth_match FROM 'truth_match.csv' WITH CSV HEADER;

DROP MATERIALIZED VIEW truth_match_input;

CREATE INDEX ON truth_match (eid);

COMMIT;
