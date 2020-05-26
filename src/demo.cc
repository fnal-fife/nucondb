#include "nucondb.h"
#include "WebAPI.h"
#include <iostream>
#include <iomanip>

using namespace std;

void
timestamp() {
    static char buf[64];
    time_t t;
    char *p;

    t = time(0);
    p = ctime_r(&t, buf);
    p[20] = 0;
    std::cout << p;
    std::cout.flush();

}

struct _Setw _ = setw(9);


int
doit() {
   char buf[512];
   static double d[9];
   static int chbits[6];
   int n, chan;
   std::string fields("hg_mean,hg_rms,hg_entries,mg_mean,mg_rms,mg_entries,lg_mean,lg_rms,lg_entries");
   //double lookuptime = 1290651324.1;
   double lookuptime = 1283590373.9924;
   double halfhour = 30 * 60;
   static int channellist[] =  {
      34739968,34740000,34740032,34740064,34740096,34740128,34740160,34740192,34742272,34742304
   };



   WebAPI::_debug = 1;
   Folder::_debug = 1;

  try {
   Folder f("pedcal", "https://dbweb0.fnal.gov/IOVServer");
   
   cout << setiosflags(ios::fixed) << setfill(' ') << setprecision(4) ;

   timestamp();
   cout << "starting...\n";

   for(int i = 0; i < 10; i++ ) {
       for (int j = 0; j< 10; j++) {

	   f.getNamedChannelData(
		  lookuptime + halfhour * i, 
		  channellist[j],
                  fields,
  	          &d[0], &d[1], &d[2], &d[3], &d[4], 
		  &d[5], &d[6], &d[7], &d[8] 
              );

           timestamp();

	   cout << "got for channel " << chan << ": " 
	        << _ << d[0] << _ << d[1] << _ <<d[2] << _ << d[3] << _ << d[4] 
                << _ << d[5] << _ << d[6] << _ <<d[7] << _ << d[8] 
		// << " '" << buf << "'" 
                << endl;
       }
   }
 } catch (WebAPIException we) {
      std::cout << "Exception:" << &we << std::endl;
 } 
}

int
main() {
   doit();
   cout << "finishing...\n";
   timestamp();
}
