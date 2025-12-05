#ifndef GRID_H
#define GRID_H

#define MAX_COST 100

#include "../lib/dynarray/dynarray.h"
#include <stddef.h>

typedef struct Point3D {
    float x;
    float y;
    float z;
} point3_t;

typedef struct Point4D {
    float a;
    float b;
    float c;
    float d;
} point4_t;

typedef struct Grid {
    size_t width;
    size_t height;
    size_t cell_size;
    dynarray_t * grid;
} grid_t;

grid_t grid_init(size_t width, size_t height, size_t cell_size);

void grid_one(grid_t * grid);

void grid_destroy(grid_t * grid);

void grid_set_value(grid_t * grid, size_t x, size_t y, size_t cost);

size_t grid_get_cost(grid_t * grid, point3_t point);

void grid_draw(grid_t * grid);

void grid_scenario_one(grid_t * grid);

#endif