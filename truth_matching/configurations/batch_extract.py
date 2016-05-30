import sys
import subprocess
import time

dbname = 'bdtaunuhad_lite'

job_suffixes = [ 
    #'sp1235', 
    'sp1237', 
    'sp1005', 
    'sp998', 
    'sigmc' ]

extract_truth_match_path = '../extract_truth_match'
truth_match_cfg_template = 'extract_truth_match_{0}.cfg'

prepare_sql_script_template = 'prepare_truth_match_input_{0}.sql'
populate_sql_script_template = 'populate_truth_match_{0}.sql'

for suffix in job_suffixes:

    print "+ begin processing {0}\n".format(suffix)
    sys.stdout.flush()
    prepare_sql_script = prepare_sql_script_template.format(suffix)
    prepare_sql_args = ["psql", "-d", dbname, "-f", prepare_sql_script ]
    truth_match_cfg = truth_match_cfg_template.format(suffix)
    truth_match_args = [ extract_truth_match_path, truth_match_cfg ]
    populate_sql_script = populate_sql_script_template.format(suffix)
    populate_sql_args = ["psql", "-d", dbname, "-f", populate_sql_script ]
    start_all = time.time()

    print "  preparing database inputs..."
    sys.stdout.flush()
    start = time.time()
    subprocess.check_call(prepare_sql_args)
    end = time.time()
    print "  completed in {0} seconds. \n".format(round(end-start, 2))
    sys.stdout.flush()

    print "  extracting truth match information..."
    sys.stdout.flush()
    start = time.time()
    subprocess.check_call(truth_match_args)
    end = time.time()
    print "  completed in {0} seconds. \n".format(round(end-start, 2))
    sys.stdout.flush()

    print "  populating database..."
    sys.stdout.flush()
    start = time.time()
    subprocess.check_call(populate_sql_args)
    end = time.time()
    print "  completed in {0} seconds. \n".format(round(end-start, 2))
    sys.stdout.flush()

    print "  done. total runtime: {0} seconds \n".format(
            round(end-start_all), 2)

