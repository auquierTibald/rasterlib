#include "rasterlib.h"

#include <stdlib.h>
#include "utils-matrix.h"
#include "utils.h"

void default_vs(struct RL_Context_t *context, RL_Triangle *triangle, void* user_data)
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
    SDL_LockMutex(context->mutex);
    da_append(&context->fragment_buffer, RL_Triangle, new);
    SDL_UnlockMutex(context->mutex);
}

void default_fs(struct RL_Context_t *context, RL_Fragment *frag, void* user_data) {
    //color = texture_sample(context->texture, tex_coord);
    frag->color = (RL_Color){.uint16 = frag->depth };
}


RL_Context* RL_CreateContext(int width, int heigth)
{
    RL_Context* context = malloc(sizeof(RL_Context));
    memset(context, '\0', sizeof(RL_Context));
    context->width = width;
    context->height = heigth;
    context-> ratio = (float)heigth / (float)width;

    context->mutex = SDL_CreateMutex();

    context->vertex_shader = default_vs;
    context->fragment_shader = default_fs;

    context->color_buffer = (RL_Color*)malloc(width * heigth * sizeof(RL_Color));
    context->depth_buffer = (float*)malloc(width * heigth * sizeof(float));

    context->vertex_input_buffer = (da_RL_Triangle)da_alloc(RL_Triangle, 1);
    context->fragment_buffer = (da_RL_Triangle)da_alloc(RL_Triangle, 1);
    context->asset_manager = (RL_AssetManager)da_alloc(RL_Asset, 1);

    return context;
}

void RL_DestroyContext(RL_Context* context)
{
    if (context->color_buffer) free(context->color_buffer);
    if (context->depth_buffer) free(context->depth_buffer);
    if (context->vertex_input_buffer.data) da_free(&(context->vertex_input_buffer));
    if (context->fragment_buffer.data) da_free(&(context->fragment_buffer));
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

void draw_fragment(RL_Context* context, RL_Fragment *frag) {
    vec3 depths = vec3(frag->tri->pos.a.z, frag->tri->pos.b.z, frag->tri->pos.c.z);
    frag->depth = 1 / dot3(vec3(1 / depths.x, 1 / depths.y, 1 / depths.z), frag->barycentric_coord);

    SDL_LockMutex(context->mutex);
    if(frag->depth >= context->depth_buffer[screen_index(context->width, frag->pos)]) {
        SDL_UnlockMutex(context->mutex);
        return;
    }
    SDL_UnlockMutex(context->mutex);

    //TEXTURE COORDINATES INTERPOLATION
    frag->tex_coord = product2(product2( frag->tri->tex.a, 1/depths.x ), frag->barycentric_coord.x);
    frag->tex_coord = sum2(frag->tex_coord, product2(product2( frag->tri->tex.b, 1/depths.y ), frag->barycentric_coord.y));
    frag->tex_coord = sum2(frag->tex_coord, product2(product2( frag->tri->tex.c, 1/depths.z ), frag->barycentric_coord.z));
    frag->tex_coord = product2(frag->tex_coord, frag->depth);

    context->fragment_shader(context, frag, context->user_data);

    SDL_LockMutex(context->mutex);
    if(frag->depth >= context->depth_buffer[screen_index(context->width, frag->pos)]) {
        SDL_UnlockMutex(context->mutex);
        return;
    }
    context->color_buffer[screen_index(context->width, frag->pos)] = frag->color;
    context->depth_buffer[screen_index(context->width, frag->pos)] = frag->depth;
    SDL_UnlockMutex(context->mutex);
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

    int A12 = v1.y - v2.y, B12 = v2.x - v1.x;
    int A23 = v2.y - v3.y, B23 = v3.x - v2.x;
    int A31 = v3.y - v1.y, B31 = v1.x - v3.x;

    ivec2 start = ivec2(minx, miny);
    int w1_row = iSignedAreaTriangle(v2, v3, start);
    int w2_row = iSignedAreaTriangle(v3, v1, start);
    int w3_row = iSignedAreaTriangle(v1, v2, start);
    for(int y = miny ; y < maxy ; y++) {

        ivec3 weights = ivec3(w1_row, w2_row, w3_row);

        for(int x = minx ; x < maxx; x++) {
            RL_Fragment frag = {.tri = tri, .pos = ivec2(x, y), .barycentric_coord = barycentric_coordinates(weights)};
            frag.barycentric_coord = pointInTriangle(weights);
            if ( !( frag.barycentric_coord.x == 0 && frag.barycentric_coord.y == 0 && frag.barycentric_coord.z == 0 ) ) draw_fragment(context, &frag);
            weights.x += A23;
            weights.y += A31;
            weights.z += A12;
        }

        w1_row += B23;
        w2_row += B31;
        w3_row += B12;

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
    else bucket.end = (i+1) * size/N_THREADS;
    return bucket;
}

int call_vertex_bucket(void* args) {
    RL_Bucket* bucket = args;
    RL_Context* context = bucket->context;
    da_range(&context->vertex_input_buffer, RL_Triangle, bucket->start, bucket->end)
        context->vertex_shader(context, element, context->user_data);
    return 0;
}

int call_fragment_bucket(void* args) {
    RL_Bucket* bucket = args;
    RL_Context* context = bucket->context;
    da_range(&context->fragment_buffer, RL_Triangle, bucket->start, bucket->end)
        draw_triangle(context, element);
    return 0;
}

void RL_Draw(RL_Context* context) {
    da_clear(&(context->fragment_buffer));
    size_t size = context->vertex_input_buffer.size;

    //CALLING VERTEX SHADERS
    for (size_t i = 0; i < N_THREADS; i++) {
        context->vertex_buckets[i] = create_bucket(context, i, size);
        char name[32] = "vertex";
        name[6] = i;
        name[7] = '\0';
        context->threads[i] = SDL_CreateThread(call_vertex_bucket, name, &context->vertex_buckets[i]);
    }
    for (size_t i = 0; i < N_THREADS; i++) SDL_WaitThread(context->threads[i], NULL);

    size = context->fragment_buffer.size;

    //CALLING FRAGMENT SHADERS
    for (size_t i = 0; i < N_THREADS; i++) {
        context->fragment_buckets[i] = create_bucket(context, i, size);
        char name[32] = "fragment";
        name[8] = i;
        name[9] = '\0';
        context->threads[i] = SDL_CreateThread(call_fragment_bucket, name, &context->fragment_buckets[i]);
    }
    for (size_t i = 0; i < N_THREADS; i++) SDL_WaitThread(context->threads[i], NULL);
}