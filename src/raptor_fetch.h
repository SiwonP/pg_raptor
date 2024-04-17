#ifndef RAPTOR_FETCH_H
#define RAPTOR_FETCH_H

#include "postgres.h"
#include "fmgr.h"

#include "executor/spi.h"
#include "utils/builtins.h"
#include "funcapi.h"

#include "raptor_structure.h"
#include "interval_utils.h"

#define QUERY_SIZE 512
#define BIG_QUERY_SIZE 1024

void fetch_time(HeapTuple *tuple, TupleDesc *tupdesc, Interval **interval);

void fetch_stop(HeapTuple *tuple, TupleDesc *tupdesc, Stop_t *stop, int fetch_time);

void fetch_route(HeapTuple *tuple, TupleDesc *tupdesc, Route_t *route);

void fetch_trip(HeapTuple *tuple, TupleDesc *tupdesc, Trip_t *trip);

void get_from_stops(char *stop_id, Stop_t **stops, size_t *total_from_stops);

void get_transfer_from_stop_id(char *stop_id, Stop_t **stops);

void get_stops_on_route_after_stop_id(char *route_id, char *stop_id, Stop_t **stops, size_t *total_stops);

void get_departure_time(Trip_t *trip, char *stop_id, Interval **departure_time);

void get_arrival_time(Trip_t *trip, char *stop_id, Interval **arrival_time);

// void get_earliest_trip(char *stop_id, char *route_id, Interval *departure_time, char **trip_id);
void get_earliest_trip(char *stop_id, char *route_id, char *departure_time, char *departure_date, Trip_t **trips, size_t *total_trips);

void get_routes_traversing_stop_id(char *stop_id, Route_t **routes, size_t *total_routes);

int get_interval_from_string(char *string, Interval *interval);

#endif