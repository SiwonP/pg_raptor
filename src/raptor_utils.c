#include "raptor_utils.h"

void fill_queue_node(Queue_node_t *node, char *stop_id, Route_t *route)
{
    node->stop_id = (char *)malloc(strlen(stop_id));
    // node->stop_id = stop_id;
    strcpy(node->stop_id, stop_id);
    node->route_id = (char *)malloc(strlen(route->route_id));
    // node->route_id = route->route_id;
    strcpy(node->route_id, route->route_id);

}

void add_to_queue(Queue_node_t **queue, size_t *size_queue, char *stop_id, Route_t **routes, size_t total_routes)
{
    size_t i = 0;

    (*queue) = (Queue_node_t *) realloc((*queue), (*size_queue + total_routes) * sizeof(Queue_node_t));

    for (i = *size_queue; i < *size_queue + total_routes; i++)
    {
        fill_queue_node(&(*queue)[i], stop_id, &(*routes)[i - *size_queue]);
    }

    *size_queue += total_routes;
}

void clear_queue(Queue_node_t *queue, size_t size_queue)
{
    size_t i = 0;
    
    for (i = 0; i < size_queue; i++)
    {
        free(queue[i].route_id);
        free(queue[i].stop_id);
    }
}

Interval *get_item(Label_t *labels, char *stop_id)
{
    Label_t *ptr;
    for (ptr = labels; ptr != NULL; ptr = ptr->next)
    {
        if (strcmp(ptr->stop_id, stop_id) == 0)
        {
            return ptr->departure_time;
        }
    }
    return NULL;
}

void delete_item(Label_t **labels, char *stop_id)
{
    Label_t *ptr, *prev;
    for (ptr = *labels, prev = NULL; ptr != NULL; prev = ptr, ptr = ptr->next)
    {
        if (strcmp(ptr->stop_id, stop_id) == 0)
        {
            if (ptr->next != NULL)
            {
                if (prev == NULL)
                {
                    *labels = ptr->next;
                } else 
                {
                    prev->next = ptr->next;
                }
            } else if (prev != NULL)
            {
                prev->next = NULL;
            } else {
                *labels = NULL;
            }

            free(ptr->stop_id);
            free(ptr);
        }
    }
}

void add_item(Label_t **labels, char *stop_id, Interval *value)
{
    Label_t *d = malloc(sizeof(Label_t));

    delete_item(labels, stop_id);

    d->stop_id = malloc(strlen(stop_id)+1);
    strcpy(d->stop_id, stop_id);
    d->departure_time = value;
    d->next = *labels;
    *labels = d;
}