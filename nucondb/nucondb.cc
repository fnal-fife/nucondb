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

static char ebuf[64];

// initialize folder with folder name, server url
Folder::Folder( std::string name, std::string url ) throw(WebAPIException) {
   _foldername = name;
   _url = url;
   _cache_key = -1;
   _cache_start = 0;
   _cache_end = 0;
}

// get key for given time
long int
Folder::getKey(double when)  throw(WebAPIException){
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
Folder::getTimes(double when)  throw(WebAPIException){
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

    if (res.size() > 0 && res.front().when > when ) {
         sprintf(ebuf, "Time %f:", when);
         throw(WebAPIException(ebuf, "not in returned timelist -- Assertion error"));
    }

    return res;
}

// utility, split a string into a list -- like perl/python split()

std::vector<std::string>
split(std::string s, char c ){
   size_t pos, p2;
   pos = 0;
   std::vector<std::string> res;
   while( std::string::npos != (p2 = s.find(c,pos)) ) {
	res.push_back(s.substr(pos, p2 - pos));
        pos = p2 + 1;
   }
   res.push_back(s.substr(pos));
   return res;
}

// get all channels for time nearest when into cache
void
Folder::fetchData(long key) throw(WebAPIException) {
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
Folder::fetchData(double when)  throw(WebAPIException){
    std::string columnstr;
    long int key = 1;

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
       sprintf(ebuf, "Time %f: ", when);
       throw(WebAPIException(ebuf, "not found in database."));
    }

    // search list of times for nearest one to us
    // if it isn't off the end
    if (when > _times.back().when) {
       key = _times.back().key;
    } else {
	for(unsigned int i = 0; i < _times.size()-1 ; i++ ) {
	    if ( when >= _times[i].when and when < _times[i+1].when) {
               key = _times[i].key;
	    }
	}
    }

    this->fetchData(key);
}


int
Folder::parse_fields(const char *pc, va_list al) {
     void *vp;
     int res;
     std::vector<std::string>::iterator it;
     res = vsscanf(pc, format_string(), al);

     return res;
}

// getChannelData --
// first frob the cache for data for requested time
// then lookup channel data in cache
// finally unpack data with vsprintf

int
Folder::getChannelData(double t, int chan, ...) throw(WebAPIException) {
    va_list al;
    int l, m, r;
    int val;
    int comma;
    int res;

    va_start(al, chan);

    fetchData(t);

    l=0; r = _n_datarows-1;
    m = (l + r + 1)/2;

    if (r < l) {
       sprintf(ebuf, "time %f: ", t);
       throw(WebAPIException(ebuf, "Data not found in database."));
    }

    // binary search for channel...
    while( l < r ) {
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

     if (m >= _n_datarows) {
         sprintf(ebuf, "Channel %d: ", chan);
         throw(WebAPIException(ebuf , "not found in data."));
     }

     _debug && std::cout << "found slot " << m ;
     _debug && std::cout.flush();
     _debug && std::cout << ": " << _cache_data[m] << std::endl;

     _debug && std::cout.flush();

     comma = _cache_data[m].find_first_of(',');
     val = atol(_cache_data[m].c_str());
     if (val == chan) {
         return this->parse_fields(_cache_data[m].c_str() + comma + 1, al);
     } else {
         sprintf(ebuf, "Channel %d: ", chan);
         throw(WebAPIException(ebuf , "not found in data."));
     }
}

// give a scanf format string for the current data types
char *
Folder::format_string() {
    std::vector<std::string>::iterator it;
    static char buf[2048];
    char *sep = (char *)"";

    buf[0] = 0;
    
    _debug && std::cout << "format_string: ";
    for( it = _types.begin(), it++; it != _types.end(); it++ ) {
        _debug && std::cout << "type: " << *it << "\n";
        _debug && std::cout.flush();
        strcat(buf, sep);
        switch((*it)[0]) {
        case 'd': strcat(buf, "%lf"); break; // double precision
        case 's': strcat(buf, "%f");  break; // single precision
        case 'f': strcat(buf, "%f");  break; // float
        // case 't': strcat(buf, "\"%a[^\"]\"");  break; // text
        case 't': strcat(buf, "%a[^,]");  break; // text
        case 'i': strcat(buf, "%d");  break; // integer
        case 'l': strcat(buf, "%ld"); break; // long integer
        default:  strcat(buf, "%ld"); break; // unknowns are long(?)
        }
        sep = (char *)",";
    }
    strcat(buf,"\\n");
    _debug && std::cout << "\n got: " << buf << "\n";
    _debug && std::cout.flush();

    return buf; 
}

void
test_gettimes(Folder &d) {
   std::vector<Folder::tk> tl;   
   std::vector<Folder::tk>::iterator it;
   char *sep;
  
   tl = d.getTimes((double)time(0));
   std::cout << "got time list: ";
   sep = (char *)"{";
   for( it = tl.begin(); it != tl.end(); it++) {
       std::cout << sep << it->when << ":" << it->key;
       sep = (char *)",";
   }
   std::cout << "}\n";
}

static int channellist[] =  {
      1052672, 6715360,1052704,1052736,1052768,1052800,1052832,1052864,1052896,1052928,1052960,
   };

void
test_getchanneldata_window(Folder &f) {
   static double  d1, d2, d3, d4, d5, d6, d7, d8, d9;
   int i1, i2, i3, i4, i5;

   std::cout << "looking for first time:" << "\n";
   f.getChannelData(1283590373.9924,1052704,
	         &i1, &i2, &i3, &i4, &i5,
                 &d1, &d2, &d3, &d4, &d5, &d6, &d7, &d8, &d9);
   std::cout << "looking for second time:" << "\n";
   f.getChannelData(1283592173.9924,1052704,
	         &i1, &i2, &i3, &i4, &i5,
                 &d1, &d2, &d3, &d4, &d5, &d6, &d7, &d8, &d9);

   try {
   std::cout << "looking for nonexistant channel:" << "\n";
   f.getChannelData(1283592173.9924,1052703,
	         &i1, &i2, &i3, &i4, &i5,
                 &d1, &d2, &d3, &d4, &d5, &d6, &d7, &d8, &d9);
       std::cout << "Failed to get exception!";
       abort();
   } catch (WebAPIException we) {
       std::cout << "got exception:" << &we << "\n";
   }
}
  


void
test_getchanneldata(Folder &f) {
   static double  d1, d2, d3, d4, d5, d6, d7, d8, d9;
   int i1, i2, i3, i4, i5;
   int i;



   for (i = 0; i < 10; i++) {
	   f.getChannelData((double)time(0), channellist[i], 
                 &i1, &i2, &i3, &i4, &i5, 
		 &d1, &d2, &d3, &d4, &d5, &d6, &d7, &d8, &d9);

           std::cout << std::setiosflags(std::ios::fixed) << std::setfill(' ') << std::setprecision(4);
	   std::cout << "got for channel " << channellist[i] << ": " <<
	    std::setw(9)<<d1 << std::setw(9)<<d2 << std::setw(9)<<d3 << std::setw(9)<<d4 << std::setw(9)<<d5 << std::setw(9)<<d6 << std::setw(9)<<d7 << std::setw(9)<<d8 << std::setw(9)<<d9 << std::endl;
   }
   f.getChannelData((double)time(0), 8820704, 
	 &i1, &i2, &i3, &i4, &i5, 
	 &d1, &d2, &d3, &d4, &d5, &d6, &d7, &d8, &d9);

   std::cout << std::setiosflags(std::ios::fixed) << std::setfill(' ') << std::setprecision(4);
   std::cout << "got for channel " << 8820704 << ": " <<
    std::setw(9)<<d1 << std::setw(9)<<d2 << std::setw(9)<<d3 << std::setw(9)<<d4 << std::setw(9)<<d5 << std::setw(9)<<d6 << std::setw(9)<<d7 << std::setw(9)<<d8 << std::setw(9)<<d9 << std::endl;
}

static char pos[512],atten_factor[512],strip_condition[512], stuff[512];

void
test3() {
   int pos_num;
static int chbits[5];
static double d[8];
static char *text1;
static char *text2;
static char *text3;

   Folder d3("atten", "http://dbweb1.fnal.gov:8080/wsgi/IOVServer");
   d3.getChannelData(
	 1300969766.0,
         1210377216,
         &chbits[0], &chbits[1], &chbits[2], &chbits[3], &chbits[4], &d[0], &d[1], &d[2], &d[3], &d[4], &d[5], &d[6], &d[7], &pos_num, &text1, &text2, &text3
        );

     std::cout << std::setiosflags(std::ios::fixed) << std::setfill(' ') << std::setprecision(4);
     std::cout << "got:"
	  << std::setw(9) << chbits[0] << std::setw(9) << chbits[1] << std::setw(9) << chbits[2] << std::setw(9) << chbits[3] << std::setw(9) << chbits[4] << std::setw(9) << d[0] << std::setw(9) << d[1] << std::setw(9) << d[2] << std::setw(9) << d[3] << std::setw(9) << d[4] << std::setw(9) << d[5] << std::setw(9) << d[6] << std::setw(9) << d[7] << std::setw(9) << pos_num << "\n===\n" << std::setw(9) << text1 << "\n===\n" << std::setw(9) << text2 << "\n==\n" << std::setw(9) << text3 << std::setw(9) << "\n";

  free(text1);
  free(text2);
  free(text3);
}

#ifdef UNITTEST

char decode_test[] = "test%20this%23stuff";
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
   try {
           if (1) test3();
           if (1) {
	   Folder d2("pedcal", "http://dbweb1.fnal.gov:8080/wsgi/IOVServer");
	   test_gettimes(d2);
	   test_getchanneldata(d2);
	   test_getchanneldata_window(d2);
           }
   } catch (WebAPIException we) {
      std::cout << "Exception:" << &we << std::endl;
   }
 
   std::cout << "Done!";
   std::cout.flush();
         
}
#endif
