#ifndef RAPTOR_STRUCTURE_H
#define RAPTOR_STRUCTURE_H

#include <stdint.h>
#include <postgres.h>
#include <datatype/timestamp.h>

/**
 * @brief
*/

typedef struct Stop 
{
    char        *stop_id;
    char        *stop_name;
    Interval    *arrival_time;
    Interval    *departure_time;
} Stop_t;

typedef struct Route
{
    char    *route_id;
    char    *route_name;
} Route_t;

typedef struct Trip
{
    char        *trip_id;
    char        *stop_id;
    Interval    *departure_time;
    int         stop_sequence;
} Trip_t;


typedef struct Queue_node
{
    char    *stop_id;
    char    *route_id;
} Queue_node_t;


typedef struct Label
{
    char            *stop_id;
    Interval        *departure_time;
    struct Label    *next;

} Label_t;

#endif