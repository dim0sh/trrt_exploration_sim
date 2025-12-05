#include "grid.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "../lib/raylib/raylib.h"

void grid_destroy(grid_t * grid) {
    arr_free(grid->grid);
}

void grid_zero(grid_t * grid) {
    size_t cost = 0;
    for (size_t i = 0; i < grid->width*grid->height; i++) {
        arr_push(size_t, grid->grid, &cost);
    }
}

grid_t grid_init(size_t width, size_t height, size_t cell_size) {
    grid_t result = {
        .width = width,
        .height = height,
        .grid = arr(size_t),
        .cell_size = cell_size,
    };
    grid_zero(&result);
    return result;
}

void grid_set_value(grid_t * grid, size_t x, size_t y, size_t cost) {
    if (!grid) return;
    if (x >= grid->width || y >= grid->height) return;
    size_t grid_idx = y * grid->width + x;
    arr_set(size_t, grid->grid, grid_idx, &cost);
}

size_t grid_get_value(grid_t * grid, size_t x, size_t y) {
    if (!grid) return 0;
    if (x >= grid->width || y >= grid->height) return 0;
    size_t grid_idx = y * grid->width + x;
    size_t *cost = arr_get(size_t, grid->grid, grid_idx);
    return *cost;
}

size_t grid_get_cost(grid_t * grid, point3_t point) {
    size_t cost = 0;
    cost = grid_get_value(grid, point.x, point.z);

    return cost;
}

void grid_draw(grid_t * grid) {
    size_t draw_height = 0;
    Color color = RED;
    size_t x = 0, z = 0;
    size_t cell_size = grid->cell_size;
    for (size_t i = 0; i < grid->height; i++) {
        for (size_t j = 0; j < grid->width; j++) {
            
            draw_height = 0;
            x=i*cell_size;
            z=j*cell_size;
            if (grid_get_value(grid, i, j) > MAX_COST) {
                draw_height+=2;
                color = BLACK;
            } else {
                size_t cost = grid_get_value(grid, i, j);
                // printf("cost:\t%lld\n",cost);
                // size_t factor = cost/MAX_COST;

                color = (Color){cost*2,0,200-cost*2,255};
            }
            DrawCube(
                (Vector3){x,draw_height,z}, 
                cell_size, 2, cell_size, 
                color
            );
        }
    }
}

void grid_scenario_one(grid_t * grid) {
    size_t cost = 10;
    for (size_t i = grid->width/2; i<grid->width; i++) {

        for (size_t j = 0; j<grid->height; j++) {
            grid_set_value(grid, i, j, cost);
        }
        if (i%3==0) {
            cost += 15;
        }
    }

    for (int i = 0; i<10; i++) {
        size_t x = rand()%grid->width;
        size_t z = rand()%grid->height;
        for (int j = 0; j < 6; j++) {
            for (int k = 0; k < 4; k++) {
                grid_set_value(grid, x+j, z+k, MAX_COST+1);
            }
        }

    }
}