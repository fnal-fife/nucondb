#include "ifdh.h"
#include "utils.h"
#include <string>
#include <sstream>
#include <iostream>
#include <../numsg/numsg.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

using namespace std;

namespace ifdh_ns {

int
ifdh::_debug = 0;

void
path_prepend( string s1, string s2) {
    stringstream setenvbuf;
    string curpath(getenv("PATH"));

    setenvbuf << s1 << s2 << ":" << curpath;
    setenv("PATH", setenvbuf.str().c_str(), 1);
}

//
// make sure appropriate environment stuff is set
// and improve our chances of finding cpn, globus tools, etc.
//
static void
check_env() {
    static int checked = 0;
    static const char *cpn_basedir = "/grid/fermiapp/products/common/prd/cpn";

    if (!checked) {
        checked = 1;

        // try to find cpn even if it isn't setup
        if (!getenv("CPN_DIR")) {
           if (0 == access(cpn_basedir, R_OK)) {
              for (int i = 10; i > 0; i-- ) {
                  stringstream setenvbuf;
                  setenvbuf << cpn_basedir << "/v1_" << i << "/NULL";
                  if (0 == access(setenvbuf.str().c_str(), R_OK)) {
                      setenv("CPN_DIR", setenvbuf.str().c_str(), 1);
                  }
              }
           }
        }
              
        string path(getenv("PATH"));
        char *p;

        if (0 != (p = getenv("VDT_LOCATION"))) {
            if (string::npos == path.find(p)) {
               path_prepend(p,"/globus/bin");
               path_prepend(p,"/srm-client-fermi/bin");
            }
        } else if (0 == access("/usr/bin/globus-url-copy", R_OK)) {
            if (string::npos == path.find("/usr/bin")) {
               path_prepend("/usr","/bin");
            }
        } else {
             // where is vdt?!?
             ;
        }
    }
}

string cpn_loc  = "cpn";  // just use the one in the PATH -- its a product now
string fermi_gsiftp  = "gsiftp://fg-bestman1.fnal.gov:2811";
string bestmanuri = "srm://fg-bestman1.fnal.gov:10443/srm/v2/server?SFN=";
std::string ifdh::_default_base_uri = "http://samweb.fnal.gov:8480/sam/";

string datadir() {
    stringstream dirmaker;
    string localpath;
    int res;
    
    dirmaker << (
       getenv("_CONDOR_SCRATCH_DIR")?getenv("_CONDOR_SCRATCH_DIR"):
       getenv("TMPDIR")?getenv("TMPDIR"):
       "/var/tmp"
    )
       << "/ifdh_" << getppid();

    if ( 0 != access(dirmaker.str().c_str(), W_OK) ) {
        res = mkdir(dirmaker.str().c_str(),0700);
        ifdh::_debug && cout <<  "mkdir " << dirmaker.str() << " => " << res << "\n";
    }
    localpath = dirmaker.str();
    if (localpath.substr(0,2) == "//" ){ 
        localpath = localpath.substr(1);
    }
    return localpath;
}


int
ifdh::cleanup() {
    string cmd("rm -rf ");
    cmd = cmd + datadir();
    return system(cmd.c_str());
}
// file io

extern "C" { const char *get_current_dir_name(); }

string 
ifdh::localPath( string src_uri ) {
    int baseloc = src_uri.rfind("/") + 1;
    return datadir() + "/" + src_uri.substr(baseloc);
}

string 
ifdh::fetchInput( string src_uri ) {
    stringstream cmd;
    stringstream err;
    string path;
    int res, res2;
    int p1, p2;

    if (src_uri.substr(0,7) == "file://") {
        std::vector<std::string> args;
        path = localPath( src_uri );
        args.push_back(src_uri.substr(7));
        args.push_back(path);
        cp( args );
        return path;
    }
    if (src_uri.substr(0,6) == "srm://"  ||
           src_uri.substr(0,9) == "gsiftp://") {
        cmd << (src_uri[0] == 's' ? "srmcp" : "globus-url-copy")
            << " " << src_uri 
            << " " << "file:///" << localPath( src_uri )
            << " >&2" ;
        _debug && std::cerr << "running: " << cmd.str() << "\n";
        res = system(cmd.str().c_str());
        _debug && std::cerr << "res is: " << res << "\n";
        p1 = cmd.str().rfind(" ");
        p2 = cmd.str().rfind(" ", p1-1);
        path = cmd.str().substr(p2 + 8 , p1 - p2 - 8);
        res2 = access(path.c_str(), R_OK);
        _debug && std::cerr << "access res is: " << res2 << "\n";
        if (res != 0 || res2 != 0) {
            err << "exit code: " << res << " errno: " << errno << "path: " << path << "access:" << res2;
            throw(WebAPIException("srmcp failed:", err.str() ));
        }
        _debug && std::cerr << "returning:" << path << "\n";
        _lastinput = path;
        return path;
    }
    throw(WebAPIException("Unknown uri type",src_uri.substr(0,8)));
}

// file output
//
// keep track of output filenames and last input file name
// for renameOutput, and copytBackOutput...
//
int 
ifdh::addOutputFile(string filename) {
    fstream outlog((datadir()+"/output_files").c_str(), ios_base::app|ios_base::out);
    if (!_lastinput.size() && getenv("IFDH_INPUT_FILE")) {
        _lastinput = getenv("IFDH_INPUT_FILE");
    }
    outlog << filename << " " << _lastinput << "\n";
    outlog.close();
    return 1;
}

int 
ifdh::copyBackOutput(string dest_dir) {
    stringstream cmd;
    string line;
    stringstream err;
    string file;
    size_t spos;
    int res;
    string outfiles_name;
    // int baseloc = dest_dir.find("/") + 1;
    outfiles_name = datadir()+"/output_files";
    fstream outlog(outfiles_name.c_str(), ios_base::in);
    if ( outlog.fail()) {
       return 0;
    }

    if (0 == access(dest_dir.c_str(), W_OK) && (0 == getenv("IFDH_FORCE") || getenv("IFDH_FORCE")[0] == 'c')) {
        // destination is visible, so use cpn
	cmd  << cpn_loc;
        while (!outlog.eof() && !outlog.fail()) {
            getline(outlog,line);
            spos = line.find(' ');
            if (spos != string::npos) {
	        file = line.substr(0,spos);
            } else {
                file = line;
            }
            if ( !file.size() ) {
               break;
            }
            cmd << " " << file;
        }
        cmd << " " << dest_dir;

    } else {

        struct hostent *hostp;
        std::string gftpHost("if-gridftp-");
        gftpHost.append(getexperiment());

        if ( 0 != (hostp = gethostbyname(gftpHost.c_str())) && (0 == getenv("IFDH_FORCE") || getenv("IFDH_FORCE")[0] == 'e') ) {
            //  if experiment specific gridftp host exists, use it...
            
            cmd << "globus-url-copy ";
            while (!outlog.eof() && !outlog.fail()) {
		getline(outlog, line);
		spos = line.find(' ');
		if (spos != string::npos) {
		    file = line.substr(0,spos);
		} else {
		    file = line;
		}
                if ( !file.size() ) {
                   break;
                }
		cmd << " file:///" << file;
	    }
            cmd << " ftp://" << gftpHost << dest_dir;

        } else {
            // destination is not visible, use srmcp
            
	    cmd << "srmcp";
            while (!outlog.eof() && !outlog.fail()) {
		getline(outlog, line);
		spos = line.find(' ');
		if (spos != string::npos) {
		    file = line.substr(0,spos);
		} else {
		    file = line;
		}
                if ( !file.size() ) {
                   break;
                }
		cmd << " file:///" << file;
	    }
	    cmd << " " << bestmanuri << dest_dir;
       }
    }
    res = system(cmd.str().c_str());
    if (res != 0) {
        err << "command: '" << cmd << "'exit code: " << res;
        throw(WebAPIException("copy  failed:", err.str().c_str() ));
    }
    return 1;
}

// logging
int 
ifdh::log( string message ) {
  if (!numsg::getMsg()) {
      numsg::init(getexperiment(),1);
  }
  numsg::getMsg()->printf("ifdh: %s", message.c_str());
  return 0;
}

int 
ifdh::enterState( string state ){
  numsg::getMsg()->start(state.c_str());
  return 1;
}

int 
ifdh::leaveState( string state ){
  numsg::getMsg()->finish(state.c_str());
  return 1;
}

WebAPI * 
do_url_2(int postflag, va_list ap) {
    stringstream url;
    stringstream postdata;
    static string urls;
    static string postdatas;
    const char *sep = "";
    char * name;
    char * val;

    val = va_arg(ap,char *);
    if (val == 0 || *val == 0) {
        throw(WebAPIException("Environment variables IFDH_BASE_URI and EXPERIMENT not set and no URL set!",""));
    }
    while (strlen(val)) {
        url << sep << val;
        val = va_arg(ap,char *);
        sep = "/";
    }

    sep = postflag? "" : "?";

    name = va_arg(ap,char *);
    val = va_arg(ap,char *);
    while (strlen(name)) {
        (postflag ? postdata : url)  << sep << name << "=" << val;
        sep = "&";
        name = va_arg(ap,char *);
        val = va_arg(ap,char *);
    }
    urls = url.str();
    postdatas= postdata.str();

    if (ifdh::_debug) std::cerr << "calling WebAPI with url: " << urls << " and postdata: " << postdatas << "\n";

    return new WebAPI(urls, postflag, postdatas);
}

int
do_url_int(int postflag, ...) {
    va_list ap;
    int res;
    va_start(ap, postflag);
    WebAPI *wap = do_url_2(postflag, ap);
    res = wap->getStatus() - 200;
    if (ifdh::_debug) std::cerr << "got back int result: " << res << "\n";
    delete wap;
    return res;
}


string
do_url_str(int postflag,...) {
    va_list ap;
    string res("");
    string line;
    va_start(ap, postflag);
    WebAPI *wap = do_url_2(postflag, ap);
    while (!wap->data().eof()) {
      getline(wap->data(), line);
      res = res + line;
    }
    if (ifdh::_debug) std::cerr << "got back string result: " << res << "\n";
    delete wap;
    return res;
}

vector<string>
do_url_lst(int postflag,...) {
    va_list ap;
    string line;
    vector<string> res;
    va_start(ap, postflag);
    WebAPI *wap = do_url_2(postflag, ap);
    while (!wap->data().eof()) {
        getline(wap->data(), line);
        res.push_back(line);
    }
    delete wap;
    return res;
}

//datasets
int 
ifdh::createDefinition( string name, string dims, string user, string group) {
  return do_url_int(1,_baseuri.c_str(),"createDefinition","","name",name.c_str(), "dims", WebAPI::encode(dims).c_str(), "user", user.c_str(),"group", group.c_str(), "","");
}

int 
ifdh::deleteDefinition( string name) {
  return  do_url_int(1,_baseuri.c_str(),"deleteDefinition","","name", name.c_str(),"","");
}

string 
ifdh::describeDefinition( string name) {
  return do_url_str(0,_baseuri.c_str(),"describeDefinition", "", "name", name.c_str(), "","");
}

vector<string> 
ifdh::translateConstraints( string dims) {
  return do_url_lst(0,_baseuri.c_str(),"files", "list", "", "dims", WebAPI::encode(dims).c_str(), "","" );
}

// files
vector<string> 
ifdh::locateFile( string name) {
  return do_url_lst(0,_baseuri.c_str(), "locateFile", "", "file", name.c_str(), "", "" );  
}

string ifdh::getMetadata( string name) {
  return  do_url_str(0, _baseuri.c_str(),"getMetadata", "", "name", name.c_str(), "","");
}

//
string 
ifdh::dumpStation( string name, string what ) {
  return do_url_str(0,_baseuri.c_str(),"dumpStation", "", "station", name.c_str(), "dump", what.c_str(), "","");
}

// projects
string ifdh::startProject( string name, string station,  string defname_or_id,  string user,  string group) {
  return do_url_str(1,_baseuri.c_str(),"startProject","","name",name.c_str(),"station",station.c_str(),"defname",defname_or_id.c_str(),"username",user.c_str(),"group",group.c_str(),"","");
}

string 
ifdh::findProject( string name, string station){
   return do_url_str(0,_baseuri.c_str(),"findProject","","name",name.c_str(),"station",station.c_str(),"","");
}

string 
ifdh::establishProcess( string projecturi, string appname, string appversion, string location, string user, string appfamily , string description , int filelimit ) {
  char buf[64];
  snprintf(buf, 64, "%d", filelimit); 

  if (filelimit == -1) {
     filelimit = 0;
  }

  return do_url_str(1,projecturi.c_str(),"establishProcess", "", "appname", appname.c_str(), "appversion", appversion.c_str(), "deliverylocation", location.c_str(), "username", user.c_str(), "appfamily", appfamily.c_str(), "description", description.c_str(), "filelimit", buf, "", "");
}

string ifdh::getNextFile(string projecturi, string processid){
  return do_url_str(1,projecturi.c_str(),"processes",processid.c_str(),"getNextFile","","","");
}

string ifdh::updateFileStatus(string projecturi, string processid, string filename, string status){
  return do_url_str(1,projecturi.c_str(),"processes",processid.c_str(),"updateFileStatus","","filename", filename.c_str(),"status", status.c_str(), "","");
}

int 
ifdh::endProcess(string projecturi, string processid) {
  cleanup();
  return do_url_int(1,projecturi.c_str(),"processes",processid.c_str(),"endProcess","","","");
}

string 
ifdh::dumpProcess(string projecturi, string processid) {
  return do_url_str(1,projecturi.c_str(),"processes",processid.c_str(),"dumpProcess","","","");
}

int ifdh::setStatus(string projecturi, string processid, string status){
  return do_url_int(1,projecturi.c_str(),"processes",processid.c_str(),"setStatus","","status",status.c_str(),"","");
}

int ifdh::endProject(string projecturi) {
  return do_url_int(1,projecturi.c_str(),"endProject","","","");
}

ifdh::ifdh(std::string baseuri) { 
    check_env();
    if (0 != getenv("IFDH_DEBUG")) { 
	_debug = 1;
    }
    if ( baseuri == "" ) {
        _baseuri = getenv("IFDH_BASE_URI")?getenv("IFDH_BASE_URI"):(getenv("EXPERIMENT")?_default_base_uri+getenv("EXPERIMENT")+"/api":"") ;
     } else {
       _baseuri = baseuri;
    }
    _debug && std::cerr << "ifdh constructor: _baseuri is '" << _baseuri << "'\n";
}

void
ifdh::set_base_uri(std::string baseuri) { 
    _baseuri = baseuri; 
}

// give output files reported with addOutputFile a unique name
int 
ifdh::renameOutput(std::string how) {
    std::string outfiles_name = datadir()+"/output_files";
    std::string new_outfiles_name = datadir()+"/output_files.new";
    std::string line;
    fstream outlog(outfiles_name.c_str(), ios_base::in);
    fstream newoutlog(new_outfiles_name.c_str(), ios_base::out);
    std::string file, infile, froms, tos, outfile;
    int res;
    size_t spos;
   

    if ( outlog.fail()) {
       return 0;
    }

    if (how[0] == 's') {

        spos = how.find('/',2);
        froms = how.substr( 2, spos - 2);
        tos = how.substr( spos+1, how.size() - spos - 2);

        _debug && std::cerr << "replacing " << froms << " with " << tos << "\n";

        while (!outlog.eof() && !outlog.fail()) {
            getline(outlog,line);
            spos = line.find(' ');
         
            file = line.substr(0,spos);
            infile = line.substr(spos+1, line.size() - spos - 1);

            if ( ! file.size() ) {
               break;
            }
  
            spos = infile.rfind(froms);
            if (string::npos != spos) { 
                outfile = infile;
                outfile = outfile.replace(spos, froms.size(), tos);
                _debug && std::cerr << "renaming: " << file << " " << outfile << "\n";
                res = rename(file.c_str(), outfile.c_str());
                if ( 0 != res ) {
                    throw(WebAPIException("rename failed:", file.c_str() ));
                }
            } else {
                outfile = file;
            }
            newoutlog << outfile << " " << infile << "\n";

        }
    } else if (how[0] == 'u') {
        int count = 0;
        char hbuf[512];
        gethostname(hbuf, 512);
        time_t t = time(0);
        int pid = getpid();


        while (!outlog.eof() && !outlog.fail()) {
            count++;

            getline(outlog,line);
            spos = line.find(' ');
         
            file = line.substr(0,spos);
            infile = line.substr(spos+1, line.size() - spos - 1);

            if ( ! file.size() ) {
               break;
            }
  
            spos = file.find('.');
            if (spos == string::npos) {
               spos = 0;
            }

            stringstream uniqstr;
            uniqstr << '_' << hbuf << '_' << t << '_' << pid << '_' << count;

            outfile = file;
            outfile = outfile.insert( spos, uniqstr.str() );
	    rename(file.c_str(), outfile.c_str());

	    _debug && std::cerr << "renaming: " << file << " " << outfile << "\n";
            newoutlog << outfile << " " << infile << "\n";
        }
    }
    outlog.close();
    newoutlog.close();
    return rename(new_outfiles_name.c_str(), outfiles_name.c_str());
}

}
