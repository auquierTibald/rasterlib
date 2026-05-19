#ifndef RASTERLIB_TYPEDEFS_H
#define RASTERLIB_TYPEDEFS_H

#include <stdint.h>
#include <stddef.h>
#include <utils.h>

//BASIC GEOMETRY TYPES
typedef struct {
    int x, y;
} ivec2;

typedef struct {
    int x, y, z;
} ivec3;

typedef struct {
    float x, y;
} vec2;

typedef struct {
    float x, y ,z;
} vec3;

typedef struct {
    vec2 a, b, c;
} triangle2;

typedef struct {
    vec3 a, b, c;
} triangle3;

typedef struct
{
    float* data;
    int rows, cols;
} matrix;

//RENDERING TYPES

typedef struct RL_Context_t;

typedef struct {
    triangle3 pos;
    triangle2 tex;
    triangle3 normal;
} RL_Triangle;

typedef struct {
    struct RL_Context_t *context;
    size_t start, end;
} RL_Bucket;

typedef void (*RL_Shader)(struct RL_Context_t* context, RL_Triangle *triangle);

#define RL_VERTEX_SHADER 0
#define RL_FRAGMENT_SHADER 1

//TEXTURE TYPES
typedef struct { uint8_t a, r, g, b; } RL_Color_argb;

typedef uint32_t RL_Color_uint32;

typedef union {
    RL_Color_argb argb;
    RL_Color_uint32 uint32;
} RL_Color;

typedef struct {
    unsigned char* pixels;
    int w, h, comps;
} RL_Texture;

//MESH TYPES
typedef da(vec3) da_vec3;
typedef da(vec2) da_vec2;
typedef da(ivec3) da_ivec3;

typedef struct {
    da_vec3 v_coords;
    da_vec2 v_tex_coords;
    da_vec3 v_normals;

    da_ivec3 i_coords;
    da_ivec3 i_tex_coords;
    da_ivec3 i_normals;

} RL_Mesh;

typedef struct {
    RL_Mesh *mesh;
    RL_Texture *tex;
    RL_Shader vertex_shader, fragment_shader;
    matrix model_matrix;
} RL_Model;

#endif