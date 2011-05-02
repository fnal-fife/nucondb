#include <sys/time.h>
#include <vector>
#include <list>
#include <sstream>
#include <fstream>
#include "WebAPI.h"

class Folder {
public:
     struct tk { double when; long int key; };
private:
     double _cache_start;
     double _cache_end;
     long int _cache_key;
     std::vector<std::string> _columns;
     std::vector<std::string> _types;
     std::vector<std::string> _cache_data;
     int _n_datarows;
     std::vector<tk> _times;
     std::string _url;
     std::string _foldername;

     int parse_fields(const char *, va_list);

public:
     static int _debug;
     // internal in back of getChannelData1
     std::vector<tk> 
	getTimes(double when) throw(WebAPIException);    // gets list of times in week surrounding when

     void fetchData(double when) throw(WebAPIException); // gets data for a time in cache
     void fetchData(long key) throw(WebAPIException);    // gets data for a time in cache

     Folder( std::string name, std::string url ) throw(WebAPIException); // bookkeeping...
     int getChannelData(double t, int chan, ...) throw(WebAPIException); // fetches data 
     char *format_string();
     long int getKey(double) throw(WebAPIException);
};

