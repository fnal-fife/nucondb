#define _GNU_SOURCE 

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include <curl/curl.h>

#include "ifbeam_c.h"

/*
 * Returns dataset for the bundle from IFBeam DB for the specified time
 */
Dataset getBundleForTime(const char *url, const char *bundle, const double t, int *error) {

    char sbuf[512];

    snprintf(sbuf, sizeof (sbuf)-2, "%s/data?b=%s&t=%.3f", url, bundle, t);

    return getData(sbuf, NULL, error);
}

/*
 * Returns dataset for the bundle from IFBeam DB for the specified time range
 */
Dataset getBundleForInterval(const char *url, const char *bundle, const double t0, const double t1, int *error) {

    char sbuf[512];

    snprintf(sbuf, sizeof (sbuf)-2, "%s/data?b=%s&t0=%.3f&t1=%.3f", url, bundle, t0, t1);

    return getData(sbuf, NULL, error);
}


/*
 * Returns dataset for the variable in event from IFBeam DB for the specified time
 */
Dataset getEventVarForTime(const char *url, const char *event, const char *var, const double t, int *error) {

    char sbuf[512];

    snprintf(sbuf, sizeof (sbuf)-2, "%s/data?e=%s&v=%s&t=%.3f&f=csv", url, event, var, t);

    return getData(sbuf, NULL, error);
}


/*
 * Returns dataset for the variable in event from IFBeam DB for the specified time range
 */
Dataset getEventVarForInterval(const char *url, const char *event, const char *var, const double t0, const double t1, int *error) {

    char sbuf[512];

    snprintf(sbuf, sizeof (sbuf)-2, "%s/data?e=%s&v=%s&t0=%.3f&t1=%.3f&f=csv", url, event, var, t0, t1);

    return getData(sbuf, NULL, error);
}

