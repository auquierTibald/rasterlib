#ifndef RASTERLIB_UTILS_H
#define RASTERLIB_UTILS_H

#include "rasterlib.h"
#include "typedefs.h"

void clamp(int *target, int min, int max);
int min3(int a, int b, int c);
int max3(int a, int b, int c);

vec2 sum2(vec2 target, vec2 value);
vec3 sum3(vec3 target, vec3 value);

vec2 product2(vec2 target, float value);
vec3 product3(vec3 target, float value);

float dot2(vec2 a, vec2 b);
float dot3(vec3 a, vec3 b);

int idot2(ivec2 a, ivec2 b);
int idot3(ivec3 a, ivec3 b);

vec3 cross3(vec3 a, vec3 b);
vec3 diff3(vec3 target, vec3 value);

vec2 perp(vec2 v);
ivec2 iperp(ivec2 v);

#endif //RASTERLIB_UTILS_H