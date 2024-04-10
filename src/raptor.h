#ifndef RAPTOR_H
#define RAPTOR_H

#include "postgres.h"
#include "fmgr.h"

#include "executor/spi.h"
#include "utils/builtins.h"
#include "funcapi.h"

#include "raptor_structure.h"
#include "raptor_fetch.h"
#include "raptor_steps.h"

#define MAX_ROUND 4

void init_labels(Label_t **labels, Stop_t *from_stops, size_t total_from_stops);

void rator_update_labels(Label_t **labels);

void raptor_process_round(
    int round, Stop_t *marked_stops, size_t size_marked_stops, Queue_node_t **queue, 
    size_t *size_queue, Label_t **labels_round, Label_t **labels_star);

Datum raptor_get_time(PG_FUNCTION_ARGS);

Datum raptor_init(PG_FUNCTION_ARGS);

Datum raptor_run(PG_FUNCTION_ARGS);


#endif