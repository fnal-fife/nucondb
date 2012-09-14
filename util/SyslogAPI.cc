#include "SyslogAPI.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

namespace ifdh_util_ns {

int SyslogAPI::_debug = 0;

SyslogAPI::SyslogAPI(char *desthost, int destport, int parentflag): _parentflag(parentflag) {
     struct sockaddr_in sa;
     struct hostent *hostp;

     memset((char *)&sa, 0, sizeof(sa));
     sa.sin_family = AF_INET;

     _socket = socket(AF_INET, SOCK_DGRAM, 0);
     bind(_socket, (struct sockaddr *)&sa, sizeof(sa));

     hostp = gethostbyname(desthost);
     _destaddr.sin_family = AF_INET;
     _destaddr.sin_port = htons(destport);
     memcpy((char *)&_destaddr.sin_addr, hostp->h_addr, hostp->h_length);
}

SyslogAPI::~SyslogAPI() {
     close(_socket);
}

ssize_t
SyslogAPI::send( int facility, int severity, const char *tag, const char *msg) {

     struct hostent *phe;
     char hostbuf[512];
     std::stringstream st;
     int pri = facility * 8 + severity;
     time_t t = time(0);
     ssize_t r;
     char *date = ctime(&t)+4;
     date[15] = 0;

     gethostname(hostbuf, 512);
     phe = gethostbyname(hostbuf);

     st << "<" << pri << ">";
     st << date;
     st << ' ' << phe->h_name  << ' ' ;
     st << tag << "[" << (_parentflag ? getppid() : getpid())  << "]: ";
     st << msg;

     return sendto(_socket, 
		st.str().c_str(), 
		st.str().length(), 
                0,
		(struct sockaddr*)&_destaddr, 
		sizeof(_destaddr));
}

};
#ifdef UNITTEST
main() {

   SyslogAPI s("localhost");
   s.send(17, 1, "SyslogAPI", "Client test message");
}
#endif
