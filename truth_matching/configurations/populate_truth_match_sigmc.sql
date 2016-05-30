BEGIN;

CREATE TABLE truth_match_sigmc (
  eid integer,
  pruned_mc_from_vertices integer[],
  pruned_mc_to_vertices integer[],
  matching integer[],
  y_match_status integer[],
  exist_matched_y integer
);

\copy truth_match_sigmc FROM 'truth_match_sigmc.csv' WITH CSV HEADER;

DROP MATERIALIZED VIEW truth_match_input_sigmc;

COMMIT;
