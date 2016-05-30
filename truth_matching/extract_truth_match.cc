#include <iostream>
#include <vector>
#include <fstream>

#include <PsqlReader.h>
#include <pgstring_convert.h>

#include <boost/program_options.hpp>

#include "TruthMatcher.h"

namespace po = boost::program_options;

void extract_truth_match(const po::variables_map &vm);

int main(int argc, char **argv) {

  try {

    // define program options
    po::options_description generic("Generic options");
    generic.add_options()
        ("help,h", "produce help message")
    ;

    po::options_description config("Configuration options");
    config.add_options()
        ("dbname", po::value<std::string>(), 
             "database name. ")
        ("table_name", po::value<std::string>(), 
             "name of the table containing the truth match inputs. ")
        ("output_fname", po::value<std::string>(), 
             "output csv file name to store extracted result. ")
        ("cursor_fetch_size", po::value<int>()->default_value(5000), 
             "number of rows per cursor fetch. ")
    ;

    po::options_description hidden("Hidden options");
    hidden.add_options()
        ("config_file", po::value<std::string>(), 
             "name of a configuration file. ")
    ;

    po::options_description cmdline_options;
    cmdline_options.add(generic).add(config).add(hidden);

    po::options_description config_file_options;
    config_file_options.add(config);

    po::options_description visible;
    visible.add(generic).add(config);

    po::positional_options_description p;
    p.add("config_file", -1);

    // parse program options and configuration file
    po::variables_map vm;
    store(po::command_line_parser(argc, argv).
          options(cmdline_options).positional(p).run(), vm);
    notify(vm);

    if (vm.count("help") || !vm.count("config_file")) {
      std::cout << std::endl;
      std::cout << "Usage: ./extract_truth_match ";
      std::cout << "[options] config_fname" << std::endl;
      std::cout << visible << "\n";
      return 0;
    }

    std::ifstream fin(vm["config_file"].as<std::string>());
    if (!fin) {
      std::cout << "cannot open config file: ";
      std::cout << vm["config_file"].as<std::string>() << std::endl;
      return 0;
    }

    store(parse_config_file(fin, config_file_options), vm);
    notify(vm);

    // main routine
    extract_truth_match(vm);

  } catch(std::exception& e) {

    std::cerr << "error: " << e.what() << "\n";
    return 1;

  } catch(...) {

    std::cerr << "Exception of unknown type!\n";
    return 1;
  }

  return 0;
}

void extract_truth_match(const po::variables_map &vm) {

  // open database connection and populate fields
  std::string dbname = vm["dbname"].as<std::string>();
  std::string table_name = vm["table_name"].as<std::string>();
  int cursor_fetch_size = vm["cursor_fetch_size"].as<int>();

  PsqlReader psql;
  psql.open_connection("dbname="+dbname);
  psql.open_cursor(table_name, 
      { "eid", 
        "mc_n_vertices", "mc_n_edges", 
        "mc_from_vertices", "mc_to_vertices", "mc_lund_id",
        "reco_n_vertices", "reco_n_edges", 
        "reco_from_vertices", "reco_to_vertices", "reco_lund_id", 
        "h_reco_idx", "hmcidx", 
        "l_reco_idx", "lmcidx", 
        "gamma_reco_idx", "gammamcidx", 
        "y_reco_idx"}, cursor_fetch_size);

  int eid;
  int mc_n_vertices, mc_n_edges;
  std::vector<int> mc_from_vertices, mc_to_vertices, mc_lund_id;
  int reco_n_vertices, reco_n_edges;
  std::vector<int> reco_from_vertices, reco_to_vertices, reco_lund_id;
  std::vector<int> h_reco_idx, hmcidx;
  std::vector<int> l_reco_idx, lmcidx;
  std::vector<int> gamma_reco_idx, gammamcidx;
  std::vector<int> y_reco_idx;

  // open output file and write title line
  std::string output_fname = vm["output_fname"].as<std::string>();
  std::ofstream fout; fout.open(output_fname);
  fout << "eid,pruned_mc_from_vertices,pruned_mc_to_vertices,";
  fout << "matching,y_match_status,exist_matched_y" << std::endl;

  // main loop
  size_t n_records = 0;
  while (psql.next()) {

    ++n_records;

    // load record information
    pgstring_convert(psql.get("eid"), eid);
    pgstring_convert(psql.get("mc_n_vertices"), mc_n_vertices);
    pgstring_convert(psql.get("mc_n_edges"), mc_n_edges);
    pgstring_convert(psql.get("mc_from_vertices"), mc_from_vertices);
    pgstring_convert(psql.get("mc_to_vertices"), mc_to_vertices);
    pgstring_convert(psql.get("mc_lund_id"), mc_lund_id);
    pgstring_convert(psql.get("reco_n_vertices"), reco_n_vertices);
    pgstring_convert(psql.get("reco_n_edges"), reco_n_edges);
    pgstring_convert(psql.get("reco_from_vertices"), reco_from_vertices);
    pgstring_convert(psql.get("reco_to_vertices"), reco_to_vertices);
    pgstring_convert(psql.get("reco_lund_id"), reco_lund_id);
    pgstring_convert(psql.get("h_reco_idx"), h_reco_idx);
    pgstring_convert(psql.get("hmcidx"), hmcidx);
    pgstring_convert(psql.get("l_reco_idx"), l_reco_idx);
    pgstring_convert(psql.get("lmcidx"), lmcidx);
    pgstring_convert(psql.get("gamma_reco_idx"), gamma_reco_idx);
    pgstring_convert(psql.get("gammamcidx"), gammamcidx);
    pgstring_convert(psql.get("y_reco_idx"), y_reco_idx);

    
    // compute truth match
    TruthMatcher tm;
    tm.set_graph(
        mc_n_vertices, mc_n_edges,
        mc_from_vertices, mc_to_vertices,
        mc_lund_id, 
        reco_n_vertices, reco_n_edges,
        reco_from_vertices, reco_to_vertices,
        reco_lund_id, 
        { h_reco_idx, l_reco_idx, gamma_reco_idx }, 
        { hmcidx, lmcidx, gammamcidx }
    );

    // compute from and to vertices of pruned mc graph
    std::vector<int> from_vertices, to_vertices;
    TruthMatcher::Graph pruned_mcgraph = tm.get_pruned_mc_graph();
    boost::graph_traits<TruthMatcher::Graph>::edge_iterator ei, ei_end;
    for (std::tie(ei, ei_end) = edges(pruned_mcgraph); ei != ei_end; ++ei) {
      from_vertices.push_back(
          pruned_mcgraph[source(*ei, pruned_mcgraph)].idx_);
      to_vertices.push_back(
          pruned_mcgraph[target(*ei, pruned_mcgraph)].idx_);
    }

    // get matching result
    std::vector<int> matching = tm.get_matching();

    // get y matched status and set indicator
    int exist_matched_y = 0;
    std::vector<int> y_match_status(y_reco_idx.size(), -1);
    for (size_t i = 0; i < y_reco_idx.size(); ++i) {
      if (matching[y_reco_idx[i]] >= 0) {
        y_match_status[i] = 1;
        exist_matched_y = 1;
      }
    }

    // write a line
    fout << eid << ",";
    fout << vector2pgstring(from_vertices) << ",";
    fout << vector2pgstring(to_vertices) << ",";
    fout << vector2pgstring(matching) << ",";
    fout << vector2pgstring(y_match_status) << ",";
    fout << exist_matched_y;
    fout << std::endl;
  }

  // close file
  fout.close();

  // close database connection
  psql.close_cursor();
  psql.close_connection();

  std::cout << "processed " << n_records << " rows. " << std::endl;

}
