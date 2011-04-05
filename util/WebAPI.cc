#include "nucondb.h"
#include "WebAPI.h"
#include <fstream>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <iomanip>
#include <ext/stdio_filebuf.h>

// debug flag
int WebAPI::_debug(0);

//
// initialize exception
// - zero fill whole structure
// - set message and tag fields
// this works if we're subclassed from GaudiException or our
// SimpleException...
//
WebAPIException::WebAPIException( std::string message, std::string tag ) throw() {
   // memset(this, 0, sizeof(*this));
   m_message = message;
   m_tag = tag;
}

std::ostream& operator<< ( std::ostream& os , const SimpleExceptionSuper  *pse )
 { pse && (os << pse->tag() << pse->message()) ; return os; }

// parseurl(url)
//   parse a url into 
//   * type/protocol, 
//   * host
//   * port
//   * path
//   so that it can be fetched directly

WebAPI::parsed_url 
WebAPI::parseurl(std::string url) throw(WebAPIException) {
     int i, j;                // string indexes
     WebAPI::parsed_url res;  // resulting pieces
     std::string part;        // partial url

     i = url.find_first_of(':');
     if (url[i+1] == '/' and url[i+2] == '/') {
        res.type  = url.substr(0,i);
     } else {
        throw(WebAPIException(url,"BadURL: has no slashes, must be full URL"));
     }
     if (res.type != "http" && res.type != "https" ) {
        throw(WebAPIException(url,"BadURL: only http: and https: supported"));
     }
     part = url.substr(i+3);
     i = part.find_first_of(':');
     j = part.find_first_of('/');
     if( i < 0 || i > j) {
	 // no port number listed, dedault to 80 or 443
         res.host = part.substr(0,j);
         res.port = (res.type == "http") ?  80 : 443;
     } else {
	 res.host = part.substr(0,i);
         res.port = atol(part.substr(i+1,j-i).c_str());
    }
    res.path = part.substr(j);
    return res;
}

// we need lots of system network bits
// to make a network connection...
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// fetch a URL, opening a filestream to the content
// we do klugy looking things here to directly return
// the network connection, rather than saving he data
// in a file and returning that.

WebAPI::WebAPI(std::string url) throw(WebAPIException) {
     int s;			// unix socket file descriptor
     WebAPI::parsed_url pu;     // parsed url.
     struct sockaddr_in server; // connection address struct
     struct hostent *hostp;     // gethostbyname() result
     static char buf[512];      // buffer for header lines
     int status;                // http status
     int retries;
     int redirectflag = 1;
     int hcount;
     __gnu_cxx::stdio_filebuf<char> *buf_out = 0;

     _debug && std::cout << "fetchurl: " << url << std::endl;
     _debug && std::cout.flush();
     retries = 0;

     while( redirectflag ) {
         hcount = 0;
         status = 500;

         // stuff from ye olde BSD IP Tutorial...
         s = socket(AF_INET, SOCK_STREAM,0);

         pu = parseurl(url);

         if (pu.type == "http") {
             // connect directly
	     hostp = gethostbyname(pu.host.c_str());
	     if (!hostp) {
		 retries++;
		 if (retries > 14) {
		     throw(WebAPIException(url,"FetchError: Retry count exceeded, gethostbyname failed"));
		 }
		 _debug && std::cout << "gethostname failed , waiting ..." << retries << std::endl;
		 _debug && std::cout.flush();
		 sleep(1);
		 continue;
	     }
	     _debug && std::cout << "looking up host " << pu.host << " got " << hostp->h_name << "\n";

	     server.sin_family = AF_INET;
	     server.sin_port = htons(pu.port);
	     memcpy( &server.sin_addr, hostp->h_addr, hostp->h_length);

	     retries = 0;
	     while (connect(s,(struct sockaddr *)&server,sizeof(server)) < 0) {
		 retries++;
		 if (retries > 14) {
		     throw(WebAPIException(url,"FetchError: Retry count exceeded, connect failed"));
		 }
		 _debug && std::cout << "connect failed , waiting ...";
		 sleep(5 << retries);
		 _debug && std::cout << "retrying ...\n";
	     }

	     // start of black magic -- use stdio_filebuf class to 
	     // attach to socket...
	     _buf_in = new  __gnu_cxx::stdio_filebuf<char> (dup(s), std::fstream::in|std::fstream::binary); 
	     if (!_buf_in) {
		throw(WebAPIException(url,"MemoryError: new failed"));
	     }
	     _fromsite.std::ios::rdbuf(_buf_in);

	     buf_out = new  __gnu_cxx::stdio_filebuf<char>(dup(s), std::fstream::out|std::fstream::binary); 
	     if (!buf_out) {
		throw(WebAPIException(url,"MemoryError: new failed"));
	     }
	     _tosite.std::ios::rdbuf(buf_out);


	     close(s);		      // don't need original dd anymore...
	     // end of black magic
         } else if (pu.type == "https") {

            // XXX How do we detect/retry https fails?

            int inp[2], outp[2], pid;
            pipe(inp);
            pipe(outp);
            if (0 == (pid = fork())) {
                // child -- run openssl s_client
                const char *proxy = getenv("X509_USER_PROXY");
                std::stringstream hostport;

                hostport << pu.host << ":" << pu.port;

                _debug && std::cout << "openssl"<< ' ' << "s_client"<< ' ' << "-connect"<< ' ' << hostport.str().c_str() << ' ' << "-cert"<< ' ' << proxy << ' ' << "-quiet" ;

                std::cout.flush();

                // fixup file descriptors so our in/out are pipes
                close(0);      
                dup(inp[0]);   
                close(1);
                dup(outp[1]);

                close(inp[0]); close(inp[1]); 
                close(outp[0]);close(outp[1]);

                // run openssl...
                execlp("openssl", "s_client", "-connect", hostport.str().c_str(), "-cert", proxy, "-quiet", 0);
                exit(-1);
            } else {
                // parent, fix up pipes, make streams
                close(inp[0]);  
                close(outp[1]);  
	        _buf_in = new  __gnu_cxx::stdio_filebuf<char> (outp[0], std::fstream::in|std::fstream::binary); 
		 if (!_buf_in) {
		    throw(WebAPIException(url,"MemoryError: new failed"));
		 }
	         _fromsite.std::ios::rdbuf(_buf_in);

                 buf_out = new  __gnu_cxx::stdio_filebuf<char>(inp[1], std::fstream::out|std::fstream::binary);
		 if (!buf_out) {
		    throw(WebAPIException(url,"MemoryError: new failed"));
		 }
		 _tosite.std::ios::rdbuf(buf_out);
            }
         } else {
            throw(WebAPIException(url,"BadURL: only http: and https: supported"));
         }

	 // now some basic http protocol
	 _tosite << "GET " << pu.path << " HTTP/1.0\r\n";
	 _tosite << "Host: " << pu.host << "\r\n";
	 _tosite << "\r\n";
	 _tosite.flush();

         _debug && std::cout << "sent request\n";

	 do {
	    _fromsite.getline(buf, 512);
            hcount++;

	    _debug && std::cout << "got header line " << buf << "\n";

	    if (strncmp(buf,"HTTP/1.", 7) == 0) {
		status = atol(buf + 8);
	    }

	    if (strncmp(buf, "Location: ", 10) == 0) {
		if (buf[strlen(buf)-1] == '\r') {
		    buf[strlen(buf)-1] = 0;
		}
		url = buf + 10;
	    }

	 } while (_fromsite.gcount() > 2 || hcount < 3); // end of headers is a blank line

	 _debug && std::cout << "http status: " << status << std::endl;

	 _tosite.close();
         if (buf_out) 
             delete buf_out;

	 if (status < 301 || status > 303 ) {
            redirectflag = 0;
	 } else {
	     // we're going to redirect again, so close the _fromsite side
	     _fromsite.close();
             delete _buf_in;
         }
     }
}

WebAPI::~WebAPI() {
    _tosite.close();
    delete _buf_in;
}

void
test_WebAPI_fetchurl() {
   std::string line;


   WebAPI ds("http://www-css.fnal.gov/~mengel/Ascii_Chart.html");

    std::cout << "ds.data().eof() is " << ds.data().eof() << std::endl;
    while(!ds.data().eof()) {
        getline(ds.data(), line);

        std::cout << "got line: " << line << std::endl;;
   }
   std::cout << "ds.data().eof() is " << ds.data().eof() << std::endl;
   ds.data().close();

   WebAPI ds2("http://home.fnal.gov/~mengel/Ascii_Chart.html");

    while(!ds2.data().eof()) {
        getline(ds2.data(), line);

        std::cout << "got line: " << line << std::endl;;
   }
   std::cout << "ds.data().eof() is " << ds2.data().eof() << std::endl;

   try {
      WebAPI ds3("https://plone4.fnal.gov/P1/Main/");
      while(!ds3.data().eof()) {
	    getline(ds3.data(), line);

	    std::cout << "got line: " << line << std::endl;;
      }
   } catch (WebAPIException we) {
      std::cout << &we << std::endl;
   }

   try {
      WebAPI ds4("http://nosuch.fnal.gov/~mengel/Ascii_Chart.html");
   } catch (WebAPIException we) {
      std::cout << &we << std::endl;
   }
   try {
      WebAPI ds5("borked://nosuch.fnal.gov/~mengel/Ascii_Chart.html");
   } catch (WebAPIException we) {
      std::cout << &we << std::endl;
   }
}

#ifdef UNITTEST

int
main() {
   WebAPI::_debug = 1;
   test_WebAPI_fetchurl();
}
#endif
