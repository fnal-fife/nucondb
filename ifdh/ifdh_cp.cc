
#include "ifdh.h"
#include "utils.h"
#include <iostream>
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
#include <sys/resource.h>
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

struct stat *
cache_stat(std::string s) {
   int res;
   static struct stat sbuf;
   static string last_s;
   if (last_s == s) {
       return &sbuf;
   }
   res = stat(s.c_str(), &sbuf);
   if (res != 0) {
       return 0;
   }
   last_s = s;
   return  &sbuf;
}

bool
is_directory(std::string dirname) {
    struct stat *sb = cache_stat(dirname);
    if (!sb) {
        return false;
    }
    ifdh::_debug && std::cout << "is_directory(" << dirname << ") -> " <<  S_ISDIR(sb->st_mode);
    return S_ISDIR(sb->st_mode) != 0;
}

bool 
is_bestman_server(std::string uri) {
   std::stringstream cmd;
   static char buf[512];
   FILE *pf;

   // if it starts with a slash, we're rewriting it to 
   // the fermi bestman server...
   if (uri[0] == '/') 
        return true;

   ifdh::_debug && cout << "checking with srmping\n";
   cmd << "srmping " << uri;
   pf = popen(cmd.str().c_str(),"r");
   bool found = false;
   while (fgets(buf, 512, pf)) {
       ifdh::_debug && cout << "srmping says: " << buf;
       if (0 == strncmp("backend_type:BeStMan", buf, 20)) {
           found = true; 
           break;
       }
   }
   pclose(pf);
   return found;
}

#include <dirent.h>
int
is_empty(std::string dirname) {
    DIR *pd = opendir(dirname.c_str());
    struct dirent *pde;
    int count = 0;

    while( 0 != (pde = readdir(pd)) ) {
        count++;
        if (count > 2) {
            break;
        }
    }
    closedir(pd);
    ifdh::_debug && cout << "is_empty " << dirname << "is " << (count > 2);
    return count == 2;
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
	while (!feof(pf) && !ferror(pf)) {
            if (fgets(buf, 512, pf)) {
                fputs(buf,stdout);
                fflush(stdout);
            }
	}
        if (ferror(pf)) {
	    pclose(pf);
	    throw( std::logic_error("Could not get CPN lock: error reading lock output"));
        }
	pclose(pf);

	// pick lockfile name out of last line
	std::string lockfilename(buf);
	size_t pos = lockfilename.rfind(' ');
	lockfilename = lockfilename.substr(pos+1);

        // trim newline
	lockfilename = lockfilename.substr(0,lockfilename.length()-1);

	// kick off a backround thread to update the
	// lock file every minute that we're still running

	if ( lockfilename[0] != '/' || 0 != access(lockfilename.c_str(),W_OK)) {
	    throw( std::logic_error("Could not get CPN lock."));
	}
     
	_heartbeat_pid = fork();
	if (_heartbeat_pid == 0) {
	    parent_pid = getppid();
	    while( 0 == kill(parent_pid, 0) ) {
                // touch our lockfile
		if ( 0 != utimes(lockfilename.c_str(), NULL)) {
                    perror("Lockfile touch failed");
                    exit(1);
                }
		sleep(60);
	    }
	    exit(0);
	}
    }

    void
    free() {
        int res;
        if (!getenv("CPN_DIR") || 0 != access(getenv("CPN_DIR"),R_OK)) {
            return;
        }
        kill(_heartbeat_pid, 15);
        waitpid(_heartbeat_pid, &res, 0);
        system("$CPN_DIR/bin/lock free");
        _heartbeat_pid = -1;
        if (!WIFSIGNALED(res) || 15 != WTERMSIG(res)) {
            stringstream basemessage;
            basemessage <<"lock touch process exited code " << res << " signalled: " << WIFSIGNALED(res) << " signal: " << WTERMSIG(res);
            throw( std::logic_error(basemessage.str()));
        }
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
  if ( !listf.eof() ) {
      std::string basemessage("error reading list of files file: ");
      throw( std::logic_error(basemessage += fname));
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

std::vector<std::string> 
ifdh::build_stage_list( bool dirflag, std::vector<std::string> args, int curarg) {
   std::vector<std::string>  res;
   std::string stagefile("stage_");
   std::string ustring;
   std::string stage_location;

   // unique string for this stage out
   ustring = unique_string();
   std::string mkdirstring("srmmkdir -2 ");

   // if we are told how to stage, use it, fall back to OSG_SITE_WRITE,
   //  or worst case, the bestman gateway.
   std::string base_uri(getenv("IFDH_STAGE_VIA")? getenv("IFDH_STAGE_VIA"):
			getenv("OSG_SITE_WRITE")? getenv("OSG_SITE_WRITE"): bestman_srm_uri + "/grid/data/");
   base_uri = (base_uri + "/" + getexperiment());
   stagefile += ustring;

   // make sure directory hierarchy is there..
   system( (mkdirstring +  base_uri + "/ifdh_stage").c_str() );
   system( (mkdirstring +  base_uri + "/ifdh_stage/queue").c_str() );
   system( (mkdirstring +  base_uri + "/ifdh_stage/data").c_str() );
   system( (mkdirstring +  base_uri + "/ifdh_stage/data/" + ustring).c_str() );

   // open our stageout queue file/copy back instructions
   fstream stageout(stagefile.c_str(), fstream::out);

   // get args in single src dest ";" pair format...
   if (dirflag) { 
       args = slice_directories(args, curarg);
       curarg = 0;
   }
   
   for( std::vector<std::string>::size_type i = curarg; i < args.size(); i+=3 ) {

       // we're going to keep this in our stage queue area
       stage_location = base_uri + "/ifdh_stage/data/" + ustring + "/" +  basename(args[i].c_str());

       // we copy to the stage location
       res.push_back(args[i]);
       res.push_back(stage_location);
       res.push_back(";");

       // the stage item out is from there to the final destination
       stageout << stage_location << ' ' << args[i+1]  << '\n';
   }

   // copy our queue file in last, it means the others are ready to copy
   res.push_back( stagefile );
   res.push_back( base_uri + "/ifdh_stage/queue/" + stagefile );

   return res;
}

int 
ifdh::cp( std::vector<std::string> args ) {

    std::string gftpHost("if-gridftp-");
    std::vector<std::string>::size_type curarg = 0;
    string force = " ";
    cpn_lock cpn;
    int res;
    int rres = 0;
    bool recursive = false;
    bool dest_is_dir = false;
    struct rusage rusage_before, rusage_after;
    time_t time_before, time_after;

    if (_debug) {
         std::cout << "entering ifdh::cp( ";
         for( std::vector<std::string>::size_type i = 0; i < args.size(); i++ ) {
             std::cout << args[i] << " ";
         }
         std::cout << "rusage blocks before: " << rusage_before.ru_inblock << " " << rusage_before.ru_oublock << "\n"; 
    }


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
           force = args[curarg].substr(8,1);
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

    if (cwd[0] != '/') {
        throw( std::logic_error("unable to determine current working directory"));
    }

    for( std::vector<std::string>::size_type i = curarg; i < args.size(); i++ ) {
       if (args[i][0] != ';' && args[i][0] != '/' && args[i].find("srm:") != 0 && args[i].find("gsiftp:") != 0) {
	   args[i] = cwd + "/" + args[i];
       }
    }

    if (getenv("IFDH_STAGE_VIA") || getenv("IFDH_STAGE")) {
       args = build_stage_list(dest_is_dir, args, curarg);
       curarg = 0;
       dest_is_dir = false;
    }

    // now decide local/remote
    // if anything is not local, use remote
    bool use_srm = false;
    bool use_exp_gridftp = false;
    bool use_bst_gridftp = false;
    bool use_cpn = true;
    bool use_dd = false;
    bool use_any_gridftp = false;

    if (force[0] == ' ') { // not forcing anything, so look

	for( std::vector<std::string>::size_type i = curarg; i < args.size(); i++ ) {

            if( args[i] == ";" ) {
                continue;
            }

            if( args[i].find("srm:") == 0)  { 
               use_cpn = false; 
               use_srm = true; 
               break; 
            }

            if( args[i].find("gsiftp:") == 0) { 
               use_cpn = false; 
               use_bst_gridftp = true; 
               break; 
            }

	    if( 0 != access(args[i].c_str(),R_OK) ) {
	       
		if ( i == args.size() - 1 || args[i+1] == ";" ) {

		   if (0 != access(parent_dir(args[i]).c_str(),R_OK)) {
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
     } else if (force[0] == 'd') {
         use_cpn = false;
         use_dd = true;
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
          std::string basemessage("invalid --force= option: ");
          throw( std::logic_error(basemessage += force));
     }

     use_any_gridftp = use_any_gridftp || use_exp_gridftp || use_bst_gridftp;

     if (recursive && (use_srm || use_dd )) { 
        throw( std::logic_error("invalid use of -r with --force=srm or --force=dd"));
     }

     //
     // default to dd for non-recursive copies.
     // 
     if (use_cpn && !recursive) {
         use_cpn = 0;
         use_dd = 1;
     }

     if (use_exp_gridftp) {
        gftpHost.append(getexperiment());
        gftpHost.append(".fnal.gov");
     }

     //
     // srmcp and dd only do specific srcfile,destfile copies
     //
     if (dest_is_dir && (use_srm || use_dd || use_any_gridftp)) {
         args = slice_directories(args, curarg);
         dest_is_dir = false;
         curarg = 0;
     }

     int error_expected;
     int keep_going = 1;

     cpn.lock();

     getrusage(RUSAGE_CHILDREN, &rusage_before);
     if (_debug) {
         std::cout << "rusage blocks before: " << rusage_before.ru_inblock << " " << rusage_before.ru_oublock << "\n"; 
     }
     time(&time_before);

     while( keep_going ) {
         stringstream cmd;

         cmd << (use_dd ? "dd bs=512k " : use_cpn ? "cp "  : use_srm ? "srmcp -2 "  : use_any_gridftp ? "globus-url-copy " : "false" );

         if (recursive) {
            cmd << "-r ";
         }

         if ((recursive || dest_is_dir) && use_any_gridftp) {
             cmd << "-cd ";
         }

         error_expected  = 0;

         while (curarg < args.size() && args[curarg] != ";" ) {

            args[curarg] = fix_recursive_arg(args[curarg],recursive);

            if (is_directory(args[curarg])) {

		if ( use_bst_gridftp || use_exp_gridftp ) {
		    if ( is_empty(args[curarg])) {
			ifdh::_debug && std::cout << "expecting error due to empty dir " << args[curarg] << "\n";
				error_expected = 1;
		    }
		    if ( args[curarg][args[curarg].length()-1] != '/' ) {
			args[curarg] += "/";
		    }
		}

                // emulate globus-url-copy directory behavior of doing 1-level
                // deep copy for cpn by doing dir/*
                if (use_cpn && !recursive && curarg != args.size() - 1 && args[curarg+1] != ";" && !is_empty(args[curarg]) ) {
                    args[curarg] += "/*";   
                }

            }

	    if (use_any_gridftp && dest_is_dir && (curarg == args.size() - 1 || args[curarg+1] == ";" ) &&  args[curarg][args[curarg].length()-1] != '/' ) {
		 
		    args[curarg] += "/";
	    }

            if (use_cpn) { 
                // no need to munge arguments, take them as is.
                cmd << args[curarg] << " ";
            } else if (use_dd) {
                if (curarg == args.size() - 1 || args[curarg+1] == ";" ){
                   cmd << "of=" << args[curarg] << " ";
                } else {
                   cmd << "if=" << args[curarg] << " ";
                }
            } else if (0 == local_access(args[curarg].c_str(), R_OK)) {
                cmd << "file:///" << args[curarg] << " ";
            } else if (( curarg == args.size() - 1 || args[curarg+1] == ";" ) && (0 == local_access(parent_dir(args[curarg]).c_str(), R_OK))) {
                cmd << "file:///" << args[curarg] << " ";
            } else if (use_srm) {
                if( args[curarg].find("srm:") == 0)  
		    cmd << args[curarg] << " ";
                else
		    cmd << bestman_srm_uri << args[curarg] << " ";

                //
                // if it is a third party copy *to* a BestMan server
                // use -pushmode=true See:
                // http://ticket.grid.iu.edu/goc/5232?expandall=true&sort=up
                //
                if (( curarg == args.size() - 1 || args[curarg+1] == ";" ) && is_bestman_server( args[curarg]) && std::string::npos == cmd.str().find("file:///") ) {
                    
                    cmd.str( cmd.str().replace(5,1," -pushmode=true "));
                }

            } else if (use_exp_gridftp) {
                if( args[curarg].find("gsiftp:") == 0)  
                    cmd << args[curarg] << " ";
                else
                    cmd << "gsiftp://" << gftpHost << args[curarg] << " ";
            } else if (use_bst_gridftp) {
                if( args[curarg].find("gsiftp:") == 0)  
                    cmd << args[curarg] << " ";
                else
                    cmd << bestman_ftp_uri << args[curarg] << " ";
            }
            curarg++;
        }

        _debug && std::cerr << "running: " << cmd.str() << "\n";

        res = system(cmd.str().c_str());
       
        if ( res != 0 && error_expected ) {
            _debug && std::cerr << "expected error...\n";
            res = 0;
        }


        if (res != 0 && rres == 0) {
            rres = res;
        }
 
        if (curarg < args.size() && args[curarg] == ";" ) {
            curarg++;
            keep_going = 1;
        } else {
            keep_going = 0;
        }
    }

    time(&time_after);
    getrusage(RUSAGE_CHILDREN, &rusage_after);

    long int delta_in = rusage_after.ru_inblock - rusage_before.ru_inblock;
    long int delta_out = rusage_after.ru_oublock - rusage_before.ru_oublock;
    long int delta_t = time_after - time_before;

    if (_debug) {
         std::cout << "rusage blocks after: " << rusage_after.ru_inblock << " " << rusage_after.ru_oublock << "\n"; 
         std::cout << "total blocks: " <<  delta_in << " in " << delta_out << " out " << delta_t << " seconds\n";
    }
    stringstream logmessage;
    logmessage << "ifdh cp: blocks: " <<  delta_in << " in " << delta_out << " out " << delta_t << " seconds \n";
    this->log(logmessage.str());

    cpn.free();

    return rres;
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
                string srmcmd("srmrm -2 ");
                srmcmd +=  bestman_srm_uri + s + " ";
                _debug && std::cout << "running: " << srmcmd << "\n";
                res = system(srmcmd.c_str());
            }
        }
    }
    return res;
}

}
