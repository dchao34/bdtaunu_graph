#include <iostream>
#include "ParticleTable.h"

int main() {
  ParticleTable particle_table("pdt.dat");
  std::cout << "Upsilon(4S)" << " " << particle_table.get("Upsilon(4S)") << std::endl;
  std::cout << "B+" << " " << particle_table.get("B+") << std::endl;
  std::cout << "B0" << " " << particle_table.get("B0") << std::endl;
  std::cout << "D*+" << " " << particle_table.get("D*+") << std::endl;
  std::cout << "D*0" << " " << particle_table.get("D*0") << std::endl;
  std::cout << "D0" << " " << particle_table.get("D0") << std::endl;
  std::cout << "D+" << " " << particle_table.get("D+") << std::endl;
  std::cout << "K_S0" << " " << particle_table.get("K_S0") << std::endl;
  std::cout << "rho+" << " " << particle_table.get("rho+") << std::endl;
  std::cout << "pi0" << " " << particle_table.get("pi0") << std::endl;
  std::cout << "K+" << " " << particle_table.get("K+") << std::endl;
  std::cout << "pi+" << " " << particle_table.get("pi+") << std::endl;
  std::cout << "e+" << " " << particle_table.get("e+") << std::endl;
  std::cout << "mu+" << " " << particle_table.get("mu+") << std::endl;
  std::cout << "gamma" << " " << particle_table.get("gamma") << std::endl;
  return 0;
}
