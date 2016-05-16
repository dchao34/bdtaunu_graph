#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "pgstring_convert.h"
#include "PsqlReader.h"

int main() {

  
  // open database connection
  PsqlReader psql;
  psql.open_connection("dbname=testing");
  psql.open_cursor(
      "framework_ntuples", 
      { "eid", "mclen", "daulen", "dauidx", "mclund" }, 5000);

  // open output file and write title line
  std::ofstream fout; fout.open("mcgraph_adjacency.csv");
  fout << "eid,n_vertices,n_edges,from,to,lund_id" << std::endl;

  int eid;
  int mclen;
  std::vector<int> mclund, daulen, dauidx;

  int n_vertices, n_edges;
  std::vector<int> from_vertices, to_vertices;

  while (psql.next()) {

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

  return 0;
}
