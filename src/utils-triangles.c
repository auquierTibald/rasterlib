//
// Created by tibald on 20/08/2026.
//
#include "utils-triangles.h"

#include <stdlib.h>


#include "typedefs.h"
#include "utils.h"

int iSignedAreaTriangle(ivec2 a, ivec2 b, ivec2 c)
{
    return (a.y - b.y) * c.x + (b.x - a.x) * c.y + (a.x * b.y - a.y * b.x);
}

vec3 barycentric_coordinates(ivec3 weights) {
    const float sum = weights.x + weights.y + weights.z;
    return vec3((float)weights.x / sum, (float)weights.y / sum, (float)weights.z / sum);
}

vec3 pointInTriangle(ivec3 weights)
{
    if (!(weights.x <= 0 && weights.y <= 0 && weights.z <= 0) || (weights.x >= 0 && weights.y >= 0 && weights.z >= 0)) return vec3(0, 0, 0);
    const float sum = weights.x + weights.y + weights.z;
    if (sum == 0 ) return vec3(0, 0, 0);

    return vec3((float)weights.x / sum, (float)weights.y / sum, (float)weights.z / sum);
}

float toScreen(float coord, int size)
{
    return (float)size/2 + coord * (float)size/2;
}

vec3 vertex_to_screen(RL_Context* context, vec3 vertex) {
    int width, height; RL_GetDisplay(context, &width, &height);
    return (vec3){toScreen(vertex.x, width), toScreen(-vertex.y, height), vertex.z};
}

static vec3 vertex_to_screen_wh(int width, int height, vec3 vertex) {
    return (vec3){toScreen(vertex.x, width), toScreen(vertex.y, height), vertex.z};
}

static vec3 project_vertex_wh(int width, int height, vec3 vertex) {
    const vec3 proj = (vec3){vertex.x / vertex.z, -vertex.y / vertex.z, vertex.z};
    return vertex_to_screen_wh(width, height, proj);
}

triangle3 project_triangle(RL_Context* context, triangle3 tri) {
    int width, height; RL_GetDisplay(context, &width, &height);
    return (triangle3){project_vertex_wh(width, height, tri.a), project_vertex_wh(width, height, tri.b), project_vertex_wh(width, height, tri.c)};
}

int screen_index(int stride, ivec2 pos) {
    return pos.y * stride + pos.x;
}

static vec3 near_clip_line(vec3 v1, vec3 v2) {
    vec3 n = vec3(0, 0, 1);
    float t = 0.01f - dot3(n, v1) / dot3(n, vec3(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z));
    return vec3(v1.x + t * (v2.x-v1.x), v1.y + t * (v2.y-v1.y), v1.z + t * (v2.z-v1.z));
}

near_clip_result near_clip_triangle(RL_Context* context, RL_Triangle* tri) {
    vec3 v1 = tri->pos.a, v2 = tri->pos.b, v3 = tri->pos.c;
    vec3 inside_points[3]; int n_inside_points = 0;
    vec3 outside_points[3]; int n_outside_points = 0;

    bool flip_point = false;

    if (v1.z > 0) inside_points[n_inside_points++] = v1;
    else           outside_points[n_outside_points++] = v1;
    if (v2.z > 0) {
        inside_points[n_inside_points++] = v2;
        flip_point = true;
    }
    else           outside_points[n_outside_points++] = v2;
    if (v3.z > 0) inside_points[n_inside_points++] = v3;
    else           outside_points[n_outside_points++] = v3;

    switch (n_inside_points) {
        default:
            return (near_clip_result){.triangles = NULL, .triangle_count = 0};

        case 1: { // 1 vertex inside, 2 outside : clipped TRIANGLE
            vec3 o1 = near_clip_line(outside_points[0], inside_points[0]), o2 = near_clip_line(outside_points[1], inside_points[0]);
            if (flip_point) tri->pos = (triangle3){inside_points[0], o2, o1};
            else            tri->pos = (triangle3){inside_points[0], o1, o2};

            near_clip_result res = {
                .triangles = malloc(sizeof(RL_Triangle)),
                .triangle_count = 1
            };
            res.triangles[0] = *tri;
            return res;
        }

        case 2: { // 2 vertex inside, 1 outside : clipped QUAD
            vec3 o1 = near_clip_line(outside_points[0], inside_points[0]), o2 = near_clip_line(outside_points[0], inside_points[1]);

            RL_Triangle tri1 = *tri;
            RL_Triangle tri2 = *tri;

            if (flip_point) {
                tri1.pos.a = inside_points[0]; tri1.pos.b = inside_points[1]; tri1.pos.c = o1;
                tri2.pos.a = inside_points[1]; tri2.pos.b = o2; tri2.pos.c = o1;
            } else {
                tri1.pos.a = inside_points[1]; tri1.pos.b = inside_points[0]; tri1.pos.c = o1;
                tri2.pos.a = o1; tri2.pos.b = o2; tri2.pos.c = inside_points[1];
            }

            near_clip_result res = {
                .triangles = malloc(2*sizeof(RL_Triangle)),
                .triangle_count = 2
            };
            res.triangles[0] = tri1;
            res.triangles[1] = tri2;
            return res;
        }
        case 3: { // 3 vertices inside, no clipping
            near_clip_result res = {
                .triangles = malloc(sizeof(RL_Triangle)),
                .triangle_count = 1
            };
            res.triangles[0] = *tri;
            return res;
        }
    }
    return (near_clip_result){.triangles = NULL, .triangle_count = 0};
}