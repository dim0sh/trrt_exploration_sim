#include "sim.h"
// #include "dynarray/dynarray.h"
#include "grid.h"
#include "ui_renderer.h"
#include "trrt.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

base_sim_model *base_sim_model_init(
    int fps,
    int window_width,
    int window_height,
    const char *title,
    int camera_mode
) {
    base_sim_model *model = malloc(sizeof(*model));
    model->fps = fps;
    model->window_width = window_width;
    model->window_height = window_height;
    model->title = title;
    model->camera_mode = camera_mode;
    Camera camera = { 0 };
    camera.position = (Vector3){ 20.0f, 20.0f, 20.0f };    
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          
    camera.fovy = 104.0f;                                
    camera.projection = CAMERA_PERSPECTIVE; 
    model->camera = camera;
    model->ctx = malloc(sizeof(mu_Context));
    return model;
}

sim_model *sim_model_init(
    int fps, 
    int window_width, 
    int window_height, 
    const char *title, 
    int camera_mode,
    data_model *data_model
) {
    sim_model *model = malloc(sizeof(*model));
    model->base_model = base_sim_model_init(fps, window_width, window_height, title, camera_mode);
    model->data_model = data_model;
    return model;
}

void sim_init(sim_model *model, bool cursor) {
    InitWindow(model->base_model->window_width, model->base_model->window_height, model->base_model->title);
    
    SetTargetFPS(model->base_model->fps);

    if (!cursor) {
        DisableCursor(); 
    }
    mu_init(model->base_model->ctx);
    /* try to load a larger custom font from the repo fonts folder; fallback to default */
    Font loaded = LoadFontEx("fonts/Science_Gothic/static/ScienceGothic-Regular.ttf", 32, NULL, 0);
    if (loaded.texture.id != 0) {
        model->base_model->font = loaded;
    } else {
        model->base_model->font = GetFontDefault();
    }

    murl_setup_font_ex(model->base_model->ctx, &model->base_model->font);
    // delfault ui layout x width
    model->base_model->ctx->style->size.x = 200;
}
// ---
// sim data
// ---
struct DataModel {
    float time;
    int active_trrt;
    int pause;
    size_t sim_step;
    trrt_t trrt;
};
// ---
// ui declaration
// ---
void sim_ui(sim_model *model) {
    mu_Context *ctx = model->base_model->ctx;
    murl_handle_input(ctx);
    mu_begin(ctx);
    if (mu_begin_window(ctx, "Simulation Settings", mu_rect(20, 20, 300, 100))) {
        mu_checkbox(ctx, "trrt active", &model->data_model->active_trrt);
        mu_label(ctx, "Simulation Step");
        char buff[12];
        sprintf(buff,"%lld",model->data_model->sim_step); mu_label(ctx, buff);
        mu_end_window(ctx);
    }
    mu_end(ctx);
}
// ---
// draw loop
// ---
void sim_draw(sim_model *model) {
    BeginDrawing();
        ClearBackground(WHITE);
        BeginMode3D(model->base_model->camera);
            DrawGrid(100, 10.0);
            grid_draw(&model->data_model->trrt.grid);
            trrt_draw(&model->data_model->trrt);
        EndMode3D();
        murl_render(model->base_model->ctx);
    EndDrawing();
}
// ---
// update loop
// ---
void sim_update(sim_model *model) {
    model->data_model->time += GetFrameTime();
    if (model->data_model->time > 0.2 && model->data_model->active_trrt) {
        model->data_model->time = 0;
        trrt_next(&model->data_model->trrt); 
        model->data_model->sim_step++;
    }
}

void sim_window(sim_model *model) {
    while (!WindowShouldClose()) {
        sim_update(model);
        sim_ui(model);
        
        if (IsKeyPressed(KEY_P)) {
            model->data_model->pause = (model->data_model->pause + 1)%2;
            if (!model->data_model->pause) {
                DisableCursor();
            } else {
                EnableCursor();
            }
        }
        if (!model->data_model->pause) {
            // DisableCursor();
            UpdateCamera(&model->base_model->camera, model->base_model->camera_mode);
        }

        sim_draw(model);
    }
    CloseWindow();
}

void sim_destroy(sim_model *model) {
    /* unload font if it was loaded from file (GetFontDefault() shares system font texture) */
    if (model->base_model->font.texture.id != GetFontDefault().texture.id) {
        UnloadFont(model->base_model->font);
    }
    free(model->base_model->ctx);
    free(model->base_model);
}
// ---
// main loop
// ---
void sim_loop() {
    data_model sim_data = (data_model){
        .time = 0,
        .trrt = trrt_init(
            (point3_t){0,0,0},
            50,
            50,
            4),
        .active_trrt = 0,
        .pause = 0,
        .sim_step = 0,
    };

    sim_model *model = sim_model_init(
        1000, 
        2560, 
        1440, 
        "TRRT simulation", 
        CAMERA_ORTHOGRAPHIC,
        &sim_data
    );
    // init values here
    printf("sim_start\n");
    grid_scenario_one(&sim_data.trrt.grid);
    sim_init(model, false);           

    // this is the loop
    sim_window(model);
    // after loop

    trrt_destroy(&sim_data.trrt);
    sim_destroy(model);
}