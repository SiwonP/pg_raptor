#include "raptor_steps.h"


int raptor_accumulate_routes(Stop_t *marked_stops, size_t size_marked_stops, Queue_node_t **queue, size_t *size_queue)
{
    size_t i, total_routes = 0;
    Route_t *routes = NULL;
    for (i = 0; i < size_marked_stops; i++)
    {
        get_routes_traversing_stop_id(marked_stops[i].stop_id, &routes, &total_routes);
        add_to_queue(queue, size_queue, marked_stops[i].stop_id, &routes, total_routes);
    }

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
            // if ()
            // {
                Interval *label_interval = NULL;
                // get departure time at stop j during current_trip
                get_departure_time(current_trip, stops[j].stop_id, &interval);
                label_interval = get_item(labels_round  [round-1], stops[j].stop_id);
                if (label_interval->time <= interval->time)
                {
                    // update current_trip
                    // TODO
                } 
            // }
        }

    }
    return 1;
}

int raptor_add_transfers(Stop_t *marked_stops)
{

    return 1;
}