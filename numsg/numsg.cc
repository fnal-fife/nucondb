#include "numsg.h"
#include <sstream>
#include <stdlib.h>
#include <stdarg.h>

numsg *numsg::_singleton = 0;

numsg::numsg(const char *jobname, char *host, int port, int parentflag) : 
	_sa(host, port, parentflag), 
	_cur_state("boot"), 
	_old_states(), 
	_jobname(jobname?jobname:getenv("NU_LOG_TAG")),
        _facility(17),
        _severity(5)
{;}

numsg::~numsg() {
    new_state("exiting");
}

numsg *
numsg::init(const char *jobname, int parentflag) {
    char hostbuf[512];
    char *host;
    char *pcolon;
    int port;
    
    host = getenv("NU_LOG_HOST");
    if (host) { 
	strncpy(hostbuf, host, 512);
	if (0 != (pcolon = strchr(hostbuf,':'))) {
	    port = atoi(pcolon + 1);
	    *pcolon = 0; 
	} else {
	    port = 514;
	}
    } else {
        strcpy(hostbuf, "localhost");
        port = 514;
    }
    if (jobname == 0 && getenv("NU_MSG_TAG"))  {
        jobname = getenv("NU_MSG_TAG");
    }
    numsg::_singleton = new numsg(jobname, hostbuf, port, parentflag);
    return numsg::_singleton;
}

numsg *
numsg::getMsg() {
    return numsg::_singleton;
}

void
numsg::new_state(const char *state) {
    std::stringstream st;
    _cur_state = state;
    st <<  "State: " << _cur_state;
    _sa.send( _facility, _severity, _jobname.c_str(), st.str().c_str());
}

void
numsg::start(const char *what) {
    _starttime = time(0);
    _old_states.push_front(_cur_state);
    new_state(what);
}

void
numsg::printf(const char *fmt, ...) {
    static char buf[1024];
    va_list va;
    va_start(va, fmt);
    vsprintf(buf, fmt, va);
    _sa.send( _facility, _severity, _jobname.c_str(), buf);
}

void
numsg::finish(const char *what) {
    std::stringstream st;
    std::string s;

    printf("Finished: %s -- elapsed %d sec.", what, (time(0) - _starttime));
    new_state(_old_states.front().c_str());
    _old_states.pop_front();
}


#ifdef UNITTEST
main() {
    numsg *nm;

    putenv("NU_LOG_HOST=localhost:514");
    //nm = numsg::init("testjob");
    nm = numsg::init(0);
    numsg::getMsg()->new_state("foo");
    numsg::getMsg()->new_state("bar");
    numsg::getMsg()->start("laundry");
    sleep(2);
    numsg::getMsg()->printf("test %s with %d things", "washer", 25);
    sleep(2);
    numsg::getMsg()->start("dishes");
    sleep(2);
    numsg::getMsg()->finish("dishes");
    sleep(2);
    numsg::getMsg()->finish("laundry");
    numsg::getMsg()->printf("Exiting:");
}
#endif
