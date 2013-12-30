#ifndef FIFE_IFBEAM_H
#define FIFE_IFBEAM_H   1


#include <stdlib.h>
#include <time.h>

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

typedef struct {
	long clock;			// Measurement epoch time
	char device[16];	// Device name
	char units[16];		// Units
	double value;		// Scalar device value && vector_size==0
    size_t vector_size;	// Number of elements in data array
	double vector[1];	// Vector device values && vector_size > 0
} *Measurement;

/*
 * Functions to work with the tuples in the dataset
 */
int getNmeasurements(Dataset dataset);            	/* Returns number of measurements in the dataset 	*/

Measurement getMeasurement(Dataset dataset, int i); /* Returns NULL if out of range             		*/

Measurement getFirstMeasurement(Dataset dataset);   /* Returns the first tuple in the dataset   		*/

Measurement getNextMeasurement(Dataset dataset);    /* Returns the next tuple if available      		*/

/*
 * Utility functions
 */
void setUserAgent(char *text);                      /* Set User-Agent header for HTTP request           */

#ifdef __cplusplus 
} 
#endif 

#endif
