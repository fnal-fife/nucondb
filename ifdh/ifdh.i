%module ifdh 

%include "std_string.i"

%include "std_vector.i"

namespace std {
    %template(vectors) vector<string>;
};

%{
#define SWIG_FILE_WITH_INIT
#include "ifdh.h"
%}

%include "ifdh.h"
