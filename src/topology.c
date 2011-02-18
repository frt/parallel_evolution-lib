#include "topology.h"
#include "stdlib.h"

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
	free(*adjacency_list);
	*adjacency_list = NULL;
}

status_t adjacency_list_add(adjacency_list_t *adjacency_list, int node_id)
{
	/* TODO */
	return FAIL;
}

status_t adjacency_list_get_all(adjacency_list_t *adjacency_list, int **id_array)
{
	/* TODO */
	return FAIL;
}
/* --- adjacency_list end --- */

/* --- node_list start --- */
status_t node_list_create(node_list_t **node_list)
{
	/* TODO */
	return FAIL;
}

void node_list_destroy(node_list_t **node_list)
{
	/* TODO */
}

status_t node_list_add(node_list_t *node_list, int id)
{
	/* TODO */
	return FAIL;
}

status_t node_list_add_adjacency(node_list_t *node_list, int id, int adjacent_id)
{
	/* TODO */
	return FAIL;
}

status_t node_list_get_first(node_list_t *node_list, node_t **node)
{
	/* TODO */
	return FAIL;
}

status_t node_list_get_next(node_list_t *node_list, node_t **node)
{
	/* TODO */
	return FAIL;
}
/* --- node_list end --- */

/* --- topology start --- */
status_t topology_create(topology_t **topology)
{
	/* TODO */
	return FAIL;
}

void topology_destroy(topology_t **topology)
{
	/* TODO */
}

status_t topology_add_node(topology_t *topology, int node_id)
{
	/* TODO */
	return FAIL;
}

status_t topology_add_adjacency(topology_t *topology, int node_id, int adjacent_node_id)
{
	/* TODO */
	return FAIL;
}

status_t topology_get_first_node(topology_t *topology, int *node_id /* output */, int **adjacency_array /* another output */)
{
	/* TODO */
	return FAIL;
}

status_t topology_get_next_node(topology_t *topology, int *node_id /* output */, int **adjacency_array /* another output */)	/* will return FAIL after the end */
{
	/* TODO */
	return FAIL;
}
/* --- topology end --- */
