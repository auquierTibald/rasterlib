#include "rasterlib.h"

#include <stdlib.h>
#include <string.h>

RL_Context* RL_CreateContext(int width, int heigth)
{
    RL_Context* context = (RL_Context*)malloc(sizeof(RL_Context));
    bzero(context, sizeof(RL_Context));
    context->width = width;
    context->height = heigth;

    context->color_buffer = (uint32_t*)malloc(width * heigth * sizeof(uint32_t));
    context->depth_buffer = (double*)malloc(width * heigth * sizeof(double));


    return context;
}

void RL_DestroyContext(RL_Context* context)
{
    if (context->color_buffer) free(context->color_buffer);
    if (context->depth_buffer) free(context->depth_buffer);
    if (context->triangle_buffer) free(context->triangle_buffer);
    free(context);
}

void RL_SetDisplay(RL_Context* context, int width, int heigth)
{
    context->width = width;
    context->height = heigth;

    free(context->color_buffer);
    free(context->depth_buffer);

    context->color_buffer = malloc(width * heigth * sizeof(unsigned char));
    context->depth_buffer = malloc(width * heigth * sizeof(double));
}
void RL_SetTexture(RL_Context* context, RL_Texture *tex)
{
    context->texture = tex;
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
    if (context->triangle_buffer) free(context->triangle_buffer);
    context->triangle_buffer = malloc(size * sizeof(RL_Triangle));
    memcpy(context->triangle_buffer, data, size * sizeof(RL_Triangle));
    context->triangle_buffer_size = size;
}

void* call_vs_bucket(void* args) {
    RL_Bucket* bucket = (RL_Bucket*) args;
    RL_Context* context = (RL_Context*) bucket->context;
    for (size_t i = bucket->start; i < bucket->end; i++)
        context->vertex_shader(context, &context->triangle_buffer[i]);
    return NULL;
}

void* call_fs_bucket(void* args) {
    RL_Bucket* bucket = (RL_Bucket*) args;
    RL_Context* context = (RL_Context*) bucket->context;
    for (size_t i = bucket->start; i < bucket->end; i++)
        context->fragment_shader(context, &context->triangle_buffer[i]);
    return NULL;
}

void RL_Render(RL_Context* context) {
    if (context->triangle_buffer_size <= N_THREADS) {
        //CALLING VERTEX SHADERS
        for (size_t i = 0; i < context->triangle_buffer_size; i++) {
            RL_Bucket bucket = {.context = context, .start = i, .end = i+1};
            context->buckets[i] = bucket;
            pthread_create(&context->threads[i], NULL, call_vs_bucket, &context->buckets[i]);
        }
        for (size_t i = 0; i < context->triangle_buffer_size; i++) pthread_join(context->threads[i], NULL);

        //CALLING FRAGMENT SHADERS
        for (size_t i = 0; i < context->triangle_buffer_size; i++) {
            pthread_create(&context->threads[i], NULL, call_fs_bucket, &context->buckets[i]);
        }
        for (size_t i = 0; i < context->triangle_buffer_size; i++) pthread_join(context->threads[i], NULL);
    } else {
        size_t size = context->triangle_buffer_size / N_THREADS;
        //CALLING VERTEX SHADERS
        for (size_t i = 0; i < N_THREADS; i++) {
            RL_Bucket bucket = {.context = context};
            bucket.start = i * size;
            if (i == N_THREADS-1) bucket.end = context->triangle_buffer_size;
            else bucket.end = bucket.start + size;
            context->buckets[i] = bucket;
            pthread_create(&context->threads[i], NULL, call_vs_bucket, &context->buckets[i]);
        }
        for (size_t i = 0; i < N_THREADS; i++) pthread_join(context->threads[i], NULL);

        //CALLING FRAGMENT SHADERS
        for (size_t i = 0; i < N_THREADS; i++) {
            pthread_create(&context->threads[i], NULL, call_fs_bucket, &context->buckets[i]);
        }
        for (size_t i = 0; i < N_THREADS; i++) pthread_join(context->threads[i], NULL);
    }
}