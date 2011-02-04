#ifndef SYSLOG_API_H
#define SYSLOG_API_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

class SyslogAPI {
    static int _debug;			// debugging flag
    int _socket;			// socket we send through
    struct sockaddr_in _destaddr;	// our destination host/port
    int _parentflag;			// use our parents pid rather than ours
public:
    SyslogAPI::SyslogAPI(char *desthost, int destport = 514, int parentflag = 0);
    SyslogAPI::~SyslogAPI();
    ssize_t SyslogAPI::send( int facility, int severity, const char *tag, const char *msg);
					// actually send a syslog message
};
#endif
