import sys
import subprocess
import time

dbname = 'bdtaunuhad_lite'

job_suffixes = [ 
    'sp1235', 
    'sp1237', 
    'sp1005', 
    'sp998', 
    'sigmc' ]

extract_mcgraph_path = '../extract_mcgraph'
mcgraph_cfg_template = 'extract_mcgraph_{0}.cfg'

extract_recograph_path = '../extract_recograph'
recograph_cfg_template = 'extract_recograph_{0}.cfg'

sql_script_template = 'populate_graph_tables_{0}.sql'

for suffix in job_suffixes:

    print "+ begin processing {0}\n".format(suffix)
    sys.stdout.flush()
    mcgraph_cfg = mcgraph_cfg_template.format(suffix)
    recograph_cfg = recograph_cfg_template.format(suffix)
    sql_script = sql_script_template.format(suffix)
    extract_mcgraph_args = [ extract_mcgraph_path, mcgraph_cfg ]
    extract_recograph_args = [ extract_recograph_path, recograph_cfg ]
    sql_args = ["psql", "-d", dbname, "-f", sql_script ]
    start_all = time.time()

    print "  extracting mcgraph..."
    sys.stdout.flush()
    start = time.time()
    subprocess.check_call(extract_mcgraph_args)
    end = time.time()
    print "  completed in {0} seconds. \n".format(round(end-start, 2))
    sys.stdout.flush()

    print "  extracting recograph..."
    sys.stdout.flush()
    start = time.time()
    subprocess.check_call(extract_recograph_args)
    end = time.time()
    print "  completed in {0} seconds. \n".format(round(end-start, 2))
    sys.stdout.flush()

    print "  populating database..."
    sys.stdout.flush()
    start = time.time()
    subprocess.check_call(sql_args)
    end = time.time()
    print "completed in {0} seconds. \n".format(round(end-start, 2))
    sys.stdout.flush()

    print "  done. total runtime: {0} seconds \n".format(
            round(end-start_all), 2)

