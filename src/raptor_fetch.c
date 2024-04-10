#include "raptor_fetch.h"

void fetch_time(HeapTuple *tuple, TupleDesc *tupdesc, Interval **interval)
{
    bool is_null = 0;
    *interval = (Interval *)SPI_getbinval(*tuple, *tupdesc, 1, &is_null);
}

void fetch_stop(HeapTuple *tuple, TupleDesc *tupdesc, Stop_t *stop, int fetch_time)
{
    stop->stop_id = SPI_getvalue(*tuple, *tupdesc, 1);
    stop->stop_name = SPI_getvalue(*tuple, *tupdesc, 2);

    if (fetch_time)
    {
        bool is_null = 0;
        stop->arrival_time = (Interval *)SPI_getbinval(*tuple, *tupdesc, 3, &is_null);
        stop->departure_time = (Interval *)SPI_getbinval(*tuple, *tupdesc, 4, &is_null);
    }
}

void fetch_trip(HeapTuple *tuple, TupleDesc *tupdesc, Trip_t *trip)
{
    bool is_null = 0;
    trip->trip_id = SPI_getvalue(*tuple, *tupdesc, 1);
    trip->departure_time = (Interval *)SPI_getbinval(*tuple, *tupdesc, 2, &is_null);
}

void fetch_route(HeapTuple *tuple, TupleDesc *tupdesc, Route_t *route)
{
    route->route_id = SPI_getvalue(*tuple, *tupdesc, 1);
    route->route_name = SPI_getvalue(*tuple, *tupdesc, 2);
}


void get_earliest_trip(char *stop_id, char *route_id, Interval *departure_time, char **trip_id)
{
    Trip_t **potential_trips = NULL;

    char query[QUERY_SIZE] = {0};
    
    char base_query[] = "select t.trip_id, st.departure_time "
                        "from tan.stop_times st "
                        "join tan.trips t "
                        "on st.trip_id = t.trip_id "
                        "join tan.routes r "
                        "on r.route_id = t.route_id "
                        "where st.stop_id = '%s' "
                        "and r.route_id = '%s' ";

    SPIPlanPtr SPIplan;
    Portal SPIportal;
                        
    int moredata = 1, i = 0;
    int tuple_limit = 1000;
    size_t total_tuples = 0;

    sprintf(query, base_query, stop_id, route_id);
    SPI_connect();

    SPIplan = SPI_prepare(query, 0, NULL);

    SPIportal = SPI_cursor_open(NULL, SPIplan, NULL, NULL, true);

    while (moredata)
    {
        size_t ntuples;
        size_t t;
        SPITupleTable *tuptable;
        TupleDesc tupdesc;

        SPI_cursor_fetch(SPIportal, true, tuple_limit);
        ntuples = SPI_processed;
        total_tuples += ntuples;

        if (ntuples > 0) {

            if ((*potential_trips) == NULL)
            {
                (*potential_trips) = (Trip_t *)
                    SPI_palloc(total_tuples * sizeof(Trip_t));
            }
            else
            {
                (*potential_trips) = (Trip_t *)
                    SPI_repalloc((*potential_trips), total_tuples * sizeof(Trip_t));
            }

            if ((*potential_trips) == NULL) {
                elog(ERROR, "Out of memory");
            }

            tuptable = SPI_tuptable;
            tupdesc = SPI_tuptable->tupdesc;
            
            for (t = 0; t < ntuples; t++) {
                HeapTuple tuple = tuptable->vals[t];
                fetch_trip(&tuple, &tupdesc, &(*potential_trips)[total_tuples - ntuples + t]);
            }
            SPI_freetuptable(tuptable);
        } else {
            moredata = 0;
        }
    }

    departure_time = (*potential_trips)[0].departure_time;

    for (i = 1; i < total_tuples; i++)
    {
        if(intervalCmp((*potential_trips)[i].departure_time, departure_time) <= 0)
        {
            departure_time = (*potential_trips)[i].departure_time;
            *trip_id = (*potential_trips)[i].trip_id;
        }
    }

    SPI_cursor_close(SPIportal);

    SPI_finish();
 
}

void get_from_stops(char *stop_id, Stop_t **stops, size_t *total_from_stops)
{
    char query[QUERY_SIZE] = {0};
    
    char base_query[] = "select stop_id, stop_name "
                        "from tan.stops s "
                        "where parent_station = '%s'";

    SPIPlanPtr SPIplan;
    Portal SPIportal;

    int moredata = 1;
    int tuple_limit = 1000;
    size_t total_tuples = 0;
    *total_from_stops = 0;

    sprintf(query, base_query, stop_id);
    SPI_connect();
    
    SPIplan = SPI_prepare(query, 0, NULL);

    SPIportal = SPI_cursor_open(NULL, SPIplan, NULL, NULL, true);

    while (moredata)
    {
        size_t ntuples;
        size_t t;
        SPITupleTable *tuptable;
        TupleDesc tupdesc;

        SPI_cursor_fetch(SPIportal, true, tuple_limit);
        ntuples = SPI_processed;
        total_tuples += ntuples;

        if (ntuples > 0) {

            if ((*stops) == NULL)
            {
                (*stops) = (Stop_t *)
                    SPI_palloc(total_tuples * sizeof(Stop_t));
            }
            else
            {
                (*stops) = (Stop_t *)
                    SPI_repalloc((*stops), total_tuples * sizeof(Stop_t));
            }

            if ((*stops) == NULL) {
                elog(ERROR, "Out of memory");
            }

            tuptable = SPI_tuptable;
            tupdesc = SPI_tuptable->tupdesc;
            
            for (t = 0; t < ntuples; t++) {
                HeapTuple tuple = tuptable->vals[t];
                fetch_stop(&tuple, &tupdesc, &(*stops)[total_tuples - ntuples + t], 0);
            }
            SPI_freetuptable(tuptable);
        } else {
            moredata = 0;
        }
    }

    *total_from_stops = total_tuples;
    SPI_cursor_close(SPIportal);

    SPI_finish();
}   


void get_routes_traversing_stop_id(char *stop_id, Route_t **routes, size_t *total_routes)
{

    char query[QUERY_SIZE] = {0};
    
    char base_query[] = "select r.route_id, r.route_short_name "
                        "from tan.routes_stops rs "
                        "join tan.routes r "
                        "on r.route_id = rs.route_id "
                        "where rs.stop_id = '%s'";

    SPIPlanPtr SPIplan;
    Portal SPIportal;

    int moredata = 1;
    int tuple_limit = 1000;
    size_t total_tuples = 0;
    *total_routes = 0;

    sprintf(query, base_query, stop_id);
    SPI_connect();
    
    SPIplan = SPI_prepare(query, 0, NULL);

    SPIportal = SPI_cursor_open(NULL, SPIplan, NULL, NULL, true);

    while (moredata)
    {
        size_t ntuples;
        size_t t;
        SPITupleTable *tuptable;
        TupleDesc tupdesc;

        SPI_cursor_fetch(SPIportal, true, tuple_limit);
        ntuples = SPI_processed;
        total_tuples += ntuples;

        if (ntuples > 0) {

            if ((*routes) == NULL)
            {
                (*routes) = (Route_t *)
                    SPI_palloc(total_tuples * sizeof(Route_t));
            }
            else
            {
                (*routes) = (Route_t *)
                    SPI_repalloc((*routes), total_tuples * sizeof(Route_t));
            }

            if ((*routes) == NULL) {
                elog(ERROR, "Out of memory");
            }

            tuptable = SPI_tuptable;
            tupdesc = SPI_tuptable->tupdesc;
            
            for (t = 0; t < ntuples; t++) {
                HeapTuple tuple = tuptable->vals[t];
                fetch_route(&tuple, &tupdesc, &(*routes)[total_tuples - ntuples + t]);
            }
            SPI_freetuptable(tuptable);
        } else {
            moredata = 0;
        }
    }
    *total_routes = total_tuples;
    SPI_cursor_close(SPIportal);
    SPI_finish();
}

void get_stops_on_route_after_stop_id(char *route_id, char *stop_id, Stop_t **stops, size_t *total_stops)
{
    char query[QUERY_SIZE] = {0};

    char base_query[] = "with max_stop_sequence  as ("
                            "select route_id, direction_id, max_stop_sequence "
                            "from tan.routes_stops rs "
                            "where stop_id = '%s' "
                            "and route_id = '%s' "
                        ")"
                        "select rs.stop_id, s.stop_name "
                        "from tan.routes_stops rs "
                        "join max_stop_sequence mss "
                        "on rs.route_id = mss.route_id "
                        "join tan.stops s "
                        "on rs.stop_id  = s.stop_id "
                        "and rs.direction_id = mss.direction_id "
                        "where rs.max_stop_sequence > mss.max_stop_sequence "
                        "order by rs.max_stop_sequence";


    SPIPlanPtr SPIplan;
    Portal SPIportal;

    int moredata = 1;
    int tuple_limit = 1000;
    size_t total_tuples = 0;

    sprintf(query, base_query, stop_id, route_id);

    SPI_connect();
    
    SPIplan = SPI_prepare(query, 0, NULL);

    SPIportal = SPI_cursor_open(NULL, SPIplan, NULL, NULL, true);

    while (moredata)
    {
        size_t ntuples;
        size_t t;
        SPITupleTable *tuptable;
        TupleDesc tupdesc;

        SPI_cursor_fetch(SPIportal, true, tuple_limit);
        ntuples = SPI_processed;
        total_tuples += ntuples;

        if (ntuples > 0) {
            if ((*stops) == NULL)
                (*stops) = (Stop_t *)
                    SPI_palloc(total_tuples * sizeof(Stop_t));
            else
                (*stops) = (Stop_t *)
                    SPI_repalloc((*stops), total_tuples * sizeof(Stop_t));

            if ((*stops) == NULL) {
                elog(ERROR, "Out of memory");
            }

            tuptable = SPI_tuptable;
            tupdesc = SPI_tuptable->tupdesc;

            for (t = 0; t < ntuples; t++) {
                HeapTuple tuple = tuptable->vals[t];
                fetch_stop(&tuple, &tupdesc, &(*stops)[total_tuples - ntuples + t], 1);
            }
            SPI_freetuptable(tuptable);
        } else {
            moredata = 0;
        }
    }
    *total_stops = total_tuples;
    SPI_cursor_close(SPIportal);
    SPI_finish();

}

void get_departure_time(Trip_t *trip, char *stop_id, Interval **departure_time)
{
    char query[QUERY_SIZE] = {0};

    char base_query[] = "select departure_time "
                        "from tan.trips t "
                        "join tan.stop_times st "
                        "on t.trip_id = st.trip_id "
                        "where t.trip_id = '%s' "
                        "and st.stop_id = '%s'";
    
    SPIPlanPtr SPIplan;
    Portal SPIportal;

    int moredata = 1;
    int tuple_limit = 1000;
    size_t total_tuples = 0;

    sprintf(query, base_query, trip->trip_id, stop_id);
    SPI_connect();
    
    SPIplan = SPI_prepare(query, 0, NULL);

    SPIportal = SPI_cursor_open(NULL, SPIplan, NULL, NULL, true);

    while (moredata)
    {
        size_t ntuples;
        size_t t;
        SPITupleTable *tuptable;
        TupleDesc tupdesc;

        SPI_cursor_fetch(SPIportal, true, tuple_limit);
        ntuples = SPI_processed;
        total_tuples += ntuples;

        if (ntuples > 0) {
            if ((*departure_time) == NULL)
                (*departure_time) = (Interval *)
                    SPI_palloc(total_tuples * sizeof(Interval));
            else
                (*departure_time) = (Interval *)
                    SPI_repalloc((*departure_time), total_tuples * sizeof(Interval));

            if ((*departure_time) == NULL) {
                elog(ERROR, "Out of memory");
            }

            tuptable = SPI_tuptable;
            tupdesc = SPI_tuptable->tupdesc;

            for (t = 0; t < ntuples; t++) {
                HeapTuple tuple = tuptable->vals[t];
                // fetch_stop(&tuple, &tupdesc, &(*stops)[total_tuples - ntuples + t], 0);
                fetch_time(&tuple, &tupdesc, departure_time);
            }
            SPI_freetuptable(tuptable);
        } else {
            moredata = 0;
        }
    }

    SPI_cursor_close(SPIportal);
}

void get_arrival_time(Trip_t *trip, char *stop_id, Interval **arrival_time)
{
    char query[QUERY_SIZE] = {0};

    char base_query[] = "select arrival_time "
                        "from tan.trips t "
                        "join tan.stop_times st "
                        "on t.trip_id = st.trip_id "
                        "where t.trip_id = '%s' "
                        "and st.stop_id = '%s'";
    
    SPIPlanPtr SPIplan;
    Portal SPIportal;

    int moredata = 1;
    int tuple_limit = 1000;
    size_t total_tuples = 0;

    sprintf(query, base_query, trip->trip_id, stop_id);
    SPI_connect();
    
    SPIplan = SPI_prepare(query, 0, NULL);

    SPIportal = SPI_cursor_open(NULL, SPIplan, NULL, NULL, true);

    while (moredata)
    {
        size_t ntuples;
        size_t t;
        SPITupleTable *tuptable;
        TupleDesc tupdesc;

        SPI_cursor_fetch(SPIportal, true, tuple_limit);
        ntuples = SPI_processed;
        total_tuples += ntuples;

        if (ntuples > 0) {
            if ((*arrival_time) == NULL)
                (*arrival_time) = (Interval *)
                    SPI_palloc(total_tuples * sizeof(Interval));
            else
                (*arrival_time) = (Interval *)
                    SPI_repalloc((*arrival_time), total_tuples * sizeof(Interval));

            if ((*arrival_time) == NULL) {
                elog(ERROR, "Out of memory");
            }

            tuptable = SPI_tuptable;
            tupdesc = SPI_tuptable->tupdesc;

            for (t = 0; t < ntuples; t++) {
                HeapTuple tuple = tuptable->vals[t];
                // fetch_stop(&tuple, &tupdesc, &(*stops)[total_tuples - ntuples + t], 0);
                fetch_time(&tuple, &tupdesc, arrival_time);
            }
            SPI_freetuptable(tuptable);
        } else {
            moredata = 0;
        }
    }

    SPI_cursor_close(SPIportal);
}


void get_transfer_from_stop_id(char *stop_id, Stop_t **stops)
{

    char query[QUERY_SIZE] = {0};
    
    char base_query[] = "with station as ("
	                "select parent_station"
	                "from tan.stops s" 
	                "where s.stop_id = '%s'"
                    ")"
                    "select stop_id, stop_name"
                    "from tan.stops s"
                    "join station" 
                    "on station.parent_station = s.parent_station";

    SPIPlanPtr SPIplan;
    Portal SPIportal;

    int moredata = 1;
    int tuple_limit = 1000;
    size_t total_tuples = 0;

    sprintf(query, base_query, stop_id);
    SPI_connect();
    
    SPIplan = SPI_prepare(query, 0, NULL);

    SPIportal = SPI_cursor_open(NULL, SPIplan, NULL, NULL, true);

    while (moredata)
    {
        size_t ntuples;
        size_t t;
        SPITupleTable *tuptable;
        TupleDesc tupdesc;

        SPI_cursor_fetch(SPIportal, true, tuple_limit);
        ntuples = SPI_processed;
        total_tuples += ntuples;

        if (ntuples > 0) {
            if ((*stops) == NULL)
                (*stops) = (Stop_t *)
                    SPI_palloc(total_tuples * sizeof(Stop_t));
            else
                (*stops) = (Stop_t *)
                    SPI_repalloc((*stops), total_tuples * sizeof(Stop_t));

            if ((*stops) == NULL) {
                elog(ERROR, "Out of memory");
            }

            tuptable = SPI_tuptable;
            tupdesc = SPI_tuptable->tupdesc;

            for (t = 0; t < ntuples; t++) {
                HeapTuple tuple = tuptable->vals[t];
                fetch_stop(&tuple, &tupdesc, &(*stops)[total_tuples - ntuples + t], 0);
            }
            SPI_freetuptable(tuptable);
        } else {
            moredata = 0;
        }
    }

    SPI_cursor_close(SPIportal);
}