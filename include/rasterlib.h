#ifndef RASTERLIB_RASTERLIB_H
#define RASTERLIB_RASTERLIB_H

#include <stdint.h>
#include <pthread.h>
#include "typedefs.h"
#include "assets_loaders.h"

#define N_THREADS 64

typedef struct RL_Context_t {
    int width, height;

    uint32_t *color_buffer;
    double   *depth_buffer;

    RL_Triangle *triangle_buffer;
    size_t triangle_buffer_size;
    RL_Shader vertex_shader, fragment_shader;
    RL_Texture *texture;
    matrix model_matrix, view_matrix;

    pthread_t threads[N_THREADS];
    RL_Bucket buckets[N_THREADS];

    RL_AssetManager *asset_manager;
} RL_Context;

RL_Context* RL_CreateContext(int width, int heigth);
void RL_DestroyContext(RL_Context* context);

void RL_SetDisplay(RL_Context* context, int width, int heigth);
void RL_SetTexture(RL_Context* context, RL_Texture *tex);
void RL_SetModelMatrix(RL_Context* context, matrix model_matrix);
void RL_SetViewMatrix(RL_Context* context, matrix view_matrix);


void RL_UseShader(RL_Context* context, RL_Shader shader, int shader_type);
void RL_TriangleData(RL_Context* context, RL_Triangle* data, size_t size);
void RL_Render(RL_Context* context);


#endif //RASTERLIB_RASTERLIB_H