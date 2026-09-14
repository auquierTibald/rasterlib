//
// Created by tibald on 20/08/2026.
//

#ifndef RASTERLIB_UTILS_TRIANGLES_H
#define RASTERLIB_UTILS_TRIANGLES_H
#include "typedefs.h"

#include <stdbool.h>

#include "rasterlib.h"

int iSignedAreaTriangle(ivec2 a, ivec2 b, ivec2 c);

vec3 barycentric_coordinates(ivec3 weights);

vec3 pointInTriangle(ivec3 weights);

float toScreen(float coord, int size);
vec3 vertex_to_screen(RL_Context* context, vec3 vertex);
triangle3 project_triangle(RL_Context* context, triangle3 tri);

int screen_index(int stride, ivec2 pos);

typedef struct {
    RL_Triangle *triangles;
    size_t triangle_count;
} near_clip_result;

near_clip_result near_clip_triangle(RL_Context* context,RL_Triangle *triangle);

#endif //RASTERLIB_UTILS_TRIANGLES_H
