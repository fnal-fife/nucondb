#ifndef SYSLOG_API_H
#define SYSLOG_API_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

namespace ifdh_util_ns {

class SyslogAPI {
    static int _debug;			// debugging flag
    int _socket;			// socket we send through
    struct sockaddr_in _destaddr;	// our destination host/port
    int _parentflag;			// use our parents pid rather than ours
public:
    SyslogAPI(char *desthost, int destport = 514, int parentflag = 0);
    ~SyslogAPI();
    ssize_t send( int facility, int severity, const char *tag, const char *msg);
					// actually send a syslog message
};

};

using namespace ifdh_util_ns;
#endif
