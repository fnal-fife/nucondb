#include "nucondb.h"
#include <memory>
// ART bits...
namespace art {
  class ActivityRegistry;   // declaration only
  class ModuleDescription;  // declaration only
  class Run;		    // declaration only
}
namespace fhicl {
  class ParameterSet;       // declaration only
}

namespace nucondb_ns {

class nucondb_art  {
       
public:
        
        std::auto_ptr<Folder> getFolder(std::string name, std::string url, std::string tag = "");

        // ART constructor...
        nucondb_art( fhicl::ParameterSet const & cfg, art::ActivityRegistry &r);
};

}

#ifdef DEFINE_ART_SERVICE
DEFINE_ART_SERVICE(nucondb_ns::nucondb_art);
#endif

// this is redundant, given the ifbeam include,  but just to be clear

using namespace nucondb_ns;
