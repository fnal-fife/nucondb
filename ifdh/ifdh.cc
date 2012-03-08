#include "ifdh.h"
#include <sstream>
#include <iostream>

use std;

string cpn_loc  = "/grid/app/minos/scripts/cpn";
string fermi_gsiftp  = "gsiftp://fg-bestman1.fnal.gov:2811";
string bestmanuri = "srm://fg-bestman1.fnal.gov:10443"

string datadir() {
    strstream dirmaker;
    
    dirmaker << getenv("OSG_DATA")?getenv("OSG_DATA"):"/tmp"
             << "/pid_" << getppid();
    if ( 0 == access(dirmaker.str().c_str(), W_OK) ) {
        mkdir(dirmaker.str().c_str());
    }
    return dirmaker.str().c_str();
}

int ifdh::cleanup() {
    system("rm -rf " + datadir());
}


// file input
string ifdh::fetchInput( string src_uri ) {
    strstream cmd;
    int baseloc = src_uri.rfind("/") + 1;

    if (src_uri.substr(0,7) == "file://") {
	cmd << cpn_loc 
            << " " << src_uri.substr(7) 
            << " " << datadir() << "/" << src_uri.substr(baseloc);
        system(cmd.str());
        p = cmd.str().rfind(" ");
        return cmd.str().substr(p+1);
    }
    if (src_uri.substr(0,6) == "srm://") {
        cmd << "srmcp" 
            << " " << src_uri 
            << " " << "file:///" << datadir() << "/" << src_uri.substr(baseloc);
        system(cmd.str());
        p = cmd.str().rfind(" ");
        return cmd.str().substr(p+8);
    }
    raise Exception("Unknown uri type");
}

// file output
int ifdh::addOutputFile(string filename) {
    fstream outlog(datadir()+"/output_files", ios_base::append|iosbase::out);
    outlog << filename << "\n";
    outlog.close();
}

int ifdh::copyBackOutput(string dest_dir) {
    strstream cmd;
    int baseloc = src_uri.rfind("/") + 1;
    fstream outlog(datadir()+"/output_files", ios_base::in);

    if (access(dest_dir, W_OK)) {
        // destination is visible, so use cpn
	cmd << cpn_loc;
        while (!outlog.feof()) {
            cmd << " " << getline(outlog);
        }
        cmd << " " << dest_dir;
    } else {
        // destination is not visible, use srmcp
        cmd << "srmcp"
        while (!outlog.feof()) {
            cmd << " file:///" << getline(outlog);
        }
        cmd << " " << bestmanuri << dest_dir;
    }
    system(cmd.str().c_str());
    cleanup();
}


// logging
int ifdh::log( string message ){
;
}

int ifdh::enter_state( string state ){
;
}

int ifdh::leave_state( string state ){
;
}

WebAPI
do_url_2(int postflag, va_list ap) {
    strstream url;
    strstream postdata;
    char *sep = "";
    string name;
    string val;
    va_list ap;

    val = va_arg(ap,string);
    while (val.length()) {
        url << sep << val;
        val = va_arg(ap,string);
        sep = "/";
    }

    sep = postflag? "?" : "";

    name = va_arg(ap,string);
    val = va_arg(ap,string);
    while (name.length()) {
        (postflag ? postdata : url)  << sep << name << "=" << val;
        sep = "&";
        name = va_arg(ap,string);
        val = va_arg(ap,string);
    }
    return WebAPI(url.str(), postflag, postdata.str());
}

int
do_url_int(int postflag, ...) {
    va_list ap;
    int res;
    va_start(ap, postflag);
    WebAPI wa = do_url_2(postflag, ap);
    return wa.getStatus() - 200;
}


string
do_url_str(postflag,...)
    va_list ap;
    string res("");
    va_start(ap, postflag);
    WebAPI wa = do_url_2(postflag, ap);
    while (!wa.data().feof()) {
      res = res + getline(wa.data());
    }
    return res;
}

list<string>
do_url_lis(postflag,...)
    va_list ap;
    list<string> res;
    va_start(ap, postflag);
    WebAPI wa = do_url_2(postflag, ap);
    while (!wa.data().feof()) {
        res.append(getline(wa.data()));
    }
    return res;
}

//datasets
int 
ifdh::createDefinition(string baseuri, string name, string dims, string user) {
  return do_url_int(1,baseuri,"createDefinition","","name",name, "dims", dims, "user", user,"","");
}

int 
ifdh::deleteDefinition(string baseuri, string name) {
  return  do_url_int(1,baseuri,"deleteDefinition","","name", name,"","");
}

string 
ifdh::describeDefinition(string baseuri, string name) {
  return do_url_str(0,baseuri,"describeDefinition", "", "name", name, "","");
}

list<string> 
ifdh::translateConstraints(string baseuri, string dims) {
  return do_url_lst(0,baseuri,"translateConstraints", "", "dims", dims, "format","plain", "","" );
}

// files
list<string> 
ifdh::locateFile(string baseuri, string name) {
  return do_url_lst(0,baseuri, "locateFile", "", "name", name, "", "" );  
}

string ifdh::getMetadata(string baseuri, string name) {
  return  do_url_str(0, baseuri,"getMetadata", "", "name", name, "","");
}

//
string 
ifdh::dumpStation(string baseuri, name, what = "all") {
  return do_url_str(0,baseuri,"dumpStation", "", "station", name, "dump", what, "","");
}

// projects
string ifdh::startProject(string baseuri, string name, string station,  string defname_or_id,  string user,  string group) {
  return do_url_str(1,baseuri,startProject,"","name",name,"station",station,"defn",defname_or_id,"user",user,"grou",troup,"",""0);
}

string 
ifdh::findProject(string baseuri, string name, string station){
   return do_url_str(0,baseuri,"findProject","","name",name,"station",station,"","");
}

string 
ifdh::establishProcess(string baseuri, string appname, string appversion, string location, string user, string appfamily = "", string description = "", int filelimit = -1) {
  return do_url_str(1,baseuri,"establishProcess", "", "appname", appname, "appversion", appversion, "location", location, "user", user, "appfamily", appfamily, "description", description, "", "");
}

string ifdh::getNextFile(string projecturi, string processid){
  return do_url_str(1,projecturi,"processes",processid,"getNextFile","","","");
}

string ifdh::updateFileStatus(string projecturi, string processid, string filename, string status){
  return do_url_str(1,projecturi,"processes",processid,"updateFileStatus","","file", filename,"status", status "","");
}

int ifdh::endProcess(projecturi, string processid){
  cleanup();
  return do_url_str(1,projecturi,"processes",processid,"endProcess","","","");
}

string ifdh::dumpProcess(projecturi, string processid){
}

int ifdh::setStatus(string projecturi, string processid, string status){
  return do_url_str(1,projecturi,"processes",processid,"setStatus","","status",status,"","");
}
}

int ifdh::endProject(projecturi) {
  return do_url_str(1,projecturi,"endProject","","","");
}
