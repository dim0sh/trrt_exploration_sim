#include "trrt.h"
#include "grid.h"
#include "../lib/raylib/raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <time.h>

inline node_t node_new(size_t id, size_t parent_id) {
    node_t result;
    result.id = id;
    result.parent_idx = parent_id;

    return result;
}

graph_t graph_new() {
    graph_t result;
    result.nodes = arr(node_t);
    return result;
}

inline void graph_push_node(graph_t * graph, node_t * node) {
    node->id = graph->nodes->cnt;
    arr_push(node_t,graph->nodes, node);
}

inline node_t * graph_get_node(graph_t * graph, size_t id) {
    return arr_get(node_t,graph->nodes, id);
}

void graph_free(graph_t * graph) {
    arr_free(graph->nodes);
}

trrt_t trrt_init(point3_t start_point, size_t width, size_t height, size_t cell_size) {
    trrt_t result = {
        .min_expansion_ratio = 1,
        .graph = graph_new(),
        .grid = grid_init(width, height, cell_size),
    };
    time_t t = time(NULL);
    srand(t);
    
    // start at robot position (start point)
    node_t start_node = node_new(0, 0);
    start_node.centre = start_point;
    result.start_point = start_point;

    graph_push_node(&result.graph, &start_node);
    return result;
} 

static inline float geometric_dist_quad(point3_t point, point3_t other_point) {
    float dx = point.x - other_point.x;
    float dz = point.z - other_point.z;
    return (dx*dx)+(dz*dz);
}

static inline node_t* t_alg_find_closest_neighbour(graph_t * graph, node_t * new_node, float segment_size) {
    // init test to start of graph
    dynarray_t * nodes = graph->nodes; 
    node_t * tmp_node, * res_node = arr_get(node_t,nodes, 0); 
    point3_t new_point = new_node->centre;

    float tmp_dist, dist = geometric_dist_quad(res_node->centre,new_point);
    // check remaing nodes of graph for distance to new point
    // change output to smallest distance
    for(size_t i = 1; i<nodes->cnt; i++) {
        tmp_node = arr_get(node_t,nodes, i);
        tmp_dist = geometric_dist_quad(tmp_node->centre,new_point);
        if(dist >= tmp_dist) {
            dist = tmp_dist;
            res_node = tmp_node;
        }
    }
    // shift new point closer if to far away
    // also shift further away if to close since exploration has to be ensured
    if (dist > segment_size*segment_size) {
        point3_t p_point = res_node->centre;
        float rx = new_point.x - p_point.x;
        float rz = new_point.z - p_point.z;

        float new_x = p_point.x + ((rx)/sqrtf(dist)) * segment_size *2; 
        float new_z = p_point.z + ((rz)/sqrtf(dist)) * segment_size *2;

        new_node->centre.x = new_x;
        new_node->centre.z = new_z;
    }

    return res_node;
}

static inline bool transition_test(float cost_p, float cost_n, float segment_size) {
    static const float max_cost = MAX_COST-20;
    static const float K = 20;
    float T = 1000;

    if (cost_n > max_cost) {
        return false;
    }

    if (cost_p > cost_n) {
        return true;
    }

    const float delta_cost = cost_n - cost_p;
    static const int max_fail = 10;
    int fail = 0;
    const float p = expf(((delta_cost)/segment_size)/(K*T));
    const float r = (float)(rand() % 100)/100;

    if (r < p) {
        T = T / 1.1;
        fail = 0;
        return true;
    } else {
        if (fail > max_fail) {
            T = T * 1.1;
            fail = 0;
        } else {
            fail += 1;
        }
    }

    return false;
}

void trrt_next(trrt_t * trrt) {
    // begin sampling for solution.
    size_t segment_size = 2;
    size_t rejection_distance = 4;
    float tmp_dist;
    node_t * nearest_node, new_node;

    new_node = node_new(0, 0);
    // new_node.centre = *arr_get(point3_t,t_alg_data->input, sampling_idx);
    
    
    // sample new node from input
    size_t sample_x = rand()%trrt->grid.width;
    size_t sample_z = rand()%trrt->grid.height;

    new_node.centre = (point3_t){sample_x,0,sample_z};

    nearest_node = t_alg_find_closest_neighbour(&trrt->graph, &new_node, segment_size);
    
    new_node.parent_idx = nearest_node->id;
    
    //min expansion control
    tmp_dist = geometric_dist_quad(new_node.centre, nearest_node->centre);
    
    float dist = segment_size;
    trrt->min_expansion_ratio += (tmp_dist<(dist*dist)) * 0.1;
    if (trrt->min_expansion_ratio > 1.2) {
        trrt->min_expansion_ratio -= 0.4;
        return;
    }        
    
    // new node get cost from grid
    
    new_node.cost = grid_get_cost(&trrt->grid, new_node.centre);
    
    // reject nodes too far away in y direction

    if (fabsf(new_node.centre.y - trrt->start_point.y) > rejection_distance) {
        return;
    }

    if (transition_test(nearest_node->cost, new_node.cost, segment_size)) {
        graph_push_node(&trrt->graph, &new_node);
    }    
}

void trrt_destroy(trrt_t * trrt) {
    graph_free(&trrt->graph);
}

void trrt_draw(trrt_t *trrt) {
    size_t cell_size = trrt->grid.cell_size;
    size_t draw_height = 3;
    for (size_t i = 0; i < trrt->graph.nodes->cnt; i++) {
        node_t *node = graph_get_node(&trrt->graph, i);
        Vector3 vec = {node->centre.x*cell_size,draw_height,node->centre.z*cell_size};
        Color color = DARKBLUE;
        DrawSphere(vec, 2, color);
        if (node->id != node->parent_idx) {
            node_t *parent = graph_get_node(&trrt->graph, node->parent_idx);
            Vector3 parent_vec = {parent->centre.x*cell_size,draw_height,parent->centre.z*cell_size};
            // Draw a thicker connection between nodes using a cylinder
            float link_radius = 0.6f; // increase to make the line thicker
            int  link_sides  = 8;     // number of sides for the cylinder
            DrawCylinderEx(parent_vec, vec, link_radius, link_radius, link_sides, BLUE);
        }
    }
}