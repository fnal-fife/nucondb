#include <fstream>
#include <string>
#include <ext/stdio_filebuf.h>

class WebAPI {
    std::fstream _tosite, _fromsite;
    __gnu_cxx::stdio_filebuf<char> *_buf_in;
public:
    static int _debug;
    WebAPI(std::string url);
    ~WebAPI();
    std::fstream &data() { return _fromsite; }
    
    struct parsed_url {
	 std::string type;
	 std::string host;
	 int port;
	 std::string path;
    };
    static parsed_url parseurl(std::string url);

};

