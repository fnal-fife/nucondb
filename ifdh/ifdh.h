
#include <string>
#include <vector>
#include "../util/WebAPI.h"
#include <stdlib.h>


namespace ifdh_ns {

class ifdh {
       
        std::string _baseuri;

   public:
        static int _debug;

        // generic constructor...

#ifdef SWIG
        ifdh(std::string baseuri = "") :  _baseuri(baseuri) {;}
#else
        ifdh(std::string baseuri = getenv("IFDH_BASE_URI")?getenv("IFDH_BASE_URI"):"") :  _baseuri(baseuri) {;}
#endif

        // general file copy using cpn or srmcp
        int cp(std::string src, std::string dest);

	// get input file to local scratch, return scratch location
	std::string fetchInput( std::string src_uri );

	// add output file to set
	int addOutputFile(std::string filename);

	// copy output file set to destination with cpn or srmcp
	int copyBackOutput(std::string dest_dir);

	// logging
	int log( std::string message );

        // log entering/leaving states
	int enterState( std::string state );
	int leaveState( std::string state );

	// make a named dataset definition from a dimension string
	int createDefinition( std::string name, std::string dims, std::string user, std::string group);
	// remove data set definition
	int deleteDefinition( std::string name);
        // describe a named dataset definition
	std::string describeDefinition( std::string name);
        // give file list for dimension string
	std::vector<std::string> translateConstraints( std::string dims);

	// locate a file
	std::vector<std::string> locateFile( std::string name);
        // get a files metadata
	std::string getMetadata( std::string name);

	// give a dump of a SAM station status
	std::string dumpStation( std::string name, std::string what = "all");

	// start a new file delivery project
	std::string startProject( std::string name, std::string station,  std::string defname_or_id,  std::string user,  std::string group);
        // find a started project
	std::string findProject( std::string name, std::string station);

        // set yourself up as a file consumer process for a project
	std::string establishProcess(std::string projecturi,  std::string appname, std::string appversion, std::string location, std::string user, std::string appfamily = "", std::string description = "", int filelimit = -1);
        // get the next file location from a project
	std::string getNextFile(std::string projecturi, std::string processid);
        // update the file status (use: transferred, skipped, or consumed)
	std::string updateFileStatus(std::string projecturi, std::string processid, std::string filename, std::string status);
        // end the process
        int endProcess(std::string projecturi, std::string processid);
        // say what the sam station knows about your process
        std::string dumpProcess(std::string projecturi, std::string processid);
        // set process status
	int setStatus(std::string projecturi, std::string processid, std::string status);
        // end the project
        int endProject(std::string projecturi);
        // clean up any tmp file stuff
        int cleanup();
};

}

using namespace ifdh_ns;
