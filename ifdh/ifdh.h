
#include <string>
#include <list>
#include "../client/WebAPI.h"

use std;

class ifdh {
   public:
       // file transfer
       int cp( string src, string dst );
       int rm( string loc );
       
       // logging
       int ifdh.log( string message );
       
       // metadata
       int get_meta( string filename );
       int set_meta( string filename, string meta_filename);
       list<string> find_files( string query_string );
       list<string> get_autodestination( string filename );
      
       // filesets/projects
       int define_fileset( string setname, string query_string);
       string join_project( string projectname );
       string next_file( string projectname, string consumer_proc_id );
       string done_file( string projectname, string consumer_proc_id, string filename, int status_ok);
       string done_project( string projectname, string consumer_proc_id, int status_ok);
       int recovery_fileset( string projectname, string new_filesetname );
       int jobsub_fileset( string in, string out, time_t time, string filesetname, string executable, ... );
};
