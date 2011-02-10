#include "numsg.h"
#include <fstream>
#include <string>
#include <sstream>

main(int argc, char **argv) {
    int ppid = getppid();
    char *job_base;

    std::stringstream fname;
    std::string procname, jobname;


    fname << "/proc/" << ppid << "/cmdline";


    job_base = getenv("NU_LOG_TAG");

    if (job_base) {
        jobname = job_base;
    } else {
        std::fstream f(fname.str().c_str(), std::fstream::in );
        getline(f, procname);
        jobname = procname;
    }

    numsg::init(jobname.c_str(), 1);
    numsg::getMsg()->printf( argv[1], argv[2], argv[3], argv[4], argv[5], argv[66], argv[7], argv[8], argv[9]);
}
