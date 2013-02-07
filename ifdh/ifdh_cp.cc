
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

        if (!getenv("CPN_DIR") || 0 != access(getenv("CPN_DIR"),R_OK)) {
            return;
        }

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
        if (!getenv("CPN_DIR") || 0 != access(getenv("CPN_DIR"),R_OK)) {
            return;
        }
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

  bool first = true;

  fstream listf(fname.c_str(), fstream::in);
  getline(listf, line);
  while( !listf.eof() && !listf.fail()) {
      if( !first ) {
  	 res.push_back( ";" );
      }
      first = false;
      pos = line.find(' ');
      if (pos != string::npos) {
         res.push_back( line.substr(0,pos) );
         res.push_back( line.substr(pos+1) );
      }
      getline(listf, line);
  }
 
  return res;
}

std::string parent_dir(std::string path) {
   size_t pos = path.rfind('/');
   if (pos == path.length() - 1) {
       pos = path.rfind('/', pos - 1);
   }
   ifdh::_debug && cout << "parent of " << path << " is " << path.substr(0, pos ) << "\n";
   return path.substr(0, pos);
}

//
// figure the destination filename for a source file
// and a destination directory.
//

std::string dest_file( std::string src_file, std::string dest_dir) {
    size_t pos;
    pos = src_file.rfind('/');
    if (pos == std::string::npos) {
        pos = 0;
    } else {
        pos = pos + 1;
    }
    return dest_dir + "/" + src_file.substr(pos);
}

void
test_dest_file() {
    std::string src("/tmp/f1");
    std::string ddir("/tmp/d2");
    std::cout << "src: " << src << " ddir: " << ddir << " yeilds: " << dest_file(src,ddir) << "\n";
}


std::vector<std::string> slice_directories(std::vector<std::string> args, int curarg) {
    std::vector<std::string> res;
    std::vector<std::vector<std::string>::size_type> dest_slots;
    
    //
    // find destination directory slots
    //    
    for( std::vector<std::string>::size_type i = curarg; i < args.size(); i++ ) {
        if ( i == args.size() - 1 || args[i+1] == ";" ) {
             dest_slots.push_back(i);
        }
    }

    ifdh::_debug && std::cout << "slice_diretctories: building results:" ;
    //
    // now use destination directory slots to make
    // pairwise copies
    //
    int cur_cp = 0;  // start with the zeroth copy
    for( std::vector<std::string>::size_type i = curarg; i < args.size(); i++ ) {
          if (args[i] == ";") {
              cur_cp++;  // if we see a ";" we move on to next copy 
              continue;
          }
          if( i != dest_slots[cur_cp] ) {  // don't do dest dest ";" 
              res.push_back(args[i]);   
              ifdh::_debug && std::cout << res.back() << " "; 
              res.push_back(dest_file(args[i], args[dest_slots[cur_cp]]));   
              ifdh::_debug && std::cout << res.back() << " ";
              if (i != args.size() - 2) {
                  res.push_back(";");
                  ifdh::_debug && std::cout << res.back() << " ";
              }
          }
     }
     ifdh::_debug && std::cout << "\n";

     return res;
}

//
// globus_url_copy will not do recursive directory copies
// without a trailing slash on the url... sigh
//
std::string fix_recursive_arg(std::string arg, bool recursive) {
   if (recursive && arg[arg.length()-1] != '/') {
       arg = arg + "/";
   }
   return arg;
}

int 
ifdh::cp( std::vector<std::string> args ) {

    std::string gftpHost("if-gridftp-");
    std::vector<std::string>::size_type curarg = 0;
    string force = " ";
    cpn_lock cpn;
    int res;
    bool recursive = false;
    bool dest_is_dir = false;

    if (getenv("IFDH_FORCE")) {
        force = getenv("IFDH_FORCE");
    }

    //
    // parse arguments
    //
    while (args[curarg][0] == '-') {

        //
        // first any --long args
        //
        if (args[curarg].substr(0,8) == "--force=") {
           force = args[0].substr(8,1);
           _debug && cout << "force option is " << args[curarg] << " char is " << force << "\n";
           curarg++;
           continue;
        }

        //
        // now any longer args
        //
	if (args[curarg].find('D') != std::string::npos) {
           dest_is_dir = true;
        }

	if (args[curarg].find('r')  != std::string::npos) {
           recursive = true;
        }
 
        // 
        // handle -f last, 'cause it rewrites arg list
        //
	if (args[curarg].find('f') != std::string::npos) {
	   curarg = 0;
	   args = expandfile(args[curarg + 1]);
           continue;
	}

        curarg++;
    }
   
    // convert relative paths to absolute
    
    string cwd(get_current_dir_name());

    for( std::vector<std::string>::size_type i = curarg; i < args.size(); i++ ) {
       if (args[i][0] != ';' && args[i][0] != '/') {
	   args[i] = cwd + "/" + args[i];
       }
    }

    // now decide local/remote
    // if anything is not local, use remote
    bool use_srm = false;
    bool use_exp_gridftp = false;
    bool use_bst_gridftp = false;
    bool use_cpn = true;

    if (force[0] == ' ') { // not forcing anything, so look

	for( std::vector<std::string>::size_type i = curarg; i < args.size(); i++ ) {
            if( args[i] == ";" ) {
                continue;
            }
	    if( 0 != local_access(args[i].c_str(),R_OK) ) {
	       
		if ( i == args.size() - 1 || args[i+1] == ";" ) {

		   if (0 != local_access(parent_dir(args[i]).c_str(),R_OK)) {
		       // if last one (destination)  and parent isn't 
		       // local either default to per-experiment gridftp 
		       // to get desired ownership. 
		       use_cpn = 0;
		       use_exp_gridftp = 1;
                       _debug && cout << "deciding to use exp gridftp due to: " << args[i] << "\n";
		   }      
		} else {
		   // for non-local sources, default to bestman gridftp
		   use_cpn = 0;
		   use_bst_gridftp = 1;
	           _debug && cout << "deciding to use bestman gridftp due to: " << args[i] << "\n";
		}
		break;
	     }
	 }
     } else if (force[0] == 's') {
         use_cpn = false;
         use_srm = true;
     } else if (force[0] == 'g') {
         use_cpn = false;
         use_bst_gridftp = true;
     } else if (force[0] == 'e') {
         use_cpn = false;
         use_exp_gridftp = true;
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
        gftpHost.append(".fnal.gov");
     }

     if (dest_is_dir && !use_cpn) {
         args = slice_directories(args, curarg);
         curarg = 0;
     }

     int keep_going = 1;

     cpn.lock();

     while( keep_going ) {
         stringstream cmd;

         cmd << (use_cpn ? "cp "  : use_srm ? "srmcp " : use_exp_gridftp||use_bst_gridftp ? "globus-url-copy " : "false" );
         
         if (recursive) {
            cmd << "-r ";
         }
         if (recursive && !use_cpn) {
             cmd << "-cd ";
         }

         while (curarg < args.size() && args[curarg] != ";" ) {

            args[curarg] = fix_recursive_arg(args[curarg],recursive);

            if (use_cpn) { 
                // no need to munge arguments, take them as is.
                cmd << args[curarg] << " ";
            } else if (0 == local_access(args[curarg].c_str(), R_OK)) {
                cmd << "file:///" << args[curarg] << " ";
            } else if (( curarg == args.size() - 1 || args[curarg+1] == ";" ) && (0 == local_access(parent_dir(args[curarg]).c_str(), R_OK))) {
                cmd << "file:///" << args[curarg] << " ";
            } else if (use_srm) {
		cmd << bestman_srm_uri << args[curarg] << " ";
            } else if (use_exp_gridftp) {
                cmd << "gsiftp://" << gftpHost << args[curarg] << " ";
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


int
ifdh::mv(vector<string> args) {
    int res;
    vector<string>::iterator p;

    res = cp(args);
    if ( res == 0 ) {
        args.pop_back();
        for (p = args.begin(); p != args.end() ; p++ ) {
            string &s = *p;
            if (s == ";" || *(p+1) == ";" || s[0] == '-') {
                // don't remove destinations, options, or files named ";"
                continue;
            }
            if (0 == access(s.c_str(), W_OK)) {
                _debug && std::cout << "unlinking: " << s << "\n";
                unlink(s.c_str());
            } else {
                string srmcmd("srmrm ");
                srmcmd +=  bestman_srm_uri + s + " ";
                _debug && std::cout << "running: " << srmcmd << "\n";
                // system(srmcmd.c_str());
            }
        }
    }
    return res;
}

}
