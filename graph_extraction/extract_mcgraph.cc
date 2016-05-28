#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "pgstring_convert.h"
#include "PsqlReader.h"

namespace po = boost::program_options;

void extract_mcgraph(const po::variables_map &vm);

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
             "name of the table to extract graph information. ")
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
      std::cout << "Usage: ./extract_mcgraph ";
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
    extract_mcgraph(vm);

  } catch(std::exception& e) {

    std::cerr << "error: " << e.what() << "\n";
    return 1;

  } catch(...) {

    std::cerr << "Exception of unknown type!\n";
    return 1;
  }

  return 0;
}

void extract_mcgraph(const po::variables_map &vm) {
  
  // open database connection
  std::string dbname = vm["dbname"].as<std::string>();
  std::string table_name = vm["table_name"].as<std::string>();
  int cursor_fetch_size = vm["cursor_fetch_size"].as<int>();

  PsqlReader psql;
  psql.open_connection("dbname=" + dbname);
  psql.open_cursor(table_name,
      { "eid", "mclen", "daulen", "dauidx", "mclund" }, 
      cursor_fetch_size);

  // open output file and write title line
  std::string output_fname = vm["output_fname"].as<std::string>();
  std::ofstream fout; fout.open(output_fname);
  fout << "eid,n_vertices,n_edges,";
  fout << "from_vertices,to_vertices,lund_id" << std::endl;

  int eid;
  int mclen;
  std::vector<int> mclund, daulen, dauidx;

  int n_vertices, n_edges;
  std::vector<int> from_vertices, to_vertices;

  size_t n_records = 0;
  while (psql.next()) {
    ++n_records;

    pgstring_convert(psql.get("eid"), eid);
    pgstring_convert(psql.get("mclen"), mclen);
    pgstring_convert(psql.get("daulen"), daulen);
    pgstring_convert(psql.get("dauidx"), dauidx);
    pgstring_convert(psql.get("mclund"), mclund);

    n_vertices = mclen;

    n_edges = 0; 
    from_vertices.clear(); to_vertices.clear();
    for (int i = 0; i < mclen; ++i) {
      if (daulen[i] <= 0 || dauidx[i] <= 0) { continue; }
      for (int j = dauidx[i]; j < dauidx[i]+daulen[i]; ++j) {
        from_vertices.push_back(i);
        to_vertices.push_back(j);
        ++n_edges;
      }
    }

    fout << eid << ",";
    fout << n_vertices << ",";
    fout << n_edges << ",";
    fout << vector2pgstring(from_vertices) << ",";
    fout << vector2pgstring(to_vertices) << ",";
    fout << vector2pgstring(mclund);
    fout << std::endl;

  }

  // close output file
  fout.close();

  // close database connection
  psql.close_cursor();
  psql.close_connection();

  std::cout << "processed " << n_records << " rows. " << std::endl;

}
