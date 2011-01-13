#include <sys/time.h>
#include <vector>
#include <list>
#include <sstream>
#include <fstream>


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


public:
     static int _debug;
     // internal in back of getChannelData1
     std::vector<tk> 
	getTimes(double when);  // gets list of times in week surrounding when

     void fetchData(double when);        // gets data for a time in cache
     void fetchData(long key);        // gets data for a time in cache

     Folder( std::string name, std::string url );    // bookkeeping...
				    // rounds when down to nearest 604800 sec bound
     int getChannelData(double t, int chan, ...); // fetches data via 
     char *format_string();
     long int Folder::getKey(double);
};

