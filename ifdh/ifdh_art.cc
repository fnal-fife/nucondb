#include "ifdh_art.h"

namespace ifdh_ns {

// ART Service constructor -- currently does nothing.
//
//
ifdh_art::ifdh_art( fhicl::ParameterSet const & cfg, art::ActivityRegistry &r) {
  ;
}

}

#ifdef DEFINE_ART_SERVICE
DEFINE_ART_SERVICE(ifdh_ns::ifdh_art);
#endif
