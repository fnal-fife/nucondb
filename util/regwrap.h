#include <regex.h>
#include <string>

namespace regex {

class regmatch {
     int _matched;
     int _nslots;
     regmatch_t *_data;
     std::string &_against;
public:
     regmatch(int n, std::string &against);
     regmatch(const regmatch &r);
     ~regmatch();
     void set_matched(int n);
     std::string operator[] ( int n );
     operator int();
     regmatch_t *data();

};

class regexp {
     regex_t _re;
public:
     regexp(std::string re);
     ~regexp();
     regmatch operator ()( std::string &against);
};

}

using namespace regex;
