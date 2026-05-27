#ifndef RASTERLIB_RASTERLIB_H
#define RASTERLIB_RASTERLIB_H

#include <SDL2/SDL.h>
#include "typedefs.h"
#include "assets_loaders.h"

#define N_THREADS 16

typedef da(RL_Triangle) da_RL_Triangle;
typedef da(RL_Fragment) da_RL_Fragment;

typedef struct RL_Context_t {
    int width, height;
    float ratio;

    pthread_t threads[N_THREADS];
    pthread_mutex_t mutex;

    RL_Color *color_buffer;
    double   *depth_buffer;

    RL_Bucket vertex_buckets[N_THREADS];
    RL_Bucket fragment_buckets[N_THREADS];

    da_RL_Triangle vertex_input_buffer, vertex_output_buffer;
    da_RL_Fragment fragment_buffer;
    RL_VertexShader vertex_shader;
    RL_FragmentShader fragment_shader;

    RL_Texture *texture;
    matrix model_matrix, view_matrix;

    RL_AssetManager asset_manager;

    SDL_Renderer *renderer;
    SDL_Texture *screen_texture;

} RL_Context;

RL_Context* RL_CreateContext(int width, int heigth, SDL_Renderer* renderer);
void RL_DestroyContext(RL_Context* context);

void RL_SetDisplay(RL_Context* context, int width, int heigth);
void RL_SetTexture(RL_Context* context, RL_Texture *tex);
void RL_SetModelMatrix(RL_Context* context, matrix model_matrix);
void RL_SetViewMatrix(RL_Context* context, matrix view_matrix);

void RL_Pixel(RL_Context* context, int x, int y, RL_Color c);
void RL_Clear(RL_Context* context, RL_Color c);

void RL_SetVertexShader(RL_Context* context, RL_VertexShader shader);
void RL_SetFragmentShader(RL_Context* context, RL_FragmentShader shader);
void RL_TriangleData(RL_Context* context, RL_Triangle* data, size_t size);
void RL_MeshData(RL_Context* context, RL_Mesh* mesh);
void RL_Render(RL_Context* context);


#endif //RASTERLIB_RASTERLIB_H