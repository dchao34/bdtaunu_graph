#include <iostream>
#include <vector>
#include <fstream>

#include <boost/program_options.hpp>

#include <PsqlReader.h>
#include <pgstring_convert.h>

#include "ParticleGraphWriter.h"
#include "TruthMatcher.h"

namespace po = boost::program_options;

void compute_truth_match(const po::variables_map &vm);

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
             "database name containing the truth match information. ")
        ("table_name", po::value<std::string>(), 
             "table name in the database containing the truth match inforamtion. ")
        ("pdt_fname", po::value<std::string>(), 
             "particle name lookup table file name. ")
        ("mcgraph_output", po::value<std::string>(), 
             "file name to print mc graph. ")
        ("pruned_mcgraph_output", po::value<std::string>(), 
             "file name to print pruned mc graph. ")
        ("recograph_output", po::value<std::string>(), 
             "file name to print reconstruction graph. ")
        ("truth_match_output", po::value<std::string>(), 
             "file name to print truth matched graph. ")
    ;

    po::options_description hidden("Hidden options");
    hidden.add_options()
        ("config_file", po::value<std::string>(), "name of a configuration file. ")
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
      std::cout << "Usage: ./examine_truth_match [options] config_fname" << std::endl;
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

    // compute truth match
    compute_truth_match(vm);

  } catch(std::exception& e) {

    std::cerr << "error: " << e.what() << "\n";
    return 1;

  } catch(...) {

    std::cerr << "Exception of unknown type!\n";
    return 1;
  }
  return 0;
}

void compute_truth_match(const po::variables_map &vm) {


  // open database connection and populate fields
  std::string dbname = vm["dbname"].as<std::string>();
  std::string table_name = vm["table_name"].as<std::string>();

  PsqlReader psql;
  psql.open_connection("dbname=" + dbname);
  psql.open_cursor(table_name, 
      { "eid", 
        "mc_n_vertices", "mc_n_edges", 
        "mc_from_vertices", "mc_to_vertices", "mc_lund_id",
        "reco_n_vertices", "reco_n_edges", 
        "reco_from_vertices", "reco_to_vertices", "reco_lund_id", 
        "h_reco_idx", "hmcidx", 
        "l_reco_idx", "lmcidx", 
        "gamma_reco_idx", "gammamcidx"});


  int eid;
  int mc_n_vertices, mc_n_edges;
  std::vector<int> mc_from_vertices, mc_to_vertices, mc_lund_id;
  int reco_n_vertices, reco_n_edges;
  std::vector<int> reco_from_vertices, reco_to_vertices, reco_lund_id;
  std::vector<int> h_reco_idx, hmcidx;
  std::vector<int> l_reco_idx, lmcidx;
  std::vector<int> gamma_reco_idx, gammamcidx;

  psql.next();

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

  // print graphs to file
  std::string pdt_fname = vm["pdt_fname"].as<std::string>();
  std::string mcgraph_output = vm["mcgraph_output"].as<std::string>();
  std::string pruned_mcgraph_output = vm["pruned_mcgraph_output"].as<std::string>();
  std::string recograph_output = vm["recograph_output"].as<std::string>();
  std::string truth_match_output = vm["truth_match_output"].as<std::string>();
  std::ofstream fout;

  // print mc graph
  fout.open(mcgraph_output);
  auto mcgraph_writer = make_lund_id_writer(tm.get_mc_lund_id_pm(), pdt_fname);
  mcgraph_writer.set_property("color", "blue");
  print_graph(fout, tm.get_mc_graph(), tm.get_mc_idx_pm(), mcgraph_writer);
  fout.close();

  // print pruned mc graph
  fout.open(pruned_mcgraph_output);
  auto pruned_mcgraph_writer = 
    make_lund_id_writer(tm.get_pruned_mc_lund_id_pm(), pdt_fname);
  pruned_mcgraph_writer.set_property("color", "blue");
  print_graph(fout, tm.get_pruned_mc_graph(), 
      tm.get_pruned_mc_idx_pm(), pruned_mcgraph_writer);
  fout.close();

  // print reco graph
  fout.open(recograph_output);
  auto reco_writer = make_lund_id_writer(tm.get_reco_lund_id_pm(), pdt_fname);
  reco_writer.set_property("color", "red");
  print_graph(fout, tm.get_reco_graph(), tm.get_reco_idx_pm(), reco_writer);
  fout.close();

  // print truth match
  TruthMatchGraphPrinter tm_printer(pdt_fname);
  fout.open(truth_match_output);
  tm_printer.print(fout, tm.get_reco_graph(), tm.get_matching(), 
                   tm.get_reco_idx_pm(), tm.get_reco_lund_id_pm());
  fout.close();

  // close database connection
  psql.close_cursor();
  psql.close_connection();

}
