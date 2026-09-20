#include "rasterlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "utils-triangles.h"
#include "utils-matrix.h"

typedef da(near_clip_result) da_near_clip_result;

typedef struct RL_Context_t {
    int width, height;
    float ratio;

    RL_Thread threads[N_THREADS];
    RL_Mutex mutex;

    RL_Color *color_buffer;
    float   *depth_buffer;

    RL_Bucket buckets[N_THREADS];

    da_RL_Triangle input_buffer;
    da_RL_Fragment fragment_buffers[N_THREADS];

    RL_VertexShader vertex_shader;
    RL_FragmentShader fragment_shader;

    RL_Texture *texture;

    RL_AssetManager asset_manager;

    void* user_data;

    RL_ProjectionMode_Kind projection_mode;

    da_near_clip_result near_clip_results;
} RL_Context;

void draw_fragment(RL_Context* context, RL_Fragment *frag) {
    vec3 depths = vec3(frag->tri->pos.a.z, frag->tri->pos.b.z, frag->tri->pos.c.z);
    frag->depth = 1 / dot3(vec3(1 / depths.x, 1 / depths.y, 1 / depths.z), frag->barycentric_coord);


    if(frag->depth >= context->depth_buffer[screen_index(context->width, frag->pos)]) return;

    //TEXTURE COORDINATES INTERPOLATION
    frag->tex_coord = product2(product2( frag->tri->tex.a, 1/depths.x ), frag->barycentric_coord.x);
    frag->tex_coord = sum2(frag->tex_coord, product2(product2( frag->tri->tex.b, 1/depths.y ), frag->barycentric_coord.y));
    frag->tex_coord = sum2(frag->tex_coord, product2(product2( frag->tri->tex.c, 1/depths.z ), frag->barycentric_coord.z));
    frag->tex_coord = product2(frag->tex_coord, frag->depth);

    context->fragment_shader(context, frag, context->user_data);

    context->color_buffer[screen_index(context->width, frag->pos)] = frag->color;
    context->depth_buffer[screen_index(context->width, frag->pos)] = frag->depth;
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
            if ( !(frag.barycentric_coord.x == 0 && frag.barycentric_coord.y == 0 && frag.barycentric_coord.z == 0) ){
                //RL_LockMutex(&context->mutex);
                da_append(&context->fragment_buffers[frag.pos.y / context->height / (N_THREADS-1)], RL_Fragment, frag);
                //RL_UnlockMutex(&context->mutex);
            }
            weights.x += A23;
            weights.y += A31;
            weights.z += A12;
        }

        w1_row += B23;
        w2_row += B31;
        w3_row += B12;

    }

}

void default_vs(struct RL_Context_t *context, RL_Triangle *triangle, void* user_data)
{
    RL_Default_ShaderData data = *(RL_Default_ShaderData*)user_data;
    const matrix mv = mat_mul(data.view, data.model);

    triangle->pos = mat_apply_triangle3(mv, triangle->pos);

    free(mv.data);
}

void default_fs(struct RL_Context_t *context, RL_Fragment *frag, void* user_data) {
    //color = texture_sample(context->texture, tex_coord);
    frag->color = (RL_Color){ .uint16 = 0x0000 };
}


RL_Context* RL_CreateContext(int width, int heigth)
{
    RL_Context* context = malloc(sizeof(RL_Context));
    memset(context, '\0', sizeof(RL_Context));
    context->width = width;
    context->height = heigth;
    context-> ratio = (float)heigth / (float)width;

    context->mutex = RL_CreateMutex();

    context->vertex_shader = default_vs;
    context->fragment_shader = default_fs;

    context->color_buffer = (RL_Color*)malloc(width * heigth * sizeof(RL_Color));
    context->depth_buffer = (float*)malloc(width * heigth * sizeof(float));

    context->input_buffer = (da_RL_Triangle)da_alloc(RL_Triangle, 1);
    for (size_t i = 0; i < N_THREADS; i++) context->fragment_buffers[i] = (da_RL_Fragment)da_alloc(RL_Fragment, 1);

    context->asset_manager = (RL_AssetManager)da_alloc(RL_Asset, 1);
    context->near_clip_results = (da_near_clip_result)da_alloc(near_clip_result, 1);

    context->projection_mode = RL_PROJECTION_MODE_NONE;

    return context;
}

void RL_DestroyContext(RL_Context* context)
{
    if (context->color_buffer) free(context->color_buffer);
    if (context->depth_buffer) free(context->depth_buffer);
    if (context->input_buffer.data) da_free(&(context->input_buffer));
    free(context);
}

void RL_ProjectionMode(RL_Context* context, RL_ProjectionMode_Kind mode)
{
    context->projection_mode = mode;
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

void RL_GetDisplay(RL_Context* context, int *width, int *heigth)
{
    *width = context->width;
    *heigth = context->height;
}

void RL_SetTexture(RL_Context* context, RL_Texture *tex)
{
    context->texture = tex;
}

RL_Texture* RL_GetTexture(RL_Context* context)
{
    return context->texture ;
}

RL_AssetManager* RL_GetAssetManager(RL_Context *context)
{
    return &context->asset_manager;
}

RL_Color* RL_GetColorBuffer(RL_Context *context)
{
    return context->color_buffer;
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
    if (context->input_buffer.data) da_free(&(context->input_buffer));
    context->input_buffer = (da_RL_Triangle){.data = malloc(sizeof(RL_Triangle) * size), .size = size, .capacity = size};
    memcpy(context->input_buffer.data, data, size * sizeof(RL_Triangle));
}

void RL_MeshData(RL_Context* context, RL_Mesh* mesh)
{
    da_clear(&context->input_buffer);
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
            .normal = (triangle3){vn1 ,vn2, vn3},
            .mtl = mesh->i_materials.data[i] == -1 ? NULL : &mesh->materials.data[mesh->i_materials.data[i]]
        };

        da_append(&context->input_buffer, RL_Triangle, tri);
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
    da_range(&context->input_buffer, RL_Triangle, bucket->start, bucket->end)
        context->vertex_shader(context, element, context->user_data);
    return 0;
}

int call_fragment_bucket(void* args) {
    RL_Bucket* bucket = args;
    RL_Context* context = bucket->context;
    da_foreach(&context->fragment_buffers[bucket->start], RL_Fragment) {
        draw_fragment(context, element);
    }
    return 0;
}

void create_and_call_buckets(RL_Context* context, size_t size, RL_ThreadFunction func) {
    for (size_t i = 0; i < N_THREADS; i++) {
        context->buckets[i] = create_bucket(context, i, size);
        context->threads[i] = RL_CreateThread(func, &context->buckets[i]);
    }
    for (size_t i = 0; i < N_THREADS; i++) {
        RL_JoinThread(context->threads[i]);
        RL_DestroyThread(context->threads[i]);
    }
}

void RL_SetShaderData(RL_Context* context, void* data) {
    context->user_data = data;
}

void RL_Draw(RL_Context* context) {
    for (size_t i = 0; i < N_THREADS; i++) da_clear(&context->fragment_buffers[i]);
    da_clear(&context->near_clip_results);

    da_foreach(&context->input_buffer, RL_Triangle) {
        context->vertex_shader(context, element, context->user_data);
        switch (context->projection_mode) {
            default: break;
            case RL_PROJECTION_MODE_NONE: draw_triangle(context, element); break;
            case RL_PROJECTION_MODE_PERSPECTIVE: {
                const near_clip_result result = near_clip_triangle(context, element);
                if ( result.triangles ) {
                    for (size_t i = 0; i < result.triangle_count; i++) {
                        //PROJECT TRIANGLE
                        result.triangles[i].pos = project_triangle(context, result.triangles[i].pos);
                        draw_triangle(context, &result.triangles[i]);
                    }
                    da_append(&context->near_clip_results, near_clip_result, result);
                }
                break;
            }
            case RL_PROJECTION_MODE_ORTHOGRAPHIC: printf("TODO: Orthographic Projection isn't implemented.\n"); exit(1);
        }
    }
    //create_and_call_buckets(context, context->input_buffer.size,   (RL_ThreadFunction) call_vertex_bucket);

    for (size_t i = 0; i < N_THREADS; i++) {
        context->buckets[i] = (RL_Bucket){
            .context = context,
            .start = i,
            .end = 0
        };
        context->threads[i] = RL_CreateThread((RL_ThreadFunction) call_fragment_bucket, &context->buckets[i]);
    }
    for (size_t i = 0; i < N_THREADS; i++) {
        RL_JoinThread(context->threads[i]);
        RL_DestroyThread(context->threads[i]);
    }
    da_foreach(&context->near_clip_results, near_clip_result) free(element->triangles);
}