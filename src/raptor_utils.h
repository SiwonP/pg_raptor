#ifndef RAPTOR_UTILS
#define RAPTOR_UTILS

#include "raptor_structure.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "postgres.h"
#include "fmgr.h"

#include "executor/spi.h"
#include "utils/builtins.h"
#include "funcapi.h"

/**
 * @brief
 * 
 * @param   
 * 
 * @return
*/
void fill_queue_node(Queue_node_t *node, char *stop_id, Route_t *route);

void add_to_queue(Queue_node_t **queue, size_t *size_queue, char *stop_id, Route_t **routes, size_t total_routes);

void clear_queue(Queue_node_t *queue, size_t size_queue);

Interval *get_item(Label_t *labels, char *key);

void delete_item(Label_t **labels, char *key);

void add_item(Label_t **labels, char *key, Interval *value);

char is_labelled(Label_t *labels, char *stop_id);

#endif