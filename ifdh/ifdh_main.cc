#include "ifdh.h"
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <string>
#include <vector>

using namespace std;

extern "C" {
 void exit(int);
}

static int
di(int i) {
   exit(i);  
   return 1;
}

static int
ds(string s) {
   cout << s << "\n";
   return 1;
}

static int
dv(vector<string> v) {
   for(int i = 0; i < v.size(); i++) {
        cout << v[i] << "\n";
   }
   return 1;
}

void
usage() {
    cout << "Command line syntax error";
}

int
ck(int i, int j) {
   if(i != j) {
      usage();
      exit(1);
      return 0;
   }
   return 1;
}


main(int argc, char **argv) {
    ifdh i;

    if (argc < 2) {
          usage();
          return 1;
    }
    //
    // regression test 
    //   
    if (argc >1 && 0 == strcmp(argv[1], "--regression")) {
	  string minerva_base("http://samweb-minerva.fnal.gov:20004/sam/minerva/api");
	  WebAPI::_debug = 1;
	  cout << "found it at:" <<
	  i.locateFile(minerva_base, "MV_00003142_0014_numil_v09_1105080215_RawDigits_v1_linjc.root").front();
	  cout << "definition is:" <<
	  i.describeDefinition(minerva_base, "mwm_test_1");
          return 0;
    }
    //
    // parse arguments
    //
    switch(argv[1][0]) {
    case 'a': 			ck(argc,3) && di(i.addOutputFile(argv[2]));				break;
    case 'c':
        switch(argv[1][1]) {
	case 'l':		ck(argc,2) && i.cleanup();					  	break;
 	case 'o':		ck(argc,3) && di(i.copyBackOutput(argv[2]));			  	break;
        case 'p':		ck(argc,4) && di(i.cp(argv[2],argv[3]));			 	break;
        case 'r':		ck(argc,5) && di(i.createDefinition(argv[2],argv[3],argv[4],argv[5]));	break;
        default: 
	    usage(); 
            return 1;
        }
        break;
   case 'd':
        switch(argv[1][6]) { //note: 7th char.........v.........
        case 'D':		ck(argc,4) && di(i.deleteDefinition(argv[2],argv[3]));			break;
	case 'b':		ck(argc,4) && ds(i.describeDefinition(argv[2],argv[3]));			break;
	case 'o':		ck(argc,4) && ds(i.dumpProcess(argv[2],argv[3]));				break;
        case 'a':		ck(argc,4) && ds(i.dumpStation(argv[2],argv[3]));				break;
        default: 
	    usage(); 
            return 1;
        }
        break;
   case 'e':
        switch(argv[1][6]) { //note: 7th char.........v.........
        case 'c':		ck(argc,4) && di(i.endProcess(argv[2],argv[3]));				break;
        case 'j':		ck(argc,3) && di(i.endProject(argv[2]));					break;
        case 't':               ck(argc,3) && di(i.enterState(argv[2]));					break;
        default: 
	    usage(); 
            return 1;
        }
        break;
    case 'f':
        switch(argv[1][1]) {
        case 'e':		ck(argc,3) && ds(i.fetchInput(argv[2]));					break;
        case 'i':		ck(argc,5) && ds(i.findProject(argv[2], argv[3], argv[4]));			break;
        default: 
	    usage(); 
            return 1;
        }
        break;
    case 'g':
        switch(argv[1][3]) {
        case 'M':		ck(argc,4) && ds(i.getMetadata(argv[2],argv[3]));				break;
	case 'N':		ck(argc,4) && ds(i.getNextFile(argv[2],argv[3]));				break;
        default: 
	    usage(); 
            return 1;
        }
        break;
    case 'l':
        switch(argv[1][2]) {
        case 'a':		ck(argc,3) && di(i.leaveState(argv[2]));					break;
        case 'c':		ck(argc,4) && dv(i.locateFile(argv[2],argv[3]));				break;
        case 'g':		ck(argc,3) && di(i.log(argv[2]));						break;
        default: 
	    usage(); 
            return 1;
        }
        break;
    case 's':
        switch(argv[1][1]) {	
        case 'e':		ck(argc,5) && di(i.setStatus(argv[2],argv[3],argv[4]));				break;
        case 't':		ck(argc,8) && ds(i.startProject(argv[2],argv[3],argv[4],argv[5],argv[6],argv[7]));
														break;
        default: 
	    usage(); 
            return 1;
        }
        break;
    case 't':		        ck(argc,4) && dv(i.translateConstraints(argv[2],argv[3]));			break;		
    case 'u':			ck(argc,6) && ds(i.updateFileStatus(argv[2], argv[3], argv[4], argv[5]));	break;
    default: 
	usage(); 
	return 1;
  
    }
}

