#include "nucondb_art.h"
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
    double d;

    nucondb_art ia(cfg, r);
    Folder::_debug = 1;

    try {
        std::auto_ptr<Folder> pbf( ia.getFolder("pedcal", "http://dbweb4.fnal.gov:8088/mnvcon_prd/app"));

	pbf->getNamedChannelData(
	     1300969766.0,
	     1210377216,
	     "reflect",
	     &d
	    );
    } catch (WebAPIException we) {
      std::cout << "Exception:" << &we << std::endl;
    }
 
}
