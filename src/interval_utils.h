#ifndef INTERVAL_UTILS_H
#define INTERVAL_UTILS_H

#include "postgres.h"
#include "fmgr.h"


#define raptor_PG_RETURN_INTERVAL(X) PG_RETURN_DATUM((Datum)(X))

Interval *intervalSub(Interval *interval1, Interval *interval2);

int intervalCmp(Interval *interval1, Interval *interval2);

static inline Interval *raptor_DatumGetInterval(Datum X)
{
    return (Interval *)X;
}

static inline Datum raptor_IntervalGetDatum(Interval *X)
{
    return (Datum)X;
}


#endif