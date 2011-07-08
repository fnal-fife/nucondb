#include "nucondb.h"
#include "WebAPI.h"
#include <fstream>
#include <iostream>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
    if (columnstr[0] == '-') {
        _cache_end = 9999999999.90;  // a really long time from now
    } else {
        _cache_end = atof(columnstr.c_str());
    }
    
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

void
fixquotes(char *s, int debug) {
   char *p1 = s, *p2 = s;

   if (debug) {
       std::cout << "before: " << s << "\n";
   }
   if (*p1 == '"') {
      p1++;
      while(*p1) {
         if (*p1 == '"') {
            if (*(p1+1) == '"') {
               p1++;
            } else {
               *p2++ = 0;
               break;
            }
         }
         *p2++ = *p1++;
      }
   }
   if (debug) {
      std::cout << "after: " << s << "\n";
      std::cout.flush();
   }
}

//XXX still  needs to use names...
//
int
Folder::parse_fields(std::vector<std::string> names, const char *pc, va_list al) {
    void *vp;
    int i;
    int res;
    std::vector<std::string>::iterator it, nit, cit;
    std::vector<std::string> fields;

    fields = split(pc,',');
    for( nit = names.begin(); nit != names.end(); nit++ ) {
	vp = va_arg(al, void*);
        if (!vp) {
            break;
        }
	for( i = 0; i < _columns.size(); i++ ) {
	    if ( *nit == _columns[i] ) {
                _debug && std::cout << "found " << *nit << " matching " << _columns[i] <<"\n";
		switch(_types[i][0]) {
		case 'd': //double precision
		    *(double *)vp = atof(fields[i-1].c_str());
		    break;
		case 'f': //single precision
		    *(float *)vp = atof(fields[i-1].c_str());
		    break;
		case 't': //text
		    *(char**)vp = strdup(fields[i-1].c_str());
		    fixquotes(*(char**)vp, _debug);
		    break;
		case 'i': //integer
		    *(int*)vp = atoi(fields[i-1].c_str());
		    break;
		case 'l': //long int
		    *(long*)vp = atol(fields[i-1].c_str());
		    break;
		}
	    }
	}
    }
}

// get all but the first item of a vector..
std::vector<std::string> 
vector_cdr(std::vector<std::string> &vec) {
    std::vector<std::string>::iterator it = vec.begin();
    it++;
    std::vector<std::string> res(it, vec.end());
    return res;
}

// getChannelData --
// first frob the cache for data for requested time
// then lookup channel data in cache
// finally unpack data with vsprintf

int
Folder::getChannelData(double t, int chan, ...) throw(WebAPIException) {
    va_list al;
    va_start(al, chan);
    fetchData(t);
    return getNamedChannelData_va(t, chan, vector_cdr(_columns), al);
}

int
Folder::getNamedChannelData(double t, int chan, std::string names, ...) throw(WebAPIException) {
    va_list al;
    va_start(al, names);
    std::vector<std::string> namev = split(names,',');
    return getNamedChannelData_va(t, chan, namev, al);
}

int
Folder::getNamedChannelData_va(double t, int chan, std::vector<std::string> namev, va_list al) throw(WebAPIException) {
    int l, m, r;
    int val;
    int comma;
    int res;

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
         return this->parse_fields(namev, _cache_data[m].c_str() + comma + 1, al);
     } else {
         sprintf(ebuf, "Channel %d: ", chan);
         throw(WebAPIException(ebuf , "not found in data."));
     }
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
test_getchannel_feb() {
    float g_time = 1303853231.0;

     static int chbits[5];
     static double d[39];
     int hit_id = 2364544;
    try {
     Folder f("feb","http://dbweb0.fnal.gov:8080/IOVServer");
   

     f.getChannelData(g_time , hit_id, &chbits[0], &chbits[1], &chbits[2], &chbits[3], &chbits[4], &d[0], &d[1], &d[2],\
                        &d[3], &d[4], &d[5], &d[6], &d[7], &d[8], &d[9], &d[10], &d[11], &d[12], &d[13], &d[14], &d[15],\
                        &d[16], &d[17], &d[18], &d[19], &d[20], &d[21], &d[22], &d[23], &d[24], &d[25], &d[26], &d[27],\
                        &d[28], &d[29], &d[30], &d[31], &d[32], &d[33], &d[34], &d[35], &d[36], &d[37], &d[38]);
   
   } catch (WebAPIException we) {
       std::cout << "got exception:" << &we << "\n";
   }
}

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


void
test3() {
   int pos_num;
static int chbits[5];
static double d[8];
static char *text1;
static char *text2;
static char *text3;

   Folder d3("atten", "http://dbweb0.fnal.gov/IOVServer");
   d3.getNamedChannelData(
	 1300969766.0,
         1210377216,
         "atten,atten_error,amp,amp_error,reflect,reflect_error",
         
         &d[0], &d[1], &d[2], &d[3], &d[4], &d[5]
        );

     std::cout << std::setiosflags(std::ios::fixed) << std::setfill(' ') << std::setprecision(4);
     std::cout << "got by name: "
               << "atten,atten_error,amp,amp_error,reflect,reflect_error"
               << "\n"
               << d[0] << std::setw(9) << d[1] << std::setw(9) << d[2] 
               << std::setw(9) << d[3] << std::setw(9) << d[4] 
               << std::setw(9) << d[5] 
               << "\n";

   d3.getNamedChannelData(
	 1300969766.0,
         1210377216,
         "atten",
         
         &d[0]
        );

     std::cout << std::setiosflags(std::ios::fixed) << std::setfill(' ') << std::setprecision(4);
     std::cout << "got by name: "
               << "atten"
               << "\n"
               << d[0] 
               << "\n";

   d3.getNamedChannelData(
	 1300969766.0,
         1210377216,
         "reflect",
         
         &d[0]
        );

     std::cout << std::setiosflags(std::ios::fixed) << std::setfill(' ') << std::setprecision(4);
     std::cout << "got by name: "
               << "reflect"
               << "\n"
               << d[0] 
               << "\n";

}

#ifdef UNITTEST

char decode_test[] = "test%20this%23stuff";
int
main() {

   WebAPI::_debug = 1;
   Folder::_debug = 1;


   std::cout << std::setiosflags(std::ios::fixed);

   char tfq[] = "\"string with \"\" quotes\"";

   fixquotes(tfq,1);

   //Folder d("myfolder", "http://www-oss.fnal.gov/~mengel/testcool");
   //test_gettimes(d);
   //test_getchanneldata(d);

   //std::cout << "Now the real test...\n";

   //Folder d2("sample32k", "http://rexdb01.fnal.gov:8088/IOVServer/IOVServerApp.py");

   test_getchannel_feb();
   try {
           if (1) test3();
           if (0) {
	   Folder d2("pedcal", "http://dbweb0.fnal.gov/IOVServer");
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
