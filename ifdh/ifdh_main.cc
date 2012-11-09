// generated by h_to_main.sh -- do not edit

#include "ifdh.h"

#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
using namespace std;
using namespace ifdh_util_ns;
static int di(int i)	{ exit(i);  return 1; }
static int ds(string s)	 { cout << s << "\n"; return 1; }
static int dv(vector<string> v)	{ for(size_t i = 0; i < v.size(); i++) { cout << v[i] << "\n"; } return 1; }

int
main(int argc, char **argv) { 
	ifdh i;
	try {
	if (argc > 1 && 0 == strcmp(argv[1],"cp")) di(i.cp( argv[2]?argv[2]:"", argv[3]?argv[3]:""));
	else if (argc > 1 && 0 == strcmp(argv[1],"fetchInput")) ds(i.fetchInput( argv[2]?argv[2]:""));
	else if (argc > 1 && 0 == strcmp(argv[1],"localPath")) ds(i.localPath( argv[2]?argv[2]:""));
	else if (argc > 1 && 0 == strcmp(argv[1],"addOutputFile")) di(i.addOutputFile( argv[2]?argv[2]:""));
	else if (argc > 1 && 0 == strcmp(argv[1],"copyBackOutput")) di(i.copyBackOutput( argv[2]?argv[2]:""));
	else if (argc > 1 && 0 == strcmp(argv[1],"log")) di(i.log( argv[2]?argv[2]:""));
	else if (argc > 1 && 0 == strcmp(argv[1],"enterState")) di(i.enterState( argv[2]?argv[2]:""));
	else if (argc > 1 && 0 == strcmp(argv[1],"leaveState")) di(i.leaveState( argv[2]?argv[2]:""));
	else if (argc > 1 && 0 == strcmp(argv[1],"createDefinition")) di(i.createDefinition( argv[2]?argv[2]:"", argv[3]?argv[3]:"", argv[4]?argv[4]:"", argv[5]?argv[5]:""));
	else if (argc > 1 && 0 == strcmp(argv[1],"deleteDefinition")) di(i.deleteDefinition( argv[2]?argv[2]:""));
	else if (argc > 1 && 0 == strcmp(argv[1],"describeDefinition")) ds(i.describeDefinition( argv[2]?argv[2]:""));
	else if (argc > 1 && 0 == strcmp(argv[1],"translateConstraints")) dv(i.translateConstraints( argv[2]?argv[2]:""));
	else if (argc > 1 && 0 == strcmp(argv[1],"locateFile")) dv(i.locateFile( argv[2]?argv[2]:""));
	else if (argc > 1 && 0 == strcmp(argv[1],"getMetadata")) ds(i.getMetadata( argv[2]?argv[2]:""));
	else if (argc > 1 && 0 == strcmp(argv[1],"dumpStation")) ds(i.dumpStation( argv[2]?argv[2]:"", argv[3]?argv[3]:""));
	else if (argc > 1 && 0 == strcmp(argv[1],"startProject")) ds(i.startProject( argv[2]?argv[2]:"", argv[3]?argv[3]:"", argv[4]?argv[4]:"", argv[5]?argv[5]:"", argv[6]?argv[6]:""));
	else if (argc > 1 && 0 == strcmp(argv[1],"findProject")) ds(i.findProject( argv[2]?argv[2]:"", argv[3]?argv[3]:""));
	else if (argc > 1 && 0 == strcmp(argv[1],"establishProcess")) ds(i.establishProcess( argv[2]?argv[2]:"", argv[3]?argv[3]:"", argv[4]?argv[4]:"", argv[5]?argv[5]:"", argv[6]?argv[6]:"", argv[7]?argv[7]:"", argv[8]?argv[8]:"", argv[9]?atol(argv[9]):-1));
	else if (argc > 1 && 0 == strcmp(argv[1],"getNextFile")) ds(i.getNextFile( argv[2]?argv[2]:"", argv[3]?argv[3]:""));
	else if (argc > 1 && 0 == strcmp(argv[1],"updateFileStatus")) ds(i.updateFileStatus( argv[2]?argv[2]:"", argv[3]?argv[3]:"", argv[4]?argv[4]:"", argv[5]?argv[5]:""));
	else if (argc > 1 && 0 == strcmp(argv[1],"endProcess")) di(i.endProcess( argv[2]?argv[2]:"", argv[3]?argv[3]:""));
	else if (argc > 1 && 0 == strcmp(argv[1],"dumpProcess")) ds(i.dumpProcess( argv[2]?argv[2]:"", argv[3]?argv[3]:""));
	else if (argc > 1 && 0 == strcmp(argv[1],"setStatus")) di(i.setStatus( argv[2]?argv[2]:"", argv[3]?argv[3]:"", argv[4]?argv[4]:""));
	else if (argc > 1 && 0 == strcmp(argv[1],"endProject")) di(i.endProject( argv[2]?argv[2]:""));
	else if (argc > 1 && 0 == strcmp(argv[1],"cleanup")) di(i.cleanup());
	else {

                cout << "\tifdh cp  src  dest \n\t--general file copy using cpn or srmcp\n";
                cout << "\tifdh fetchInput  src_uri \n\t--get input file to local scratch, return scratch location\n";
                cout << "\tifdh localPath  src_uri \n\t--return scratch location fetchInput would give, without copying\n";
                cout << "\tifdh addOutputFile  filename \n\t--add output file to set\n";
                cout << "\tifdh copyBackOutput  dest_dir \n\t--copy output file set to destination with cpn or srmcp\n";
                cout << "\tifdh log  message \n\t--logging \n";
                cout << "\tifdh enterState  state \n\t--log entering/leaving states\n";
                cout << "\tifdh leaveState  state \n\t--log entering/leaving states\n";
                cout << "\tifdh createDefinition  name  dims  user  group \n\t--make a named dataset definition from a dimension string\n";
                cout << "\tifdh deleteDefinition  name \n\t--remove data set definition\n";
                cout << "\tifdh describeDefinition  name \n\t--describe a named dataset definition\n";
                cout << "\tifdh translateConstraints  dims \n\t--give file list for dimension string\n";
                cout << "\tifdh locateFile  name \n\t--locate a file\n";
                cout << "\tifdh getMetadata  name \n\t--get a files metadata\n";
                cout << "\tifdh dumpStation  name  what  \n\t--give a dump of a SAM station status\n";
                cout << "\tifdh startProject  name  station  defname_or_id  user  group \n\t--start a new file delivery project\n";
                cout << "\tifdh findProject  name  station \n\t--find a started project\n";
                cout << "\tifdh establishProcess  projecturi  appname  appversion  location  user  appfamily   description   filelimit  \n\t--set yourself up as a file consumer process for a project\n";
                cout << "\tifdh getNextFile  projecturi  processid \n\t--get the next file location from a project\n";
                cout << "\tifdh updateFileStatus  projecturi  processid  filename  status \n\t--update the file status (use: transferred, skipped, or consumed)\n";
                cout << "\tifdh endProcess  projecturi  processid \n\t--end the process\n";
                cout << "\tifdh dumpProcess  projecturi  processid \n\t--say what the sam station knows about your process\n";
                cout << "\tifdh setStatus  projecturi  processid  status \n\t--set process status\n";
                cout << "\tifdh endProject  projecturi \n\t--end the project\n";
                cout << "\tifdh cleanup  \n\t--clean up any tmp file stuff\n";
		exit(1);	
	}
   } catch (WebAPIException we) {
      std::cout << "Exception:" << we.what() << std::endl;
      exit(1);
   } catch (std::logic_error le ) {
      std::cout << "Exception:" << le.what() << std::endl;
   }
}
