#ifndef RAPTOR_STEPS_H
#define RAPTOR_STEPS_H

#include <stdio.h>

#include "raptor_structure.h"
#include "raptor_fetch.h"
#include "raptor_utils.h"

// int raptor_accumulate_routes(Marked_stop_t *marked_stops, Queue_node_t *queue);

int raptor_accumulate_routes(Stop_t *marked_stops, size_t size_marked_stops, Queue_node_t **queue, size_t *size_queue);

int raptor_traverse_routes(int round, Queue_node_t *queue, size_t size_queue, Label_t **labels_round, Label_t **labels_star);

int raptor_add_transfers(Stop_t *marked_stops);

#endif