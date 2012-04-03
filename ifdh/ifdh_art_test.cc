#include "ifdh_art.h"
#include <iostream>

namespace art {
  class ActivityRegistry {
       int _dummy;
  };
}
namespace fhicl {
  class ParameterSet {
       int _dummy;
  };
}

main() {
    fhicl::ParameterSet cfg; 
    art::ActivityRegistry r;
    double ehmgpr;
    double when = 1323722800.0;

    ifdh_art ifdh(cfg, r);

    std::cout << "found: " << ifdh.locateFile("MV_00003142_0014_numil_v09_1105080215_RawDigits_v1_linjc.root")[0];
}
