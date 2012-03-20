
#include <string>
#include <vector>
#include "../client/WebAPI.h"

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

class ifdh {
       
        std::string _baseuri;

   public:
        static int _debug;

        // ART constructor...
        ifdh( fhicl::ParameterSet const & cfg, art::ActivityRegistry &r);

        // generic constructor...
        ifdh() {;}

        ifdh(std::string baseuri) :  _baseuri(baseuri) {;}

        // general copy
        int cp(std::string src, std::string dest);

	// file input
	std::string fetchInput( std::string src_uri );

	// file output
	int addOutputFile(std::string filename);
	int copyBackOutput(std::string dest_dir);

	// logging
	int log( std::string message );
	int enterState( std::string state );
	int leaveState( std::string state );

	//datasets
	int createDefinition( std::string name, std::string dims, std::string user, std::string group);
	int deleteDefinition( std::string name);
	std::string describeDefinition( std::string name);
	std::vector<std::string> translateConstraints( std::string dims);

	// files
	std::vector<std::string> locateFile( std::string name);
	std::string getMetadata( std::string name);

	//
	std::string dumpStation( std::string name, std::string what = "all");

	// projects
	std::string startProject( std::string name, std::string station,  std::string defname_or_id,  std::string user,  std::string group);
	std::string findProject( std::string name, std::string station);

	std::string establishProcess( std::string appname, std::string appversion, std::string location, std::string user, std::string appfamily = "", std::string description = "", int filelimit = -1);
	std::string getNextFile(std::string projecturi, std::string processid);
	std::string updateFileStatus(std::string projecturi, std::string processid, std::string filename, std::string status);
        int endProcess(std::string projecturi, std::string processid);
        std::string dumpProcess(std::string projecturi, std::string processid);
	int setStatus(std::string projecturi, std::string processid, std::string status);
        int endProject(std::string projecturi);
        int cleanup();
};

}

using namespace ifdh_ns;
