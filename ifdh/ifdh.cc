#include "ifdh.h"
#include <string>
#include <sstream>
#include <iostream>
#include <../numsg/numsg.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <string.h>

using namespace std;

namespace ifdh_ns {

string cpn_loc  = "/grid/app/minos/scripts/cpn";
string fermi_gsiftp  = "gsiftp://fg-bestman1.fnal.gov:2811";
string bestmanuri = "srm://fg-bestman1.fnal.gov:10443";

string datadir() {
    stringstream dirmaker;
    
    dirmaker << (getenv("OSG_DATA")?getenv("OSG_DATA"):"/tmp")
             << "/pid_" << getppid();

    if ( 0 != access(dirmaker.str().c_str(), W_OK) ) {
        mkdir(dirmaker.str().c_str(),0700);
    }
    return dirmaker.str().c_str();
}

int
ifdh::_debug = 0;

int
ifdh::cleanup() {
    string cmd("rm -rf ");
    cmd = cmd + datadir();
    return system(cmd.c_str());
}


// file io

int 
ifdh::cp(string src_path, string dest_path) {
    stringstream cmd;
    int dest_dir_loc;

    //
    // pick out the directory component in the destination
    //   
    dest_dir_loc = dest_path.rfind("/");
    //
    // if we can access source file for read and destination director for write
    // 
    if ( 0 == access(src_path.c_str(), R_OK) && 0 == access(dest_path.substr(0,dest_dir_loc).c_str(), W_OK) ) {
        cmd << "/bin/sh " << cpn_loc << " " << src_path << " " << dest_path;
        // otherwise, use srmpcp
    } else {
        cmd << "srmcp";

        if ( 0 == access(src_path.c_str(),R_OK) ) {
	   cmd << " file:///" << src_path;  
        } else {
	   cmd << " " << bestmanuri << src_path;  
        }

        if (0 == access(dest_path.substr(0,dest_dir_loc).c_str(), W_OK) ) {
	   cmd << " file:///" << dest_path;  
        } else {
	   cmd << " " << bestmanuri  << dest_path;  
        }
    }
    return system(cmd.str().c_str());
}

string 
ifdh::fetchInput( string src_uri ) {
    stringstream cmd;
    int p1, p2;
    int baseloc = src_uri.rfind("/") + 1;

    if (src_uri.substr(0,7) == "file://") {
	cmd << "/bin/sh " << cpn_loc 
            << " " << src_uri.substr(7) 
            << " " << datadir() << "/" << src_uri.substr(baseloc)
            << " >&2" ;
        _debug && cout << "running: " << cmd.str() << "\n";
        system(cmd.str().c_str());
        p1 = cmd.str().rfind(" ");
        p2 = cmd.str().rfind(" ", p1-1);
        return cmd.str().substr(p2+1, p1 - p2 -1);
    }
    if (src_uri.substr(0,6) == "srm://") {
        cmd << "srmcp" 
            << " " << src_uri 
            << " " << "file:///" << datadir() << "/" << src_uri.substr(baseloc)
            << " >&2" ;
        _debug && cout << "running: " << cmd.str() << "\n";
        system(cmd.str().c_str());
        p1 = cmd.str().rfind(" ");
        p2 = cmd.str().rfind(" ", p1-1);
        return cmd.str().substr(p2 + 8 , p1 - p2 - 8);
    }
    throw(WebAPIException("Unknown uri type",""));
}

// file output
int 
ifdh::addOutputFile(string filename) {
    fstream outlog((datadir()+"/output_files").c_str(), ios_base::app|ios_base::out);
    outlog << filename << "\n";
    outlog.close();
}

int 
ifdh::copyBackOutput(string dest_dir) {
    stringstream cmd;
    string line;
    int baseloc = dest_dir.find("/") + 1;
    fstream outlog((datadir()+"/output_files").c_str(), ios_base::in);

    if (access(dest_dir.c_str(), W_OK)) {
        // destination is visible, so use cpn
	cmd << "/bin/sh " << cpn_loc;
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
int 
ifdh::log( string message ) {
  numsg::getMsg()->printf("ifdh: %s", message.c_str());
}

int 
ifdh::enterState( string state ){
  numsg::getMsg()->start(state.c_str());
}

int 
ifdh::leaveState( string state ){
  numsg::getMsg()->finish(state.c_str());
}

WebAPI * 
do_url_2(int postflag, va_list ap) {
    stringstream url;
    stringstream postdata;
    string urls;
    string postdatas;
    const char *sep = "";
    char * name;
    char * val;

    val = va_arg(ap,char *);
    if (*val == 0) {
        throw(WebAPIException("Environment variable IFDH_BASE_URI not set!",""));
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

    if (ifdh::_debug) cout << "calling WebAPI with url: " << urls << " and postdata: " << postdatas;
    if (ifdh::_debug) cout.flush();

    return new WebAPI(urls, postflag, postdatas);
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
  return do_url_lst(0,_baseuri.c_str(),"translateConstraints", "", "dims", WebAPI::encode(dims).c_str(), "format","plain", "","" );
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
  return do_url_str(1,projecturi.c_str(),"establishProcess", "", "appname", appname.c_str(), "appversion", appversion.c_str(), "deliverylocation", location.c_str(), "username", user.c_str(), "appfamily", appfamily.c_str(), "description", description.c_str(), "", "");
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

}
