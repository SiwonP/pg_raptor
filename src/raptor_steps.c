#include "raptor_steps.h"


int raptor_accumulate_routes(Stop_t *marked_stops, size_t size_marked_stops, Queue_node_t **queue, size_t *size_queue)
{
    size_t i, total_routes = 0;
    Route_t *routes = NULL;

    elog(INFO, "accumulating routes");
    elog(INFO, "size marked stops %ld", size_marked_stops);

    for (i = 0; i < size_marked_stops; i++)
    {
        get_routes_traversing_stop_id(marked_stops[i].stop_id, &routes, &total_routes);
        elog(INFO, "found %ld routes", total_routes);

        for (int j = 0; j < total_routes; j++)
        {
            elog(INFO, "route %d : %s %s", j, routes[j].route_id, routes[j].route_name);
        }
        add_to_queue(queue, size_queue, marked_stops[i].stop_id, &routes, total_routes);
    }
    elog(INFO, "[raptor_accumulate_routes] size queue %ld", *size_queue);

    return 1;
}

int raptor_traverse_routes(int round, Queue_node_t *queue, size_t size_queue, Label_t **labels_round, Label_t **labels_star)
{
    // init variables
    Trip_t *current_trip = NULL;
    size_t i, j;
    char *stop_id, *route_id;
    Stop_t *stops = NULL;
    size_t total_stops = 0;
    Interval *interval = NULL;

    // for each (r,p) in Q 
    for (i = 0; i < size_queue; i++)
    {
        // set the current trip to NULL
        current_trip = NULL;
        interval = NULL;

        // retrieve stop_id and route_id 
        stop_id = queue[i].stop_id;
        route_id = queue[i].route_id;

        // gather all stops on route route_id coming after stop stop_id 
        get_stops_on_route_after_stop_id(route_id, stop_id, &stops, &total_stops);
        
        // for each stop j of r after stop_id
        for(j = 0; j < total_stops; j++)
        {
            Stop_t *current_stop = NULL;

            elog(WARNING, "stop %s", stops[j].stop_name);

            // if current_trip 
            if (current_trip != NULL)
            {   
                // get arrival time at stop j during current_trip
                get_arrival_time(current_trip, stops[j].stop_id, &interval);
                
                // if (interval->time < (*labels)[])
                // {

                // }

                SPI_pfree(interval);
                SPI_finish();

                interval = NULL;

            }

            // retrieving label of stop j 
            // current_stop = (Stop_t *)get_item(*labels, stops[j].stop_id);


            // if we can catch an earlier trip at stop j
            if (!is_labelled(labels_round[round-1], stops[j].stop_id))
            {
                Interval *label_interval = NULL;
                // get departure time at stop j during current_trip
                get_departure_time(current_trip, stops[j].stop_id, &interval);
                label_interval = get_item(labels_round  [round-1], stops[j].stop_id);
                if (label_interval->time <= interval->time)
                {
                    // update current_trip
                    // TODO
                } 
            }
        }

    }
    return 1;
}

int raptor_add_transfers(Stop_t *marked_stops)
{

    return 1;
}

int raptor_traverse_route(char *route_id, char *stop_id, char *departure_date, char *departure_time,  Label_t **labels_round, Label_t **labels_star)
{
    char query[QUERY_SIZE] = {0};
    Stop_t **stops;
    size_t *total_stops;

    char base_query[]= "select st2.*"
                "from stop_times st2 "
                "join ("
                "select distinct on (t.direction_id)"
                "st.stop_id,"
                "t.trip_id,"
                "st.departure_time,"
                "st.stop_sequence"
                "from trips t "
                "join stop_times st on st.trip_id = t.trip_id "
                "join routes r on r.route_id = t.route_id "
                "join calendar_dates cd on cd.service_id = t.service_id "
                "where r.route_id = '%s' "
                "and st.stop_id = '%s' "
                "and cd.\"date\" = '%s' "
                "and st.departure_time >= '%s' "
                "order by t.direction_id, st.departure_time "
                ") as pt "
                "on st2.trip_id = pt.trip_id "
                "and st2.stop_sequence > pt.stop_sequence "
                "order by st2.trip_id, st2.stop_sequence";

    SPIPlanPtr SPIplan;
    Portal SPIportal;

    int moredata = 1;
    int tuple_limit = 1000;
    size_t total_tuples = 0;

    sprintf(query, base_query, route_id, stop_id, departure_date, departure_time);

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

        if (ntuples > 0)
        {
            if ((*stops) == NULL)
                (*stops) = (Stop_t *)
                    SPI_palloc(total_tuples * sizeof(Stop_t));
            else
                (*stops) = (Stop_t *)
                    SPI_repalloc((*stops), total_tuples * sizeof(Stop_t));

            if ((*stops) == NULL)
            {
                elog(ERROR, "Out of memory");
            }

            tuptable = SPI_tuptable;
            tupdesc = SPI_tuptable->tupdesc;

            for (t = 0; t < ntuples; t++)
            {
                HeapTuple tuple = tuptable->vals[t];
                fetch_stop(&tuple, &tupdesc, &(*stops)[total_tuples - ntuples + t], 0);
            }
            SPI_freetuptable(tuptable);
        }
        else
        {
            moredata = 0;
        }
    }
    *total_stops = total_tuples;
    SPI_cursor_close(SPIportal);
    SPI_finish();
}