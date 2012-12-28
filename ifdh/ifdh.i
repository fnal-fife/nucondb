%module ifdh 

%include "std_string.i"

%include "std_vector.i"

namespace std {
    %template(vectors) vector<string>;
};

%exception { 
    try {
        $action
    } catch (ifdh_util_ns::WebAPIException &e) {
        std::string s("ifdh error: "), s2(e.what());
        s = s + s2;
        SWIG_exception(SWIG_RuntimeError, s.c_str());
    } catch (...) {
        SWIG_exception(SWIG_RuntimeError, "unknown exception");
    }
}

%{
#define SWIG_FILE_WITH_INIT
#include "ifdh.h"
%}

%include "ifdh.h"

