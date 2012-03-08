
#include <string>
#include <list>
#include "../client/WebAPI.h"
using namespace std;

class ifdh {
   public:
        // general copy
        int cp(string src, string dest);

	// file input
	string fetchInput( string src_uri );

	// file output
	int addOutputFile(string filename);
	int copyBackOutput(string dest_dir);

	// logging
	int log( string message );
	int enter_state( string state );
	int leave_state( string state );

	//datasets
	int createDefinition(string baseuri, string name, string dims, string user);
	int deleteDefinition(string baseuri, string name);
	string describeDefinition(string baseuri, string name);
	list<string> translateConstraints(string baseuri, string dims);

	// files
	list<string> locateFile(string baseuri, string name);
	string getMetadata(string baseuri, string name);

	//
	string dumpStation(string baseuri, string name, string what = "all");

	// projects
	string startProject(string baseuri, string name, string station,  string defname_or_id,  string user,  string group);
	string findProject(string baseuri, string name, string station);

	string establishProcess(string baseuri, string appname, string appversion, string location, string user, string appfamily = "", string description = "", int filelimit = -1);
	string getNextFile(string projecturi, string processid);
	string updateFileStatus(string projecturi, string processid, string filename, string status);
        int endProcess(string projecturi, string processid);
        string dumpProcess(string projecturi, string processid);
	int setStatus(string projecturi, string processid, string status);
        int endProject(string projecturi);
        void cleanup();
};
