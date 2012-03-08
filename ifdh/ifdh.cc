#include "ifdh.h"
#include <string>
#include <sstream>
#include <iostream>
#include <numsg.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <string.h>

using namespace std;

string cpn_loc  = "/grid/app/minos/scripts/cpn";
string fermi_gsiftp  = "gsiftp://fg-bestman1.fnal.gov:2811";
string bestmanuri = "srm://fg-bestman1.fnal.gov:10443";

string datadir() {
    stringstream dirmaker;
    
    dirmaker << (getenv("OSG_DATA")?getenv("OSG_DATA"):"/tmp")
             << "/pid_" << getppid();

    if ( 0 == access(dirmaker.str().c_str(), W_OK) ) {
        mkdir(dirmaker.str().c_str(),0700);
    }
    return dirmaker.str().c_str();
}

void
ifdh::cleanup() {
    string cmd("rm -rf ");
    cmd = cmd + datadir();
    system(cmd.c_str());
}


// file input
string ifdh::fetchInput( string src_uri ) {
    stringstream cmd;
    int p;
    int baseloc = src_uri.rfind("/") + 1;

    if (src_uri.substr(0,7) == "file://") {
	cmd << cpn_loc 
            << " " << src_uri.substr(7) 
            << " " << datadir() << "/" << src_uri.substr(baseloc);
        system(cmd.str().c_str());
        p = cmd.str().rfind(" ");
        return cmd.str().substr(p+1);
    }
    if (src_uri.substr(0,6) == "srm://") {
        cmd << "srmcp" 
            << " " << src_uri 
            << " " << "file:///" << datadir() << "/" << src_uri.substr(baseloc);
        system(cmd.str().c_str());
        p = cmd.str().rfind(" ");
        return cmd.str().substr(p+8);
    }
    throw(WebAPIException("Unknown uri type",""));
}

// file output
int ifdh::addOutputFile(string filename) {
    fstream outlog((datadir()+"/output_files").c_str(), ios_base::app|ios_base::out);
    outlog << filename << "\n";
    outlog.close();
}

int ifdh::copyBackOutput(string dest_dir) {
    stringstream cmd;
    string line;
    int baseloc = dest_dir.find("/") + 1;
    fstream outlog((datadir()+"/output_files").c_str(), ios_base::in);

    if (access(dest_dir.c_str(), W_OK)) {
        // destination is visible, so use cpn
	cmd << cpn_loc;
        while (!outlog.eof()) {
            getline(outlog,line);
            cmd << " " << line;
        }
        cmd << " " << dest_dir;
    } else {
        // destination is not visible, use srmcp
        cmd << "srmcp";
        while (!outlog.eof()) {
            getline(outlog, line);
            cmd << " file:///" << line;
        }
        cmd << " " << bestmanuri << dest_dir;
    }
    system(cmd.str().c_str());
    cleanup();
}


// logging
int ifdh::log( string message ) {
  numsg::getMsg()->printf("ifdh: %s", message.c_str());
}

int ifdh::enter_state( string state ){
  numsg::getMsg()->start(state.c_str());
}

int ifdh::leave_state( string state ){
  numsg::getMsg()->finish(state.c_str());
}

WebAPI * 
do_url_2(int postflag, va_list ap) {
    stringstream url;
    stringstream postdata;
    const char *sep = "";
    char * name;
    char * val;

    val = va_arg(ap,char *);
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
    return new WebAPI(url.str(), postflag, postdata.str());
}

int
do_url_int(int postflag, ...) {
    va_list ap;
    int res;
    va_start(ap, postflag);
    WebAPI *wap = do_url_2(postflag, ap);
    res = wap->getStatus() - 200;
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
    delete wap;
    return res;
}

list<string>
do_url_lst(int postflag,...) {
    va_list ap;
    string line;
    list<string> res;
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
ifdh::createDefinition(string baseuri, string name, string dims, string user) {
  return do_url_int(1,baseuri.c_str(),"createDefinition","","name",name.c_str(), "dims", dims.c_str(), "user", user.c_str(),"","");
}

int 
ifdh::deleteDefinition(string baseuri, string name) {
  return  do_url_int(1,baseuri.c_str(),"deleteDefinition","","name", name.c_str(),"","");
}

string 
ifdh::describeDefinition(string baseuri, string name) {
  return do_url_str(0,baseuri.c_str(),"describeDefinition", "", "name", name.c_str(), "","");
}

list<string> 
ifdh::translateConstraints(string baseuri, string dims) {
  return do_url_lst(0,baseuri.c_str(),"translateConstraints", "", "dims", dims.c_str(), "format","plain", "","" );
}

// files
list<string> 
ifdh::locateFile(string baseuri, string name) {
  return do_url_lst(0,baseuri.c_str(), "locateFile", "", "file", name.c_str(), "", "" );  
}

string ifdh::getMetadata(string baseuri, string name) {
  return  do_url_str(0, baseuri.c_str(),"getMetadata", "", "name", name.c_str(), "","");
}

//
string 
ifdh::dumpStation(string baseuri, string name, string what ) {
  return do_url_str(0,baseuri.c_str(),"dumpStation", "", "station", name.c_str(), "dump", what.c_str(), "","");
}

// projects
string ifdh::startProject(string baseuri, string name, string station,  string defname_or_id,  string user,  string group) {
  return do_url_str(1,baseuri.c_str(),"startProject","","name",name.c_str(),"station",station.c_str(),"defn",defname_or_id.c_str(),"user",user.c_str(),"group",group.c_str(),"","");
}

string 
ifdh::findProject(string baseuri, string name, string station){
   return do_url_str(0,baseuri.c_str(),"findProject","","name",name.c_str(),"station",station.c_str(),"","");
}

string 
ifdh::establishProcess(string baseuri, string appname, string appversion, string location, string user, string appfamily , string description , int filelimit) {
  return do_url_str(1,baseuri.c_str(),"establishProcess", "", "appname", appname.c_str(), "appversion", appversion.c_str(), "location", location.c_str(), "user", user.c_str(), "appfamily", appfamily.c_str(), "description", description.c_str(), "", "");
}

string ifdh::getNextFile(string projecturi, string processid){
  return do_url_str(1,projecturi.c_str(),"processes",processid.c_str(),"getNextFile","","","");
}

string ifdh::updateFileStatus(string projecturi, string processid, string filename, string status){
  return do_url_str(1,projecturi.c_str(),"processes",processid.c_str(),"updateFileStatus","","file", filename.c_str(),"status", status.c_str(), "","");
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

main() {
  ifdh i;
  string minerva_base("http://samweb-minerva.fnal.gov:20004/sam/minerva/api");
  WebAPI::_debug = 1;
  cout << "found it at:" <<
  i.locateFile(minerva_base, "MV_00003142_0014_numil_v09_1105080215_RawDigits_v1_linjc.root").front();
  cout << "definition is:" <<
  i.describeDefinition(minerva_base, "mwm_test_1");
}
