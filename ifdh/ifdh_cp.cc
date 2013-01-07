
#include "ifdh.h"
#include "utils.h"
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <../numsg/numsg.h>
#include <stdarg.h>
#include <string.h>

#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vfs.h> 
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <linux/nfs_fs.h>

using namespace std;

namespace ifdh_ns {

std::string bestman_srm_uri = "srm://fg-bestman1.fnal.gov:10443/srm/v2/server?SFN=";
std::string bestman_ftp_uri = "gsiftp://fg-bestman1.fnal.gov:2811";


int
local_access(const char *path, int mode) {
    static struct statfs buf;
    int res;

    res = statfs(path, &buf);
    if (0 != res ) {
       return res;
    }
    if (buf.f_type == NFS_SUPER_MAGIC) {
       return -1;
    } else {
       return access(path, mode);
    }
}

extern std::string datadir();

// file io

class cpn_lock {

private:

    int _heartbeat_pid;

public:

    void
    lock() {
	char buf[512];
	FILE *pf;
        int parent_pid;

	// call lock, skip to last line 
	pf = popen("$CPN_DIR/bin/lock","r");
	while (fgets(buf, 512, pf)) {
	    ;
	}
	pclose(pf);

	// pick lockfile name out of last line
	std::string lockfilename(buf);
	size_t pos = lockfilename.rfind(' ');
	lockfilename = lockfilename.substr(pos+1);

	// kick off a backround thread to update the
	// lock file every minute that we're still running

	if ( lockfilename[0] != '/') {
	    throw( std::logic_error("Could not get CPN lock."));
	}
     
	_heartbeat_pid = fork();
	if (_heartbeat_pid == 0) {
	    parent_pid = getppid();
	    sleep(60);
	    while( 0 == kill(parent_pid, 0) ) {
                // touch our lockfile
		utimes(lockfilename.c_str(), NULL);
		sleep(60);
	    }
	    exit(0);
	}
    }

    void
    free() {
      kill(_heartbeat_pid, 15);
      waitpid(_heartbeat_pid, NULL, 0);
      system("$CPN_DIR/bin/lock free");
      _heartbeat_pid = -1;
    }

    cpn_lock() : _heartbeat_pid(-1) { ; }
 
    ~cpn_lock()  {

       if (_heartbeat_pid != -1) {
           free();
       }
    }
};

std::vector<std::string> expandfile( std::string fname ) {
  std::vector<std::string> res;
  std::string line;
  size_t pos;

  fstream listf(fname.c_str(), fstream::in);
  while( !listf.eof() && !listf.fail()) {
      getline(listf, line);
      pos = line.find(' ');
      if (pos != string::npos) {
         res.push_back( line.substr(0,pos) );
         res.push_back( line.substr(pos+1) );
         res.push_back( ";" );
      }
  }
 
  return res;
}

std::string parent_dir(std::string path) {
   size_t pos = path.rfind('/');
   return path.substr(0, pos - 1);
}

int 
ifdh::cp( std::vector<std::string> args ) {

    std::string gftpHost("if-gridftp-");
    std::vector<std::string>::size_type curarg = 0;
    int recursive = 0;
    string force = " ";
    cpn_lock cpn;
    int res;

    // handle --force=whatever and -f initially...

    if (args[curarg].substr(0,7) == "--force=") {
       force = args[0].substr(7,1);
       curarg++;
    }

    if (args[curarg] == "-f") { 
       curarg = 0;
       args = expandfile(args[1]);
    }
   
    if (args[curarg] == "-r") { 
       recursive = 1;
       curarg++;
    }
 
    // now decide local/remote
    // if anything is not local, use remote
    int use_srm = 0;
    int use_exp_gridftp = 0;
    int use_bst_gridftp = 0;
    int use_cpn = 1;

    if (force[0] == ' ') { // not forcing anything, so look

	for( std::vector<std::string>::size_type i = curarg; i < args.size(); i++ ) {
	    if( 0 != local_access(args[i].c_str(),R_OK) ) {
	       
		if ( i == args.size() || args[i+1] == ";" ) {

		   if (0 != local_access(parent_dir(args[i]).c_str(),R_OK)) {
		       // if last one (destination)  and parent isn't 
		       // local either default to per-experiment gridftp 
		       // to get desired ownership. 
		       use_cpn = 0;
		       use_exp_gridftp = 1;
		   }      
		} else {
		   // for non-local sources, default to bestman gridftp
		   use_cpn = 0;
		   use_bst_gridftp = 1;
		}
		break;
	     }
	 }
     } else if (force[0] == 's') {
         use_cpn = 0;
         use_srm = 1;
     } else if (force[0] == 'g') {
         use_cpn = 0;
         use_bst_gridftp = 1;
     } else if (force[0] == 'e') {
         use_cpn = 0;
         use_exp_gridftp = 1;
     } else if (force[0] == 'c') { 
          ;   // forcing CPN, already set
     } else {
          throw( std::logic_error("invalid --force= option"));
     }


     if (recursive && use_srm) { 
        throw( std::logic_error("invalid use of -r with --force=srm"));
     }

     if (use_exp_gridftp) {
        gftpHost.append(getexperiment());
     }

     int keep_going = 1;

     cpn.lock();

     while( keep_going ) {
         stringstream cmd;

         cmd << (use_cpn ? "cp "  : use_srm ? "srmcp " : use_exp_gridftp||use_bst_gridftp ? "globus_url_copy " : "false" );
         
         if (recursive) {
            cmd << "-r ";
         }

         while (curarg < args.size() && args[curarg] != ";" ) {

            if (use_cpn) { 
                // no need to munge arguments, take them as is.
                cmd << args[curarg] << " ";
            } else if (0 == local_access(args[curarg].c_str(), R_OK)) {
                cmd << "file://" << args[curarg];
            } else if (( curarg == args.size() || args[curarg+1] == ";" ) && (0 == local_access(parent_dir(args[curarg]).c_str(), R_OK))) {
                cmd << "file://" << args[curarg];
            } else if (use_srm) {
		cmd << bestman_srm_uri << args[curarg] << " ";
            } else if (use_exp_gridftp) {
                cmd << "gsiftp:" << gftpHost << args[curarg] << " ";
            } else if (use_bst_gridftp) {
                cmd << bestman_ftp_uri << args[curarg] << " ";
            }
            curarg++;
        }

        _debug && std::cerr << "running: " << cmd.str() << "\n";

        res = system(cmd.str().c_str());

        if (curarg < args.size() && args[curarg] == ";" && res == 0) {
            curarg++;
            keep_going = 1;
        } else {
            keep_going = 0;
        }
    }
    cpn.free();
    return res;
} 


}
