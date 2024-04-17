-- All function definitions


-- internal functions

create or replace function pg_raptor_get_routes_traversing_stop(
    text 
)
RETURNS table(route_id varchar, route_short_name varchar)
as 'pg_raptor', 'pg_raptor_get_routes_traversing_stop'
LANGUAGE C STRICT;

create or replace function pg_raptor_get_earliest_trips(
    text,
    text, 
    text,
    text
)
RETURNS table(trip_id varchar, stop_id varchar, stop_sequence int, departure_time interval)
as 'pg_raptor', 'pg_raptor_get_earliest_trips'
LANGUAGE C STRICT;


create or replace function pg_raptor_get_interval_from_string(
    text
)
RETURNS VOID
as 'pg_raptor', 'pg_raptor_get_interval_from_string'
LANGUAGE C STRICT;

-- external functions


create or replace function pg_raptor_run(
    text,
    text
)
RETURNS table(stop_id varchar, route_id varchar) 
as 'pg_raptor', 'pg_raptor_run'
LANGUAGE C STRICT;