#include <fstream>
#include <string>

class WebAPI {
    std::fstream _tosite, _fromsite;

public:
    static int _debug;
    WebAPI(std::string url);
    std::fstream &data() { return _fromsite; }
    
    struct parsed_url {
	 std::string type;
	 std::string host;
	 int port;
	 std::string path;
    };
    static parsed_url parseurl(std::string url);

};

