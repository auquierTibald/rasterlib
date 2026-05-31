#include "rasterlib.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "utils-matrix.h"
#include "utils.h"

void default_vs(struct RL_Context_t *context, RL_Triangle *triangle)
{
      triangle3 mvp = {
          .a = project_vertex(context, mat_apply(context->view_matrix, mat_apply(context->model_matrix, triangle->pos.a))),
          .b = project_vertex(context, mat_apply(context->view_matrix, mat_apply(context->model_matrix, triangle->pos.b))),
          .c = project_vertex(context, mat_apply(context->view_matrix, mat_apply(context->model_matrix, triangle->pos.c)))
      };
     // triangle3 mvp = {
     //     .a = mat_apply(context->view_matrix, mat_apply(context->model_matrix, triangle->pos.a)),
     //     .b = mat_apply(context->view_matrix, mat_apply(context->model_matrix, triangle->pos.b)),
     //     .c = mat_apply(context->view_matrix, mat_apply(context->model_matrix, triangle->pos.c))
     // };
    RL_Triangle new = { .pos = mvp, .tex = triangle->tex, .normal = triangle->normal };
    pthread_mutex_lock(&context->mutex);
    da_append(&context->vertex_output_buffer, RL_Triangle, new);
    pthread_mutex_unlock(&context->mutex);
}

void default_fs(struct RL_Context_t *context, RL_Fragment *frag) {
    //color = texture_sample(context->texture, tex_coord);
    frag->color = (RL_Color){.uint32 = frag->depth };
}


RL_Context* RL_CreateContext(int width, int heigth, SDL_Renderer* renderer)
{
    RL_Context* context = (RL_Context*)malloc(sizeof(RL_Context));
    bzero(context, sizeof(RL_Context));
    context->width = width;
    context->height = heigth;
    context-> ratio = (float)heigth / (float)width;
    context->renderer = renderer;

    context->vertex_shader = default_vs;
    context->fragment_shader = default_fs;

    context->color_buffer = (RL_Color*)malloc(width * heigth * sizeof(RL_Color));
    context->depth_buffer = (float*)malloc(width * heigth * sizeof(float));

    context->vertex_input_buffer = (da_RL_Triangle)da_alloc(RL_Triangle, 1);
    context->vertex_output_buffer = (da_RL_Triangle)da_alloc(RL_Triangle, 1);
    context->asset_manager = (RL_AssetManager)da_alloc(RL_Asset, 1);

    context->screen_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, heigth);
    return context;
}

void RL_DestroyContext(RL_Context* context)
{
    if (context->color_buffer) free(context->color_buffer);
    if (context->depth_buffer) free(context->depth_buffer);
    if (context->vertex_input_buffer.data) da_free(&(context->vertex_input_buffer));
    if (context->vertex_output_buffer.data) da_free(&(context->vertex_output_buffer));
    free(context);
}

void RL_SetDisplay(RL_Context* context, int width, int heigth)
{
    context->width = width;
    context->height = heigth;
    context-> ratio = (float)heigth / (float)width;

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

void RL_SetModelMatrix(RL_Context* context, matrix model_matrix)
{
    context->model_matrix = model_matrix;
}
void RL_SetViewMatrix(RL_Context* context, matrix view_matrix)
{
    context->view_matrix = view_matrix;
}

void RL_Pixel(RL_Context* context, int x, int y, RL_Color c)
{
    context->color_buffer[y * context->width + x] = c;
}

void RL_Clear(RL_Context* context, RL_Color c)
{
    for (int i = 0; i < context->width * context->height; i++) {
        context->color_buffer[i] = c;
        context->depth_buffer[i] = FAR_PLANE;
    }
}

void RL_SetVertexShader(RL_Context* context, RL_VertexShader shader) { context->vertex_shader = shader; }
void RL_SetFragmentShader(RL_Context* context, RL_FragmentShader shader)   { context->fragment_shader = shader; }

void RL_TriangleData(RL_Context* context, RL_Triangle* data, size_t size)
{
    if (context->vertex_input_buffer.data) da_free(&(context->vertex_input_buffer));
    context->vertex_input_buffer = (da_RL_Triangle){.data = malloc(sizeof(RL_Triangle) * size), .size = size, .capacity = size};
    memcpy(context->vertex_input_buffer.data, data, size * sizeof(RL_Triangle));
}

void RL_MeshData(RL_Context* context, RL_Mesh* mesh)
{
    da_clear(&context->vertex_input_buffer);
    for (size_t i = 0; i < mesh->i_coords.size; i++) {

        vec3 v1 = mesh->v_coords.data[mesh->i_coords.data[i].x];
        vec3 v2 = mesh->v_coords.data[mesh->i_coords.data[i].y];
        vec3 v3 = mesh->v_coords.data[mesh->i_coords.data[i].z];

        vec2 vt1 = mesh->v_tex_coords.data[mesh->i_tex_coords.data[i].x];
        vec2 vt2 = mesh->v_tex_coords.data[mesh->i_tex_coords.data[i].y];
        vec2 vt3 = mesh->v_tex_coords.data[mesh->i_tex_coords.data[i].z];

        vec3 vn1 = mesh->v_normals.data[mesh->i_normals.data[i].x];
        vec3 vn2 = mesh->v_normals.data[mesh->i_normals.data[i].y];
        vec3 vn3 = mesh->v_normals.data[mesh->i_normals.data[i].z];

        RL_Triangle tri = (RL_Triangle){
            .pos = (triangle3){v1, v2, v3},
            .tex = (triangle2){vt1 ,vt2, vt3},
            .normal = (triangle3){vn1 ,vn2, vn3}
        };

        da_append(&context->vertex_input_buffer, RL_Triangle, tri);
    }
}

void draw_fragment(RL_Context* context, RL_Fragment* frag) {

    ivec2 v1 = ivec2( (int)frag->tri->pos.a.x, (int)frag->tri->pos.a.y);
    ivec2 v2 = ivec2( (int)frag->tri->pos.b.x, (int)frag->tri->pos.b.y);
    ivec2 v3 = ivec2( (int)frag->tri->pos.c.x, (int)frag->tri->pos.c.y);

    vec3 depths = vec3(frag->tri->pos.a.z, frag->tri->pos.b.z, frag->tri->pos.c.z);

    if ( pointInTriangle(v1, v2, v3, frag->pos, &frag->weights)) {

        frag->depth = 1 / dot3(vec3(1 / depths.x, 1 / depths.y, 1 / depths.z), frag->weights);

        pthread_mutex_lock(&context->mutex);
        if(frag->depth >= context->depth_buffer[screen_index(context->width, frag->pos)]) {
            pthread_mutex_unlock(&context->mutex);
            return;
        }
        pthread_mutex_unlock(&context->mutex);

        //TEXTURE COORDINATES INTERPOLATION
        frag->tex_coord = product2(product2( frag->tri->tex.a, 1/depths.x ), frag->weights.x);
        frag->tex_coord = sum2(frag->tex_coord, product2(product2( frag->tri->tex.b, 1/depths.y ), frag->weights.y));
        frag->tex_coord = sum2(frag->tex_coord, product2(product2( frag->tri->tex.c, 1/depths.z ), frag->weights.z));
        frag->tex_coord = product2(frag->tex_coord, frag->depth);

        context->fragment_shader(context, frag);

        pthread_mutex_lock(&context->mutex);
        if(frag->depth >= context->depth_buffer[screen_index(context->width, frag->pos)]) {
            pthread_mutex_unlock(&context->mutex);
            return;
        }
        context->color_buffer[screen_index(context->width, frag->pos)] = frag->color;
        context->depth_buffer[screen_index(context->width, frag->pos)] = frag->depth;
        pthread_mutex_unlock(&context->mutex);
    }
}

void draw_triangle(RL_Context* context, RL_Triangle *tri) {
    //2D POINTS
    ivec2 v1 = ivec2( (int)tri->pos.a.x, (int)tri->pos.a.y);
    ivec2 v2 = ivec2( (int)tri->pos.b.x, (int)tri->pos.b.y);
    ivec2 v3 = ivec2( (int)tri->pos.c.x, (int)tri->pos.c.y);

    int minx = min3(v1.x, v2.x, v3.x); clamp(&minx, 0, context->width);
    int miny = min3(v1.y, v2.y, v3.y); clamp(&miny, 0, context->height);
    int maxx = max3(v1.x, v2.x, v3.x); clamp(&maxx, 0, context->width);
    int maxy = max3(v1.y, v2.y, v3.y); clamp(&maxy, 0, context->height);

    vec3 depths = vec3(tri->pos.a.z, tri->pos.b.z, tri->pos.c.z);
    
    for(int x = minx ; x < maxx ; x++) {
        for(int y = miny ; y < maxy; y++) {

            RL_Fragment frag = {.tri = tri, .pos = ivec2(x, y)};

            if ( pointInTriangle(v1, v2, v3, frag.pos, &frag.weights)) {

                frag.depth = 1 / dot3(vec3(1 / depths.x, 1 / depths.y, 1 / depths.z), frag.weights);

                pthread_mutex_lock(&context->mutex);
                if(frag.depth >= context->depth_buffer[screen_index(context->width, frag.pos)]) {
                    pthread_mutex_unlock(&context->mutex);
                    continue;
                }
                pthread_mutex_unlock(&context->mutex);

                //TEXTURE COORDINATES INTERPOLATION
                frag.tex_coord = product2(product2( tri->tex.a, 1/depths.x ), frag.weights.x);
                frag.tex_coord = sum2(frag.tex_coord, product2(product2( tri->tex.b, 1/depths.y ), frag.weights.y));
                frag.tex_coord = sum2(frag.tex_coord, product2(product2( tri->tex.c, 1/depths.z ), frag.weights.z));
                frag.tex_coord = product2(frag.tex_coord, frag.depth);

                context->fragment_shader(context, &frag);

                pthread_mutex_lock(&context->mutex);
                if(frag.depth >= context->depth_buffer[screen_index(context->width, frag.pos)]) {
                    pthread_mutex_unlock(&context->mutex);
                    continue;
                }
                context->color_buffer[screen_index(context->width, frag.pos)] = frag.color;
                context->depth_buffer[screen_index(context->width, frag.pos)] = frag.depth;
                pthread_mutex_unlock(&context->mutex);
            }
        }
    }

}

RL_Bucket create_bucket(RL_Context* context, size_t i, size_t size) {
    RL_Bucket bucket = {.context = context};
    if (size < N_THREADS) {
        if ( i >= size) return (RL_Bucket){.context = context, .start = 0, .end = 0};
        return (RL_Bucket){.context = context, .start = i, .end = i+1};
    }
    bucket.start = i * size/N_THREADS;
    if (i == N_THREADS-1) bucket.end = size;
    else bucket.end = bucket.start + size/N_THREADS;
    return bucket;
}

void* call_vertex_bucket(void* args) {
    RL_Bucket* bucket = args;
    RL_Context* context = bucket->context;
    da_range(&context->vertex_input_buffer, RL_Triangle, bucket->start, bucket->end)
        context->vertex_shader(context, element);
    return NULL;
}

void* call_fragment_bucket(void* args) {
    RL_Bucket* bucket = args;
    RL_Context* context = bucket->context;
    da_range(&context->vertex_output_buffer, RL_Triangle, bucket->start, bucket->end)
        draw_triangle(context, element);
    return NULL;
}

void RL_Render(RL_Context* context) {
    da_clear(&(context->vertex_output_buffer));
    size_t size = context->vertex_input_buffer.size;

    //CALLING VERTEX SHADERS
    for (size_t i = 0; i < N_THREADS; i++) {
        context->vertex_buckets[i] = create_bucket(context, i, size);
        pthread_create(&context->threads[i], NULL, call_vertex_bucket, &context->vertex_buckets[i]);
    }
    for (size_t i = 0; i < N_THREADS; i++) pthread_join(context->threads[i], NULL);

    size = context->vertex_output_buffer.size;

    //CALLING FRAGMENT SHADERS
    for (size_t i = 0; i < N_THREADS; i++) {
        context->fragment_buckets[i] = create_bucket(context, i, size);
        pthread_create(&context->threads[i], NULL, call_fragment_bucket, &context->fragment_buckets[i]);
    }
    for (size_t i = 0; i < N_THREADS; i++) pthread_join(context->threads[i], NULL);


    SDL_UpdateTexture(context->screen_texture, NULL, context->color_buffer, context->width * sizeof(RL_Color));
    SDL_RenderCopy(context->renderer, context->screen_texture, NULL, NULL);
}