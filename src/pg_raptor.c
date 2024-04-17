
#include "pg_raptor.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(raptor_run);

PG_FUNCTION_INFO_V1(raptor_init);

PG_FUNCTION_INFO_V1(raptor_get_time);


void init_labels(Label_t **labels, Stop_t *from_stops, size_t total_from_stops)
{
    size_t i = 0;
    Interval null_interval;
    null_interval.time = PG_INT64_MAX;
    null_interval.day = 0;
    null_interval.month = 0;

    for (i = 0; i < total_from_stops; i++)
    {
        add_item(labels, from_stops[i].stop_id, &null_interval);
    }
}

void rator_update_labels(Label_t **labels)
{
}


Datum raptor_get_time(PG_FUNCTION_ARGS)
{
    char *stop_id = NULL;
    char *trip_id = NULL;
    SPITupleTable *tuptable;
    HeapTuple tuple;
    Interval *interval = NULL;
    Interval *return_interval;

    TupleDesc tupdesc;
    interval = malloc(sizeof(Interval));
    // int64 _time;

    return_interval = malloc(sizeof(Interval));
    return_interval->day = 0;
    return_interval->month = 0;

    char base_query[] = "select departure_time "
                        "from stop_times st "
                        "where stop_id = '%s' "
                        "and trip_id = '%s'";

    char query[QUERY_SIZE] = {0};

    stop_id = text_to_cstring(PG_GETARG_TEXT_PP(0));
    trip_id = text_to_cstring(PG_GETARG_TEXT_PP(1));

    sprintf(query, base_query, stop_id, trip_id);

    SPI_connect();

    SPI_exec(query, 1);

    tuptable = SPI_tuptable;
    tupdesc = SPI_tuptable->tupdesc;

    tuple = tuptable->vals[0];

    fetch_time(&tuple, &tupdesc, &interval);

    elog(WARNING, "time %ld", interval->time); // microseconds !!!!!!
    elog(WARNING, "time %d", interval->day);
    elog(WARNING, "time %d", interval->month);

    SPI_finish();
    // _time = interval->time;
    // free(interval);

    return_interval->time = 120000000;

    PG_RETURN_DATUM((Datum)(return_interval));
}

Datum raptor_init(PG_FUNCTION_ARGS)
{
    char *stop_id = NULL;
    Label_t *labels = NULL;
    Stop_t *from_stops = NULL;
    size_t total_from_stops = 0;

    FuncCallContext *funcctx;
    int call_cntr;
    int max_calls;
    TupleDesc tupdesc;
    AttInMetadata *attinmeta;

    stop_id = text_to_cstring(PG_GETARG_TEXT_PP(0));

    if (SRF_IS_FIRSTCALL())
    {
        MemoryContext oldcontext;

        funcctx = SRF_FIRSTCALL_INIT();
        oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
        /* One-time setup code appears here: */

        // get all stops from parent station stop_id
        get_from_stops(stop_id, &from_stops, &total_from_stops);
        init_labels(&labels, from_stops, total_from_stops);

        funcctx->max_calls = total_from_stops;
        // funcctx->max_calls = total_routes;

        if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)
            ereport(ERROR,
                    (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("function returning record called in context "
                            "that cannot accept type record")));

        attinmeta = TupleDescGetAttInMetadata(tupdesc);
        funcctx->attinmeta = attinmeta;
        // funcctx->user_fctx = routes;
        funcctx->user_fctx = labels;

        funcctx->tuple_desc = tupdesc;
        MemoryContextSwitchTo(oldcontext);
    }

    /* Each-time setup code appears here: */
    funcctx = SRF_PERCALL_SETUP();
    call_cntr = funcctx->call_cntr;
    max_calls = funcctx->max_calls;
    attinmeta = funcctx->attinmeta;
    tupdesc = funcctx->tuple_desc;
    labels = (Label_t *)funcctx->user_fctx;
    // from_stops = (Stop_t *)funcctx->user_fctx;

    /* this is just one way we might test whether we are done: */
    if (call_cntr < max_calls)
    {
        /* Here we want to return another item: */
        Datum *values;
        HeapTuple tuple;
        Datum result;
        bool *nulls;
        char stop[] = "COME2";
        int numb = 2;

        values = (Datum *)palloc(numb * sizeof(Datum));
        nulls = palloc(numb * sizeof(bool));
        nulls[0] = false;
        nulls[1] = false;

        values[0] = CStringGetDatum(stop);
        values[1] = (Datum)get_item(labels, stop);

        /* build a tuple */
        // tuple = BuildTupleFromCStrings(attinmeta, values);
        tuple = heap_form_tuple(tupdesc, values, nulls);
        /* make the tuple into a datum */
        result = HeapTupleGetDatum(tuple);

        /* clean up (this is not really necessary) */
        // pfree(values[0]);
        // pfree(values[1]);
        pfree(values);
        pfree(nulls);

        SRF_RETURN_NEXT(funcctx, result);
    }
    else
    {
        /* Here we are done returning items, so just report that fact. */
        /* (Resist the temptation to put cleanup code here.) */
        SRF_RETURN_DONE(funcctx);
    }
}

void raptor_process_round(
    int round, Stop_t *marked_stops, size_t size_marked_stops, Queue_node_t **queue,
    size_t *size_queue, Label_t **labels_round, Label_t **labels_star)
{
    *size_queue = 0;

    elog(INFO, "processing round");
    elog(INFO, "size marked stops %ld", size_marked_stops);

    if (*queue != NULL)
    {
        clear_queue(*queue, *size_queue);
        free(*queue);
        *queue = NULL;
    }
    elog(INFO, "size queue %ld", size_queue);

    raptor_accumulate_routes(marked_stops, size_marked_stops, queue, size_queue);

    elog(INFO, "size queue %ld", *size_queue);
    elog(INFO, "size marked stops %ld", size_marked_stops);

    for (int i = 0; i < *size_queue; i++)
    {
        elog(INFO, "queue %ld %s %s", i, (*queue)[i].route_id, (*queue)[i].stop_id);
    }

    elog(INFO, "traversing route");
    raptor_traverse_routes(round, *queue, *size_queue, labels_round, labels_star);
}

static void
raptor_process(Stop_t *marked_stops, size_t size_marked_stops)
{
    int round = 0;
    Queue_node_t *queue = NULL;
    size_t size_queue = 0;
    Label_t *labels_round = NULL, *labels_star = NULL;

    for (round = 1; round < MAX_ROUND || size_marked_stops == 0; round++)
    {
        elog(INFO, "round number %d", round);
        raptor_process_round(round, marked_stops, size_marked_stops, &queue, &size_queue, &labels_round, &labels_star);
    }
}

PG_FUNCTION_INFO_V1(pg_raptor_run);

Datum pg_raptor_run(PG_FUNCTION_ARGS)
{

    char *from_stop_id = NULL, *target_stop_id = NULL;
    Route_t *routes = NULL;
    // size_t total_routes = 0;
    size_t size_marked_stops = 0;
    size_t size_queue = 0;

    Queue_node_t *queue = NULL;
    Stop_t marked_stops[128];
    Stop_t from_stop;
    Stop_t target_stop;
    Label_t *Labels;

    FuncCallContext *funcctx;
    int call_cntr;
    int max_calls;
    TupleDesc tupdesc;
    AttInMetadata *attinmeta;

    memset(marked_stops, 0, 128 * sizeof(Stop_t));

    // retrieving the id of from_stop and target_stop
    from_stop_id = text_to_cstring(PG_GETARG_TEXT_PP(0));
    target_stop_id = text_to_cstring(PG_GETARG_TEXT_PP(1));

    from_stop.stop_id = from_stop_id;
    target_stop.stop_id = target_stop_id;

    elog(INFO, "from_stop_id %s", from_stop.stop_id);
    elog(INFO, "to_stop_id %s",target_stop.stop_id );

    // marking from_stop
    marked_stops[0] = from_stop;
    size_marked_stops = 1;
    size_t total_routes;

    if (SRF_IS_FIRSTCALL())
    {
        MemoryContext oldcontext;

        funcctx = SRF_FIRSTCALL_INIT();
        oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
        /* One-time setup code appears here: */

        raptor_process(marked_stops, size_marked_stops);

        funcctx->max_calls = size_queue;
        // funcctx->max_calls = total_routes;

        if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)
            ereport(ERROR,
                    (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("function returning record called in context "
                            "that cannot accept type record")));

        attinmeta = TupleDescGetAttInMetadata(tupdesc);
        funcctx->attinmeta = attinmeta;
        // funcctx->user_fctx = routes;
        funcctx->user_fctx = queue;

        MemoryContextSwitchTo(oldcontext);
    }

    /* Each-time setup code appears here: */
    funcctx = SRF_PERCALL_SETUP();
    call_cntr = funcctx->call_cntr;
    max_calls = funcctx->max_calls;
    attinmeta = funcctx->attinmeta;
    // routes = (Route_t *)funcctx->user_fctx;
    queue = (Queue_node_t *)funcctx->user_fctx;

    /* this is just one way we might test whether we are done: */
    if (call_cntr < max_calls)
    {
        /* Here we want to return another item: */
        char **values;
        HeapTuple tuple;
        Datum result;

        values = (char **)palloc(2 * sizeof(char *));
        values[0] = (char *)palloc(16 * sizeof(char));
        values[1] = (char *)palloc(16 * sizeof(char));

        // strncpy(values[0], routes[call_cntr].route_id, 16);
        strncpy(values[0], queue[call_cntr].stop_id, 16);
        // strncpy(values[1], routes[call_cntr].route_id, 16);
        strncpy(values[1], queue[call_cntr].route_id, 16);

        /* build a tuple */
        tuple = BuildTupleFromCStrings(attinmeta, values);

        /* make the tuple into a datum */
        result = HeapTupleGetDatum(tuple);

        /* clean up (this is not really necessary) */
        pfree(values[0]);
        pfree(values[1]);
        pfree(values);

        SRF_RETURN_NEXT(funcctx, result);
    }
    else
    {
        /* Here we are done returning items, so just report that fact. */
        /* (Resist the temptation to put cleanup code here.) */
        SRF_RETURN_DONE(funcctx);
        clear_queue(queue, size_queue);
        free(queue);
    }
}

PG_FUNCTION_INFO_V1(pg_raptor_get_routes_traversing_stop);

Datum pg_raptor_get_routes_traversing_stop(PG_FUNCTION_ARGS)
{

    char *stop_id = NULL;
    Route_t *routes = NULL;
    size_t total_routes;

    FuncCallContext *funcctx;
    int call_cntr;
    int max_calls;
    TupleDesc tupdesc;
    AttInMetadata *attinmeta;

    total_routes = 0;
    // retrieving the id of from_stop and target_stop
    stop_id = text_to_cstring(PG_GETARG_TEXT_PP(0));

    // marking from_stop

    if (SRF_IS_FIRSTCALL())
    {
        MemoryContext oldcontext;

        funcctx = SRF_FIRSTCALL_INIT();
        oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
        /* One-time setup code appears here: */

        get_routes_traversing_stop_id(stop_id, &routes, &total_routes);

        funcctx->max_calls = total_routes;

        if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)
            ereport(ERROR,
                    (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("function returning record called in context "
                            "that cannot accept type record")));

        attinmeta = TupleDescGetAttInMetadata(tupdesc);
        funcctx->attinmeta = attinmeta;
        // funcctx->user_fctx = routes;
        funcctx->user_fctx = routes;

        MemoryContextSwitchTo(oldcontext);
    }

    /* Each-time setup code appears here: */
    funcctx = SRF_PERCALL_SETUP();
    call_cntr = funcctx->call_cntr;
    max_calls = funcctx->max_calls;
    attinmeta = funcctx->attinmeta;
    routes = (Route_t *)funcctx->user_fctx;

    /* this is just one way we might test whether we are done: */
    if (call_cntr < max_calls)
    {
        /* Here we want to return another item: */
        char **values;
        HeapTuple tuple;
        Datum result;
        size_t route_id_len, route_name_len;
        route_id_len = strlen(routes[call_cntr].route_id);
        route_name_len = strlen(routes[call_cntr].route_name);

        values = (char **)palloc(2 * sizeof(char *));
        values[0] = (char *)palloc(route_id_len * sizeof(char));
        values[1] = (char *)palloc(route_name_len * sizeof(char));

        strncpy(values[0], routes[call_cntr].route_id, route_id_len);
        strncpy(values[1], routes[call_cntr].route_name, route_name_len);

        /* build a tuple */
        tuple = BuildTupleFromCStrings(attinmeta, values);

        /* make the tuple into a datum */
        result = HeapTupleGetDatum(tuple);

        /* clean up (this is not really necessary) */
        pfree(values[0]);
        pfree(values[1]);
        pfree(values);

        SRF_RETURN_NEXT(funcctx, result);
    }
    else
    {
        /* Here we are done returning items, so just report that fact. */
        /* (Resist the temptation to put cleanup code here.) */
        SRF_RETURN_DONE(funcctx);
        // clear_queue(queue, size_queue);
        // free(queue);
    }
}


PG_FUNCTION_INFO_V1(pg_raptor_get_earliest_trips);

Datum pg_raptor_get_earliest_trips(PG_FUNCTION_ARGS)
{

    char *stop_id = NULL, *route_id = NULL, *departure_time = NULL, *departure_date = NULL;
    Trip_t *trips = NULL;
    size_t total_trips = 0;

    FuncCallContext *funcctx;
    int call_cntr;
    int max_calls;
    TupleDesc tupdesc;
    AttInMetadata *attinmeta;

    // retrieving the id of from_stop and target_stop
    stop_id = text_to_cstring(PG_GETARG_TEXT_PP(0));
    route_id = text_to_cstring(PG_GETARG_TEXT_PP(1));
    departure_time = text_to_cstring(PG_GETARG_TEXT_PP(2));
    departure_date = text_to_cstring(PG_GETARG_TEXT_PP(3));

    // marking from_stop

    if (SRF_IS_FIRSTCALL())
    {
        MemoryContext oldcontext;

        funcctx = SRF_FIRSTCALL_INIT();
        oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
        /* One-time setup code appears here: */
        get_earliest_trip(stop_id, route_id, departure_time, departure_date, &trips, &total_trips);
        // get_routes_traversing_stop_id(stop_id, &routes, &total_routes);

        funcctx->max_calls = total_trips;

        if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)
            ereport(ERROR,
                    (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("function returning record called in context "
                            "that cannot accept type record")));

        // attinmeta = TupleDescGetAttInMetadata(tupdesc);
        tupdesc = BlessTupleDesc(tupdesc);
        // funcctx->attinmeta = attinmeta;
        // funcctx->user_fctx = routes;
        funcctx->tuple_desc = tupdesc;
        funcctx->user_fctx = trips;

        MemoryContextSwitchTo(oldcontext);
    }

    /* Each-time setup code appears here: */
    funcctx = SRF_PERCALL_SETUP();
    call_cntr = funcctx->call_cntr;
    max_calls = funcctx->max_calls;
    attinmeta = funcctx->attinmeta;
    tupdesc = funcctx->tuple_desc;
    trips = (Trip_t *)funcctx->user_fctx;

    /* this is just one way we might test whether we are done: */
    if (call_cntr < max_calls)
    {
        /* Here we want to return another item: */
        Datum *values;
        HeapTuple tuple;
        size_t num_values;
        Datum result;
        bool *nulls;

        num_values = 4;
        values = palloc(num_values * sizeof(Datum));
        nulls = palloc(num_values * sizeof(bool));

        for(int i = 0 ; i < num_values; i++) 
        {
            nulls[i] = false;
        }

        values[0] = CStringGetDatum(trips[call_cntr].trip_id);
        values[1] = CStringGetDatum(trips[call_cntr].stop_id);
        values[2] = Int32GetDatum(trips[call_cntr].stop_sequence);
        values[3] = (Datum) trips[call_cntr].departure_time;

        /* build a tuple */
        // tuple = BuildTupleFromCStrings(attinmeta, values);
        tuple = heap_form_tuple(tupdesc, values, nulls);

        /* make the tuple into a datum */
        result = HeapTupleGetDatum(tuple);

        /* clean up (this is not really necessary) */
        // pfree(values);

        SRF_RETURN_NEXT(funcctx, result);
    }
    else
    {
        /* Here we are done returning items, so just report that fact. */
        /* (Resist the temptation to put cleanup code here.) */
        SRF_RETURN_DONE(funcctx);
        // clear_queue(queue, size_queue);
        // free(queue);
    }
}

PG_FUNCTION_INFO_V1(pg_raptor_accumulate_routes);

Datum pg_raptor_accumulate_routes(PG_FUNCTION_ARGS)
{

    Stop_t *marked_stops;
    size_t size_marked_stops;
    Queue_node_t *queue;
    size_t size_queue;

    char *stop_id = NULL, *route_id = NULL, *departure_time = NULL, *departure_date = NULL;
    Trip_t *trips = NULL;
    size_t total_trips = 0;

    FuncCallContext *funcctx;
    int call_cntr;
    int max_calls;
    TupleDesc tupdesc;
    AttInMetadata *attinmeta;

    // retrieving the id of from_stop and target_stop
    stop_id = text_to_cstring(PG_GETARG_TEXT_PP(0));
    route_id = text_to_cstring(PG_GETARG_TEXT_PP(1));
    departure_time = text_to_cstring(PG_GETARG_TEXT_PP(2));
    departure_date = text_to_cstring(PG_GETARG_TEXT_PP(3));

    if (SRF_IS_FIRSTCALL())
    {
        MemoryContext oldcontext;

        funcctx = SRF_FIRSTCALL_INIT();
        oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

        /* One-time setup code appears here: */
        raptor_accumulate_routes(marked_stops, size_marked_stops, &queue, &size_queue);

        funcctx->max_calls = size_queue;

        if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)
            ereport(ERROR,
                    (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("function returning record called in context "
                            "that cannot accept type record")));

        // attinmeta = TupleDescGetAttInMetadata(tupdesc);
        tupdesc = BlessTupleDesc(tupdesc);
        // funcctx->attinmeta = attinmeta;
        // funcctx->user_fctx = routes;
        funcctx->tuple_desc = tupdesc;
        funcctx->user_fctx = queue;

        MemoryContextSwitchTo(oldcontext);
    }

    /* Each-time setup code appears here: */
    funcctx = SRF_PERCALL_SETUP();
    call_cntr = funcctx->call_cntr;
    max_calls = funcctx->max_calls;
    attinmeta = funcctx->attinmeta;
    tupdesc = funcctx->tuple_desc;
    queue = (Queue_node_t *)funcctx->user_fctx;

    /* this is just one way we might test whether we are done: */
    if (call_cntr < max_calls)
    {
        /* Here we want to return another item: */
        Datum *values;
        HeapTuple tuple;
        size_t num_values;
        Datum result;
        bool *nulls;

        num_values = 4;
        values = palloc(num_values * sizeof(Datum));
        nulls = palloc(num_values * sizeof(bool));

        for(int i = 0 ; i < num_values; i++) 
        {
            nulls[i] = false;
        }

        values[0] = CStringGetDatum(trips[call_cntr].trip_id);
        values[1] = CStringGetDatum(trips[call_cntr].stop_id);
        values[2] = Int32GetDatum(trips[call_cntr].stop_sequence);
        values[3] = (Datum) trips[call_cntr].departure_time;

        elog(WARNING, "cstring : %s", trips[call_cntr].trip_id);
        elog(WARNING, "datum : %s", values[0]);

        /* build a tuple */
        // tuple = BuildTupleFromCStrings(attinmeta, values);
        tuple = heap_form_tuple(tupdesc, values, nulls);

        /* make the tuple into a datum */
        result = HeapTupleGetDatum(tuple);

        /* clean up (this is not really necessary) */
        // pfree(values);

        SRF_RETURN_NEXT(funcctx, result);
    }
    else
    {
        /* Here we are done returning items, so just report that fact. */
        /* (Resist the temptation to put cleanup code here.) */
        SRF_RETURN_DONE(funcctx);
        // clear_queue(queue, size_queue);
        // free(queue);
    }
}

PG_FUNCTION_INFO_V1(pg_raptor_get_interval_from_string);

Datum pg_raptor_get_interval_from_string(PG_FUNCTION_ARGS)
{
    char *string;
    Interval interval;

    string = text_to_cstring(PG_GETARG_TEXT_PP(0));

    get_interval_from_string(string, &interval);

    PG_RETURN_VOID();
}