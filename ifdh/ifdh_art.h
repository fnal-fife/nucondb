#include "ifdh.h"

// ART bits...
namespace art {
  class ActivityRegistry;   // declaration only
  class ModuleDescription;  // declaration only
  class Run;		    // declaration only
}
namespace fhicl {
  class ParameterSet;       // declaration only
}


namespace ifdh_ns {

class ifdh_art : public ifdh {
public:
       
        // ART constructor...
        ifdh_art( fhicl::ParameterSet const & cfg, art::ActivityRegistry &r);

};

}

using namespace ifdh_ns;
