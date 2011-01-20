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
   int i, n, chan;
   //double lookuptime = 1290651324.1;
   double lookuptime = 1283848809.799920;
   double halfhour = 30 * 60;

   //WebAPI::_debug = 1;
   //Folder::_debug = 1;


   Folder f("pedcal", "http://rexdb01.fnal.gov:8088/wsgi/IOVServer");
   
   cout << setiosflags(ios::fixed) << setfill(' ') << setprecision(4) ;

   timestamp();
   cout << "starting...\n";

   for(i = 1; i < 10; i++ ) {
       for (chan = 1000110; chan < 1000170; chan+=20) {

	   f.getChannelData(
		  lookuptime + halfhour * i, 
		  chan, 
                  &chbits[0], &chbits[1], &chbits[2], &chbits[3], &chbits[4], &chbits[5],
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
}

int
main() {
   doit();
   cout << "finishing...\n";
   timestamp();
}
