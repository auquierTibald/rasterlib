#include "rasterlib.h"

#include <stdlib.h>
#include <string.h>

RL_Context* RL_CreateContext(int width, int heigth, SDL_Renderer* renderer)
{
    RL_Context* context = (RL_Context*)malloc(sizeof(RL_Context));
    bzero(context, sizeof(RL_Context));
    context->width = width;
    context->height = heigth;
    context->renderer = renderer;

    context->color_buffer = (RL_Color*)malloc(width * heigth * sizeof(RL_Color));
    context->depth_buffer = (double*)malloc(width * heigth * sizeof(double));

    context->screen_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, heigth);
    return context;
}

void RL_DestroyContext(RL_Context* context)
{
    if (context->color_buffer) free(context->color_buffer);
    if (context->depth_buffer) free(context->depth_buffer);
    if (context->vertex_buffer.data) da_free(&context->vertex_buffer);
    if (context->fragment_buffer.data) da_free(&context->fragment_buffer);
    free(context);
}

void RL_SetDisplay(RL_Context* context, int width, int heigth)
{
    context->width = width;
    context->height = heigth;

    if (context->color_buffer) free(context->color_buffer);
    if (context->depth_buffer) free(context->depth_buffer);

    context->color_buffer = malloc(width * heigth * sizeof(RL_Color));
    context->depth_buffer = malloc(width * heigth * sizeof(double));
    SDL_DestroyTexture(context->screen_texture);
    context->screen_texture = SDL_CreateTexture(context->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, heigth);
}
void RL_SetTexture(RL_Context* context, RL_Texture *tex)
{
    context->texture = tex;
}

void RL_SetModelMatrix(RL_Context* context, matrix model_matrix);
void RL_SetViewMatrix(RL_Context* context, matrix view_matrix);

void RL_Pixel(RL_Context* context, int x, int y, RL_Color c) {
    context->color_buffer[y * context->width + x] = c;
}

void RL_Clear(RL_Context* context, RL_Color c)
{
    for (int i = 0; i < context->width * context->height; i++) context->color_buffer[i] = c;
}



void RL_UseShader(RL_Context* context, RL_Shader shader, int shader_type)
{
    switch(shader_type) {
        case RL_VERTEX_SHADER :
            context->vertex_shader = shader;
            break;
        case RL_FRAGMENT_SHADER :
            context->fragment_shader = shader;
            break;
    }
}

void RL_TriangleData(RL_Context* context, RL_Triangle* data, size_t size)
{
    if (context->vertex_buffer.data) da_free(&context->vertex_buffer);
    context->vertex_buffer = (da_RL_Triangle){.data = malloc(sizeof(RL_Triangle) * size), .size = size, .capacity = size};
    memcpy(context->vertex_buffer.data, data, size * sizeof(RL_Triangle));
}

void* vs_bucket_call(void* args) {
    RL_Bucket* bucket = (RL_Bucket*) args;
    RL_Context* context = (RL_Context*) bucket->context;
    da_range(&context->vertex_buffer, RL_Triangle, bucket->start, bucket->end)
        context->fragment_shader(context, element);
    return NULL;
}

void* fs_bucket_call(void* args) {
    RL_Bucket* bucket = (RL_Bucket*) args;
    RL_Context* context = (RL_Context*) bucket->context;
    da_range(&context->fragment_buffer, RL_Triangle, bucket->start, bucket->end)
        context->fragment_shader(context, element);
    return NULL;
}

RL_Bucket create_bucket(RL_Context* context, size_t i, size_t size) {
    RL_Bucket bucket = {.context = context};
    bucket.start = i * size;
    if (i == N_THREADS-1) bucket.end = context->vertex_buffer.size;
    else bucket.end = bucket.start + size;
    return bucket;
}

void RL_Render(RL_Context* context) {

    size_t size = context->vertex_buffer.size / N_THREADS;

    //CALLING VERTEX SHADERS
    for (size_t i = 0; i < N_THREADS; i++) {
        context->vertex_buckets[i] = create_bucket(context, i, size);
        pthread_create(&context->threads[i], NULL, vs_bucket_call, &context->vertex_buckets[i]);
    }
    for (size_t i = 0; i < N_THREADS; i++) pthread_join(context->threads[i], NULL);

    size = context->fragment_buffer.size / N_THREADS;

    //CALLING FRAGMENT SHADERS
    for (size_t i = 0; i < N_THREADS; i++) {
        context->fragment_buckets[i] = create_bucket(context, i, size);
        pthread_create(&context->threads[i], NULL, fs_bucket_call, &context->fragment_buckets[i]);
    }
    for (size_t i = 0; i < N_THREADS; i++) pthread_join(context->threads[i], NULL);

    SDL_UpdateTexture(context->screen_texture, NULL, context->color_buffer, context->width * sizeof(RL_Color));
    SDL_RenderCopy(context->renderer, context->screen_texture, NULL, NULL);
}