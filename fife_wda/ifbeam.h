#ifndef FIFE_IFBEAM_H
#define FIFE_IFBEAM_H   1


#include <stdlib.h>

#include "wda.h"

#ifdef __cplusplus 
extern "C" { 
#endif 

/*
 * Functions for IFBeam DB variable bundles
 */
Dataset getBundleForTime(const char *url, const char *bundle, const double t, int *error);

Dataset getBundleForInterval(const char *url, const char *bundle, const double t0, const double t1, int *error);

Dataset getEventVarForTime(const char *url, const char *event, const char *var, const double t, int *error);

Dataset getEventVarForInterval(const char *url, const char *event, const char *var, const double t0, const double t1, int *error);

#ifdef __cplusplus 
} 
#endif 

#endif
