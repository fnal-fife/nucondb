#include "nucondb_art.h"

nucondb_art::nucondb_art( fhicl::ParameterSet const & cfg, art::ActivityRegistry &r) {
     ;
}
        
std::unique_ptr<Folder>
nucondb_art::getFolder(std::string name, std::string url, std::string tag)  {
    std::unique_ptr<Folder> res(new Folder(name, url, tag));
    return res;
}
