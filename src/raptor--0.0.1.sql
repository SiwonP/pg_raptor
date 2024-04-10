-- All function definitions


-- internal functions
CREATE FUNCTION raptor_run(
    TEXT,
    TEXT,
    out stop_id text,
    out route_id text  )
RETURNS SETOF RECORD
as '/usr/lib/postgresql/14/lib/raptor.so', 'raptor_run'
LANGUAGE C VOLATILE STRICT;

-- CREATE FUNCTION raptor_init(
--     TEXT,
--     OUT stop_id text,
--     OUT stop_name text
-- )
-- RETURNS SETOF RECORD
-- as '/usr/lib/postgresql/14/lib/raptor.so', 'raptor_init'
-- LANGUAGE C VOLATILE STRICT;

CREATE FUNCTION raptor_init(
    TEXT,
    out stop_id text,
    OUT departure_time INTERVAL
)
RETURNS SETOF RECORD
as '/usr/lib/postgresql/14/lib/raptor.so', 'raptor_init'
LANGUAGE C VOLATILE STRICT;

CREATE FUNCTION raptor_get_time(
    TEXT,
    TEXT
)
RETURNS INTERVAL
as '/usr/lib/postgresql/14/lib/raptor.so', 'raptor_get_time'
LANGUAGE C VOLATILE STRICT;


-- external functions
