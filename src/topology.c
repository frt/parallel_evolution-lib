#include "parallel_evolution/topology.h"
#include <stdlib.h>

/* --- adjacency_list start --- */
status_t adjacency_list_create(adjacency_list_t **adjacency_list)
{
	adjacency_list_t *new_adjacency_list;

	*adjacency_list = NULL;

	new_adjacency_list = (adjacency_list_t *)malloc(sizeof(adjacency_list_t));
	if (new_adjacency_list == NULL)
		return FAIL;
	new_adjacency_list->first = NULL;
	new_adjacency_list->last = NULL;
	new_adjacency_list->count= 0;

	*adjacency_list = new_adjacency_list;

	return SUCCESS;
}

void adjacency_list_destroy(adjacency_list_t **adjacency_list)
{
	adjacency_t *cur_adj, *aux;

	cur_adj = (*adjacency_list)->first;
	while (cur_adj != NULL) {
		aux = cur_adj;
		cur_adj = cur_adj->next;
		free(aux);
	}

	free(*adjacency_list);
	*adjacency_list = NULL;
}

status_t adjacency_list_add(adjacency_list_t *adjacency_list, int node_id)
{
	adjacency_t *new_adjacency;

	new_adjacency = (adjacency_t *)malloc(sizeof(adjacency_t));
	if (new_adjacency == NULL)
		return FAIL;

	new_adjacency->node_id = node_id;
	new_adjacency->next = NULL;

	if (adjacency_list->first == NULL) {	/* empty list */
		adjacency_list->first = new_adjacency;
	} else {
		adjacency_list->last->next = new_adjacency;
	}
	adjacency_list->last = new_adjacency;
	adjacency_list->count += 1;
	
	return SUCCESS;
}

void adjacency_list_remove(adjacency_list_t *adjacency_list, int node_id)
{
	adjacency_t *cur_adj, *prev_adj = NULL, *to_remove = NULL;

	for (cur_adj = adjacency_list->first; cur_adj != adjacency_list->last; prev_adj = cur_adj, cur_adj = cur_adj->next) {
		if (cur_adj->node_id == node_id) {
			if (prev_adj == NULL)
				adjacency_list->first = cur_adj->next;
			else
				prev_adj->next = cur_adj->next;
			to_remove = cur_adj;
		}
		cur_adj = cur_adj->next;
		if (to_remove != NULL) {
			free(to_remove);
			to_remove = NULL;
		}
	}
}

status_t adjacency_list_get_all(adjacency_list_t *adjacency_list, int **id_array)
{
	int *new_array;
	adjacency_t *cur_adj;
	int i;

	new_array = (int *)malloc(adjacency_list->count * sizeof(int));
	if (new_array == NULL)
		return FAIL;

	cur_adj = adjacency_list->first;
	for (i = 0; cur_adj != NULL; ++i) {
		new_array[i] = cur_adj->node_id;
		cur_adj = cur_adj->next;
	}

	*id_array = new_array;

	return SUCCESS;
}
/* --- adjacency_list end --- */

/* --- node start --- */
void node_remove(node_t *to_remove, node_t **prev_next)
{
	*prev_next = to_remove->next;

	/* destroy node */
	adjacency_list_destroy(&(to_remove->adjacency_list));
	free(to_remove);
}
/* --- node end --- */

/* --- node_list start --- */
status_t node_list_create(node_list_t **node_list)
{
	node_list_t *new_node_list;

	*node_list = NULL;

	new_node_list = (node_list_t *)malloc(sizeof(node_list_t));
	if (new_node_list == NULL)
		return FAIL;
	new_node_list->first = NULL;
	new_node_list->last = NULL;
	new_node_list->count= 0;

	*node_list = new_node_list;

	return SUCCESS;
}

void node_list_destroy(node_list_t **node_list)
{
	node_t *cur_node, *aux;

	cur_node = (*node_list)->first;
	while (cur_node != NULL) {
		aux = cur_node;
		cur_node = cur_node->next;
		adjacency_list_destroy(&(aux->adjacency_list));
		free(aux);
	}

	free(*node_list);
	*node_list = NULL;
}

status_t node_list_add(node_list_t *node_list, int id)
{
	node_t *new_node;

	new_node = (node_t *)malloc(sizeof(node_t));
	if (new_node == NULL)
		return FAIL;

	if (! adjacency_list_create(&(new_node->adjacency_list)) ) {
		free(new_node);
		return FAIL;
	}
	new_node->id = id;
	new_node->next = NULL;

	if (node_list->first == NULL) {
		node_list->first = new_node;
	} else {
		node_list->last->next = new_node;
	}
	node_list->last = new_node;
	node_list->count += 1;

	return SUCCESS;
}

status_t node_list_remove(node_list_t *node_list, int id)
{
	node_t *cur_node = NULL;
	node_t *prev_node = NULL;

	/* get first node */
	if (node_list_get_first(node_list, &cur_node) == FAIL)	/* node_list is empty */
		return FAIL;

	/* search for the wanted node */
	while (cur_node->id != id) {
		prev_node = cur_node;
		if (node_list_get_next(&cur_node) == FAIL)	/* node not found */
			return FAIL;
	}

	if (prev_node == NULL)	/* node found is the first */
		node_remove(cur_node, &(node_list->first));
	else
		node_remove(cur_node, &(prev_node->next));
}

node_t *node_list_find(node_list_t *node_list, int id)
{
	node_t *cur_node;

	cur_node = node_list->first;
	while (cur_node != NULL) {
		if (cur_node->id == id)
			return cur_node;
		cur_node = cur_node->next;
	}
	return NULL;	/* node with id doesn't exist */
}

status_t node_list_add_adjacency(node_list_t *node_list, int id, int adjacent_id)
{
	node_t *node;

	node = node_list_find(node_list, id);
	if (node == NULL)	/* node not found */
		return FAIL;
	else
		return adjacency_list_add(node->adjacency_list, adjacent_id);
}

status_t node_list_remove_adjacency(node_list_t *node_list, int id, int adjacent_id)
{
	node_t *node;

	node = node_list_find(node_list, id);
	if (node == NULL)	/* node not found */
		return FAIL;
	else {
		adjacency_list_remove(node->adjacency_list, adjacent_id);
		return SUCCESS;
	}
}

status_t node_list_get_first(node_list_t *node_list, node_t **node)
{
	if (node_list->first == NULL)
		return FAIL;

	*node = node_list->first;
	return SUCCESS;
}

status_t node_list_get_next(node_t **node)
{
	if ((*node)->next == NULL)
		return FAIL;

	*node = (*node)->next;
	return SUCCESS;
}
/* --- node_list end --- */

/* --- topology start --- */
status_t topology_create(topology_t **topology)
{
	topology_t *new_topology;

	new_topology = (topology_t *)malloc(sizeof(topology_t));
	if (new_topology == NULL)
		return FAIL;
	if (! node_list_create(&(new_topology->node_list)))
		return FAIL;
	new_topology->current_node = NULL;

	*topology = new_topology;
	return SUCCESS;
}

void topology_destroy(topology_t **topology)
{
	node_list_destroy(&((*topology)->node_list));
	free(*topology);
	*topology = NULL;
}

status_t topology_add_node(topology_t *topology, int node_id)
{
	return node_list_add(topology->node_list, node_id);
}

status_t topology_add_adjacency(topology_t *topology, int node_id, int adjacent_node_id)
{
	return node_list_add_adjacency(topology->node_list, node_id, adjacent_node_id);
}

status_t node_get(node_t *node, int *id /* output */, int **adjacency_array /* another output */, int *count)
{
	if (node == NULL)
		return FAIL;
	*id = node->id;
	*count = node->adjacency_list->count;
	return adjacency_list_get_all(node->adjacency_list, adjacency_array);	/* fail if array allocation fails */
}

status_t topology_get_first_node(topology_t *topology, int *node_id /* output */, int **adjacency_array /* another output */, int *count)
{
	topology->current_node = topology->node_list->first;
	return node_get(topology->current_node, node_id, adjacency_array, count);
}

/* will return FAIL after the end */
status_t topology_get_next_node(topology_t *topology, int *node_id /* output */, int **adjacency_array /* another output */, int *count)
{
	topology->current_node = topology->current_node->next;
	return node_get(topology->current_node, node_id, adjacency_array, count);
}
/* --- topology end --- */
