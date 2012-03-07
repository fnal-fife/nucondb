
#include <string>
#include <list>
#include "../client/WebAPI.h"
use std;

class ifdh {
   public:
        // general copy
        int cp(string src_uri, dest_uri);

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
	string locateFile(string baseuri, string name);
	string getMetadata(string baseuri, string name);

	//
	string dumpStation(string baseuri, name, what = "all");

	// projects
	string startProject(string baseuri, string name, string station,  string defname_or_id,  string user,  string group);
	string findProject(string baseuri, string name, string station);

	string establishProcess(string baseuri, string appname, string appversion, string location, string user, string appfamily = "", string description = "", int filelimit = -1);
	string getNextFile(string processuri);
	string updateFileStatus(string processuri, string filename, string status)
        int endProcess(processuri);
        string dumpProcess(processuri);
	int setStatus(string processuri, string status);
};
