#pragma once

#include "common.h"

/* --- adjacency_list start --- */
typedef struct adjacency {
	int node_id;
	struct adjacency *next;
} adjacency_t;

typedef struct adjacency_list {
	adjacency_t *first;
	adjacency_t *last;
	int count;
} adjacency_list_t;

status_t adjacency_list_create(adjacency_list_t **adjacency_list);
void adjacency_list_destroy(adjacency_list_t **adjacency_list);
status_t adjacency_list_add(adjacency_list_t *adjacency_list, int node_id);
/* TODO adjacency_list_remove(adjacency_list_t *adjacency_list, int node_id) */
status_t adjacency_list_get_all(adjacency_list_t *adjacency_list, int **id_array);
/* --- adjacency_list end --- */

/* --- node_list start --- */
typedef struct node {
	int id;
	adjacency_list_t *adjacency_list;
	struct node *next;	
} node_t;

typedef struct node_list {
	node_t *first;
	node_t *last;
	int count;
} node_list_t;

status_t node_list_create(node_list_t **node_list);
void node_list_destroy(node_list_t **node_list);
status_t node_list_add(node_list_t *node_list, int id);
/* TODO node_list_remove() */
status_t node_list_add_adjacency(node_list_t *node_list, int id, int adjacent_id);
/* TODO node_list_remove_adjacency() */
status_t node_list_get_first(node_list_t *node_list, node_t **node);
status_t node_list_get_next(node_t **node);
/* --- node_list end --- */

/* --- topology start --- */
/* this is a utility struct with utility functions to ease the manipulation of the topology, users should use only the topology_*() functions */
typedef struct topology {
	node_list_t *node_list;
	node_t *current_node;	/* will act as a cursor to traverse the node_list */
} topology_t;

status_t topology_create(topology_t **topology);
void topology_destroy(topology_t **topology);
status_t topology_add_node(topology_t *topology, int node_id);
/* TODO topology_remove_node() */
status_t topology_add_adjacency(topology_t *topology, int node_id, int adjacent_node_id);
/* TODO topology_remove_adjacency() */
status_t topology_get_first_node(topology_t *topology, int *node_id /* output */, int **adjacency_array /* another output */);
status_t topology_get_next_node(topology_t *topology, int *node_id /* output */, int **adjacency_array /* another output */);	/* will return FAIL after the end */
/* --- topology end --- */
