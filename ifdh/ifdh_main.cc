#include "ifdh.h"
#include <iostream>
#include <string.h>
#include <unistd.h>

using namespace std;

main(int argc, char **argv) {
    ifdh i;

    if (argc >1 && 0 == strcmp(argv[1], "--regression")) {
	  string minerva_base("http://samweb-minerva.fnal.gov:20004/sam/minerva/api");
	  WebAPI::_debug = 1;
	  cout << "found it at:" <<
	  i.locateFile(minerva_base, "MV_00003142_0014_numil_v09_1105080215_RawDigits_v1_linjc.root").front();
	  cout << "definition is:" <<
	  i.describeDefinition(minerva_base, "mwm_test_1");
          return 0;
     }
}

/*
cp(string src, string dest);
fetchInput( string src_uri );
addOutputFile(string filename);
copyBackOutput(string dest_dir);
log( string message );
enter_state( string state );
leave_state( string state );
createDefinition(string baseuri, string name, string dims, string user);
deleteDefinition(string baseuri, string name);
describeDefinition(string baseuri, string name);
translateConstraints(string baseuri, string dims);
locateFile(string baseuri, string name);
getMetadata(string baseuri, string name);
dumpStation(string baseuri, string name, string what = "all");
startProject(string baseuri, string name, string station,  string defname_or_id,  string user,  string group);
findProject(string baseuri, string name, string station);
establishProcess(string baseuri, string appname, string appversion, string location, string user, string appfamily = "", string description = "", int filelimit = -1);
getNextFile(string projecturi, string processid);
updateFileStatus(string projecturi, string processid, string filename, string status);
endProcess(string projecturi, string processid);
dumpProcess(string projecturi, string processid);
setStatus(string projecturi, string processid, string status);
endProject(string projecturi);
cleanup();
*/
