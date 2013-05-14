//#define _GNU_SOURCE 

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "ifbeam_c.h"

/*
 * Note: This code uses wda module. In order to compile it needs files wda.h and wda.o to be present.
 */

int main(void) 
{
    int i, j, k;
    int err;

    const char *url = "http://dbweb0.fnal.gov:8088/ifbeam/data";


    Dataset ds;
    Tuple tu;
    
    int len;
    int error;
    char ss[81920];
    double dd[4096];
    
    time_t t0 = time(NULL);
    ds = getBundleForInterval(url, "BNBShortTerm", t0-305, t0-300, &error);     // Get the data for bundle


    fprintf(stderr, "\nntuples=%d\n\n", getNtuples(ds));// Get the number of rows in the dataset
    
//  tu = getTuple(ds, 1);                               // Returns NULL if out of range
    tu = getFirstTuple(ds);                             // Get the very first row - contains the column names
    // If we get the names print them
    if (tu != NULL) {                                               // If everything is OK
        int nc = getNfields(tu);                                    // Get the number of columns in this row
        fprintf(stderr, "ncols=%d\n", nc);
        
        for (i = 0; i < nc; i++) {                                  // Loop to get column data as a string
            len = getStringValue(tu, i, ss, sizeof (ss), &err);     // Returns string length
            fprintf(stderr, "[%d]: l=%d, s='%s'\n", i, len, ss);    // Print the results
        }
        fprintf(stderr, "e=%s\n\n", strerror(err));                 // Was it OK?
    } 
    releaseTuple(tu);                                   // Do not forget to release the structure if you do not need it

    tu = getNextTuple(ds);                              // Get the next row, it contains the first data value in the dataset

    if (tu != NULL) {                                               // If everything is OK
        int nc = getNfields(tu);                                    // Get the number of columns in this row
        fprintf(stderr, "ncols=%d\n", nc);
        
        for (i = 0; i < nc; i++) {                                  // Loop to get column data as a string
            len = getStringValue(tu, i, ss, sizeof (ss), &err);     // Returns string length
            fprintf(stderr, "[%d]: l=%d, s='%s'\n", i, len, ss);    // Print the results
        }
        // Get double
        fprintf(stderr, "[3]: v=%f\n", getDoubleValue(tu, 3, &err));// Now extract the double value from the 3-rd column
        fprintf(stderr, "e=%s\n\n", strerror(err));                 // Was it OK?
    } else {
        fprintf(stderr, "No such tuple\n");
    }
    releaseTuple(tu);                                   // Do not forget to release the structure if you do not need it
    
    
    tu = getTuple(ds, 77);                              // Get the row with double array 

    if (tu != NULL) {                                               // If everything is OK
        int nc = getNfields(tu);                                    // Get the number of columns in this row
        fprintf(stderr, "ncols=%d\n", nc);
        
        // Get the first double
        fprintf(stderr, "[3]: v=%f\n", getDoubleValue(tu, 3, &err));// Now extract the double value from the 3-rd column 
        fprintf(stderr, "e=%s\n\n", strerror(err));                 // Was it OK?

        // Get double array
        len = getDoubleArray(tu, 3, dd, 4096, &err);                // Now extract the double array from the 3-rd column
        fprintf(stderr, "len=%d\n", len);                           // Print the length

        for (i = 0; i < len; i++) {                                 // Print the results
            fprintf(stderr, "%f, ", dd[i]);
        }
        fprintf(stderr, "\n");
        
        fprintf(stderr, "e=%s\n\n", strerror(err));                   // Was it OK?
    } else {
        fprintf(stderr, "No such tuple\n");
    }
    releaseTuple(tu);                                               // Do not forget to release the structure if you do not need it

    releaseDataset(ds);                                             // Release dataset to prevent memory leak!





    ds = getEventVarForInterval(url, "e,8f", "E:HMGPR", t0-305, t0-300, &error);     // Get the data for the variable in event
//  ds = getEventVarForTime    (url, "e,8f", "E:HMGPR", t0-305,         &error);     // Get the data for the variable in event
    fprintf(stderr, "\nntuples=%d\n\n", getNtuples(ds));// Get the number of rows in the dataset

    tu = getFirstTuple(ds);                             // Get the very first row - contains the column names
    // If we get the names print them
    if (tu != NULL) {                                               // If everything is OK
        int nc = getNfields(tu);                                    // Get the number of columns in this row
        fprintf(stderr, "ncols=%d\n", nc);
        
        for (i = 0; i < nc; i++) {                                  // Loop to get column data as a string
            len = getStringValue(tu, i, ss, sizeof (ss), &err);     // Returns string length
            fprintf(stderr, "[%d]: l=%d, s='%s'\n", i, len, ss);    // Print the results
        }
        fprintf(stderr, "e=%s\n\n", strerror(err));                 // Was it OK?
    } 
    releaseTuple(tu);                                   // Do not forget to release the structure if you do not need it

    tu = getNextTuple(ds);                              // Get the next row, it contains the first data value in the dataset

    if (tu != NULL) {                                               // If everything is OK
        int nc = getNfields(tu);                                    // Get the number of columns in this row
        fprintf(stderr, "ncols=%d\n", nc);
        
        for (i = 0; i < nc; i++) {                                  // Loop to get column data as a string
            len = getStringValue(tu, i, ss, sizeof (ss), &err);     // Returns string length
            fprintf(stderr, "[%d]: l=%d, s='%s'\n", i, len, ss);    // Print the results
        }
        // Get double
        fprintf(stderr, "[4]: v=%f\n", getDoubleValue(tu, 4, &err));// Now extract the double value from the 4-th column
        fprintf(stderr, "e=%s\n\n", strerror(err));                 // Was it OK?
    } else {
        fprintf(stderr, "No such tuple\n");
    }
    releaseTuple(tu);                                   // Do not forget to release the structure if you do not need it
    
    
    releaseDataset(ds);                                             // Release dataset to prevent memory leak!
    return 0;
}


