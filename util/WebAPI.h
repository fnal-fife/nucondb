#ifndef WEB_API_H
#define WEB_API_H 1

#include <fstream>
#include <string>
#include <ext/stdio_filebuf.h>

#ifdef  HAVE_GAUDI_EXCEPTIONS

#include "GaudiKernel/GaudiException.h"
#define ExceptionSuper GaudiException

#else

class SimpleExceptionSuper : virtual public std::exception {
protected:
     std::string m_message;
     std::string m_tag;
public:
     virtual ~SimpleExceptionSuper() throw() {;};
     std::string message() const { return m_message; }
     std::string tag() const { return m_tag; }
};

extern std::ostream& operator<< ( std::ostream& os , const SimpleExceptionSuper  *pse );
 
#define ExceptionSuper SimpleExceptionSuper

#endif

class WebAPIException : public ExceptionSuper {
public:
   WebAPIException( std::string message, std::string tag) throw();
   virtual ~WebAPIException() throw() {;};
};

class WebAPI {
    std::fstream _tosite, _fromsite;
    __gnu_cxx::stdio_filebuf<char> *_buf_in;
public:
    static int _debug;
    WebAPI(std::string url) throw(WebAPIException);
    ~WebAPI();
    std::fstream &data() { return _fromsite; }
    
    struct parsed_url {
	 std::string type;
	 std::string host;
	 int port;
	 std::string path;
    };
    static parsed_url parseurl(std::string url) throw(WebAPIException);

};

#endif
