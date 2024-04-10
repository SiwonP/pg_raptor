#include "interval_utils.h"

Interval *intervalSub(Interval *interval1, Interval *interval2)
{
    Interval *interval;
    interval = malloc(sizeof(Interval));

    interval->day = 0;
    interval->month = 0;

    interval->time = interval1->time - interval2->time;

    return interval;
}

int intervalCmp(Interval *interval1, Interval *interval2)
{
    int result = 0;

    if (interval1->time < interval2->time)
    {
        result = -1;
    } else 
    {
        result = 1;
    }

    if (interval1->time == interval2->time)
    {
        result = 0;
    }

    return result;
}

