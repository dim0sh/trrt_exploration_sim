#ifndef TRRT_H
#define TRRT_H

#include "../lib/dynarray/dynarray.h"
#include <stddef.h>
#include "grid.h"

/*
 * graph and node structs
*/

typedef struct Node {
    size_t id;
    size_t parent_idx;
    float cost;
    point3_t centre;
} node_t;

typedef struct Graph {
    dynarray_t * nodes;
} graph_t;

graph_t graph_new();
node_t * graph_get_node(graph_t * graph, size_t id);

void graph_free(graph_t * graph);

typedef struct Trrt {
    float min_expansion_ratio;
    point3_t start_point;
    graph_t graph;
    grid_t grid;
} trrt_t;

trrt_t trrt_init(point3_t start_point, size_t width, size_t height, size_t cell_size);

void trrt_next(trrt_t * t_alg_data);

void trrt_destroy(trrt_t * trrt);

void trrt_draw(trrt_t * trrt);
#endif