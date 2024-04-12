-- All function definitions


-- internal functions
-- CREATE FUNCTION pg_raptor_run(
--     TEXT,
--     TEXT,
--     out stop_id text,
--     out route_id text  )
-- RETURNS SETOF RECORD
-- as '/usr/lib/postgresql/14/lib/raptor.so', 'raptor_run'
-- LANGUAGE C VOLATILE STRICT;


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
RETURNS table(trip_id varchar, stop_id varchar)
as 'pg_raptor', 'pg_raptor_get_earliest_trips'
LANGUAGE C STRICT;


-- external functions
