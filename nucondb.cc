#include "nucondb.h"
#include "WebAPI.h"
#include <fstream>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <iomanip>

// debug flag
int pad;
int Folder::_debug(0);

// initialize folder with folder name, server url
Folder::Folder( std::string name, std::string url ) {
   _foldername = name;
   _url = url;
   _cache_start = 0;
   _cache_end = 0;
}

// get key for given time
long int
Folder::getKey(double when) {
    std::string st;
    std::stringstream fullurl;

    fullurl << std::setiosflags(std::ios::fixed);
    fullurl << _url << "/key?f=" << _foldername << "&t=" << when ;

    WebAPI s( fullurl.str() );

    (void)getline(s.data(), st); // skip header...

    getline(s.data(),st);
    return atol(st.c_str());
}

// get list of boundaries near given time from server
std::vector<Folder::tk>
Folder::getTimes(double when) {
    const int seconds_in_week = 7 * 24 * 60 * 60;
    double weekstart;
    std::vector<tk> res;
    std::string st;
    tk cur;

    weekstart = (long)when - (long)when % seconds_in_week;
    
    std::stringstream fullurl;

    fullurl << std::setiosflags(std::ios::fixed);
    fullurl << _url << "/times?f=" << _foldername << "&t=" << weekstart 
                    << "&d=" << seconds_in_week ;

    WebAPI s( fullurl.str() );

    (void)getline(s.data(), st); // skip header...

    while(!s.data().eof()) {
         getline(s.data(),st);
         if (!s.data().eof()) {
             _debug && std::cout << "gettimes got: " << st << std::endl;
             cur.key = atol(st.c_str());
             cur.when = atof(st.c_str() + st.find(',',0)+1);
             res.push_back(cur);
         }
    }

    if (res.size() > 0 && (res.front().when > when || res.back().when <= when)) {
         // throw(assert_error("timelist doesn't contain requested time");
         ;
    }

    return res;
}

// utility, split a string into a list -- like perl/python split()

std::vector<std::string>
split(std::string s, char c ){
   int pos, p2;
   pos = 0;
   std::vector<std::string> res;
   while( std::string::npos != (p2 = s.find(c,pos)) ) {
	res.push_back(s.substr(pos, p2 - pos - 1));
        pos = p2 + 1;
   }
   res.push_back(s.substr(pos));
   return res;
}

// get all channels for time nearest when into cache
void
Folder::fetchData(long key) {
    std::string columnstr;
    if (key == _cache_key) {
        return;
    }
    std::stringstream fullurl;
    fullurl << std::setiosflags(std::ios::fixed);
    fullurl << _url << "/data?f=" << _foldername << "&i=" << key ;
    WebAPI s( fullurl.str() );

    _cache_key = key;

    // get start and end times
    getline(s.data(), columnstr); 
    _debug && std::cout << "got: " << columnstr << std::endl;;
    _cache_start = atof(columnstr.c_str());

    getline(s.data(), columnstr);
    _debug && std::cout << "got: " << columnstr << std::endl;;
    _cache_end = atof(columnstr.c_str());
    
    // get column names
    getline(s.data(), columnstr);
    _debug && std::cout << "got: " << columnstr << std::endl;
    _columns = split(columnstr,',');
    // get column types
    getline(s.data(), columnstr);
    _debug && std::cout << "got: " << columnstr << std::endl;
    _types = split(columnstr,',');

    _n_datarows = 0;
    while (!s.data().eof()) {

       getline(s.data(), columnstr);

       if (!s.data().eof() && !s.data().fail() && columnstr.c_str()) {
           _debug && std::cout << "got: " << columnstr << std::endl;
	   _cache_data.push_back(columnstr);
	   _n_datarows++;
       }
   }
   s.data().close();
}

// get all channels for time nearest when into cache
void
Folder::fetchData(double when) {
    double reftime;
    std::string columnstr;
    long int key;

    if (_cache_start <= when  && when < _cache_end) {
        // its in the cache already...
        return;
    }

    // only get new time list if needed
    if (_times.size() == 0 || (when < _times.front().when || when > _times.back().when) ) {
       _times = getTimes(when);
    }

    if (_times.size() == 0) {
       _cache_start = 0;
       _cache_end = 0;
       _n_datarows = 0;
       return;
    }

    // search list of times for nearest one to us
    // if it isn't off the end
    if (when > _times.back().when) {
       key = _times.back().key;
    } else {
	for(int i = 0; i < _times.size()-1 ; i++ ) {
	    if ( when >= _times[i].when and when < _times[i+1].when) {
               key = _times[i].key;
	    }
	}
    }

    this->fetchData(key);
}

// getChannelData --
// first frob the cache for data for requested time
// then lookup channel data in cache
// finally unpack data with vsprintf

int
Folder::getChannelData(double t, int chan, ...) {
    va_list al;
    int l, m, r;
    int val;
    int comma;

    va_start(al, chan);

    fetchData(t);

    l=0; r = _n_datarows-1;
    m = (l + r + 1)/2;

    if (r < l) {
       // throw something -- no data to pull from
       return 0;
    }

    // binary search for channel...
    while( l < m && m < r ) {
        _debug && std::cout << "searching l: " << l 
                           << " m: " << m 
                           << " r: " << r;
        _debug && std::cout.flush();

        val = atol(_cache_data[m].c_str());
        _debug && std::cout << " val: " << val 
                  << std::endl;
        if( val  > chan )  r = m - 1;
        if( val == chan )  l = r = m;
        if( val  < chan )  l = m + 1;

        m = (l + r + 1)/2;
     }

     _debug && std::cout << "found slot " << m ;
     _debug && std::cout.flush();
     _debug && std::cout << ": " << _cache_data[m] << std::endl;

     _debug && std::cout.flush();

     comma = _cache_data[m].find_first_of(',');
     val = atol(_cache_data[m].c_str());
     if (val == chan) {
         return vsscanf(_cache_data[m].c_str() + comma + 1, format_string(), al);
     } else {
         // throw something -- channel not found
         return 0;
     }
}

// give a scanf format string for the current data types
char *
Folder::format_string() {
    std::vector<std::string>::iterator it;
    static char buf[1024];
    char *sep = "";

    buf[0] = 0;
    
    _debug && std::cout << "format_string: ";
    for( it = _types.begin(), it++; it != _types.end(); it++ ) {
        _debug && std::cout << "type: " << *it;
        strcat(buf, sep);
        switch((*it)[0]) {
        case 'd': strcat(buf, "%lf"); break; // double precision
        case 's': strcat(buf, "%f");  break; // single precision
        case 'f': strcat(buf, "%f");  break; // float
        case 't': strcat(buf, "%s");  break; // text
        case 'i': strcat(buf, "%d");  break; // integer
        case 'l': strcat(buf, "%ld"); break; // long integer
        default:  strcat(buf, "%ld"); break; // unknowns are long(?)
        }
        sep = ",";
    }
    strcat(buf,"\n");
    _debug && std::cout << "\n got: " << buf;

    return buf; 
}

void
test_gettimes(Folder &d) {
   std::vector<Folder::tk> tl;   
   std::vector<Folder::tk>::iterator it;
   char *sep;
  
   tl = d.getTimes((double)time(0));
   std::cout << "got time list: ";
   sep = "{";
   for( it = tl.begin(); it != tl.end(); it++) {
       std::cout << sep << it->when << ":" << it->key;
       sep = ",";
   }
   std::cout << "}\n";
}

void
test_getchanneldata_window(Folder &f) {
   char buf[512];
   static double  d1, d2, d3, d4, d5, d6, d7, d8, d9;
   int i1, i2, i3, i4, i5;
   int i, n;

   std::cout << "looking for first time:" << "\n";
   f.getChannelData(1283590373.9924,1000110,
	         &i1, &i2, &i3, &i4, &i5,
                 &d1, &d2, &d3, &d4, &d5, &d6, &d7, &d8, &d9);
   std::cout << "looking for second time:" << "\n";
   f.getChannelData(1283592173.9924,1000110,
	         &i1, &i2, &i3, &i4, &i5,
                 &d1, &d2, &d3, &d4, &d5, &d6, &d7, &d8, &d9);

}

void
test_getchanneldata(Folder &f) {
   char buf[512];
   static double  d1, d2, d3, d4, d5, d6, d7, d8, d9;
   int i1, i2, i3, i4, i5;
   int i, n;


   for (i = 10; i < 70; i+=10) {
	   f.getChannelData((double)time(0), 1000100 + i, 
                 &i1, &i2, &i3, &i4, &i5, 
		 &d1, &d2, &d3, &d4, &d5, &d6, &d7, &d8, &d9);

           std::cout << std::setiosflags(std::ios::fixed) << std::setfill(' ') << std::setprecision(4);
	   std::cout << "got for channel " << i << ": " <<
	    std::setw(9)<<d1 << std::setw(9)<<d2 << std::setw(9)<<d3 << std::setw(9)<<d4 << std::setw(9)<<d5 << std::setw(9)<<d6 << std::setw(9)<<d7 << std::setw(9)<<d8 << std::setw(9)<<d9 << std::endl;
   }
}

#ifdef UNITTEST
int
main() {

   WebAPI::_debug = 1;
   Folder::_debug = 1;

   std::cout << std::setiosflags(std::ios::fixed);

   //Folder d("myfolder", "http://www-oss.fnal.gov/~mengel/testcool");
   //test_gettimes(d);
   //test_getchanneldata(d);

   //std::cout << "Now the real test...\n";

   //Folder d2("sample32k", "http://rexdb01.fnal.gov:8088/IOVServer/IOVServerApp.py");
   Folder d2("pedcal", "http://rexdb01.fnal.gov:8088/wsgi/IOVServer");
   test_gettimes(d2);
   test_getchanneldata(d2);
   test_getchanneldata_window(d2);
}
#endif
