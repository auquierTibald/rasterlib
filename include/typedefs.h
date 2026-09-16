#ifndef RASTERLIB_TYPEDEFS_H
#define RASTERLIB_TYPEDEFS_H

#include <stdint.h>
#include <stddef.h>
#include <utils-da.h>

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

#define ivec2(x, y) (ivec2){(x), (y)}
#define ivec3(x, y, z) (ivec3){(x), (y), (z)}

#define vec2(x, y) (vec2){(x), (y)}
#define vec3(x, y, z) (vec3){(x), (y), (z)}

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

//COLOR UNION TYPE
typedef union {
    struct { unsigned short b:4, g:4, r:4, a:4; } argb;
    uint16_t uint16;
} RL_Color;

//RENDERING TYPES

struct RL_Context_t;

typedef struct {
    triangle3 pos;
    triangle2 tex;
    triangle3 normal;
} RL_Triangle;

typedef struct {
    RL_Triangle *tri;

    ivec2 pos;
    vec2 tex_coord;
    vec3 normal;

    vec3 barycentric_coord;
    float depth;

    RL_Color color;
} RL_Fragment;

typedef struct {
    struct RL_Context_t *context;
    size_t start, end;
} RL_Bucket;

typedef void (*RL_VertexShader)(struct RL_Context_t* context, RL_Triangle *triangle, void* user_data);
typedef void (*RL_FragmentShader)(struct RL_Context_t* context, RL_Fragment *fragment, void* user_data);

//TEXTURE TYPE

typedef struct {
    unsigned char* pixels;
    int w, h, comps;
} RL_Texture;

//MESH TYPES
typedef da(vec3) da_vec3;
typedef da(vec2) da_vec2;
typedef da(ivec3) da_ivec3;

typedef struct {
    char* name;
    vec3 Ka, Kd, Ks, Ke;
    float Ns, Ni, d;
    RL_Texture *texture;
} RL_Material;

typedef struct {
    RL_Material *material;
    size_t idx;
} material_idx;

typedef da(RL_Material) da_RL_Material;
typedef da(material_idx) da_mat_idx;

typedef struct {
    da_vec3 v_coords;
    da_vec2 v_tex_coords;
    da_vec3 v_normals;

    da_ivec3 i_coords;
    da_ivec3 i_tex_coords;
    da_ivec3 i_normals;

    da_RL_Material materials;
    da_idx i_materials;

} RL_Mesh;

typedef struct {
    RL_Mesh *mesh;
    RL_VertexShader vertex_shader, fragment_shader;
    matrix model_matrix;
} RL_Model;

#endif