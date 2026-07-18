#include "utils.h"

void clamp(int *target, int min, int max)
{
    if(*target > max) {*target = max;}
    else if(*target < min) {*target = min;}
}

int min3(int a, int b, int c) {
    if(a > b)
    {
        if(c > b) return b;
        return c;
    }
    if(c > a) return a;
    return c;
}
int max3(int a, int b, int c) {
    if(a < b)
    {
        if(c < b) return b;
        return c;
    }
    if(c < a) return a;
    return c;

}

vec2 sum2(vec2 target, vec2 value)
{
    vec2 res;
    res.x = target.x + value.x;
    res.y = target.y + value.y;
    return res;
}

vec3 sum3(vec3 target, vec3 value)
{
    vec3 res;
    res.x = target.x + value.x;
    res.y = target.y + value.y;
    res.z = target.z + value.z;
    return res;
}

vec2 product2(vec2 target, float value)
{
    vec2 res = {0};
    res.x = target.x * value;
    res.y = target.y * value;
    return res;
}

vec3 product3(vec3 target, float value)
{
    vec3 res = {0};
    res.x = target.x * value;
    res.y = target.y * value;
    res.z = target.z * value;
    return res;
}

float dot3(vec3 a, vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float dot2(vec2 a, vec2 b)
{
    return a.x * b.x + a.y * b.y;
}


int idot3(ivec3 a, ivec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

int idot2(ivec2 a, ivec2 b)
{
    return a.x * b.x + a.y * b.y;
}

vec3 cross3(vec3 a, vec3 b)
{
    vec3 res;
    res.x = a.y*b.z - a.z*b.y;
    res.y = a.z*b.x - a.x*b.z;
    res.z = a.x*b.y - a.y*b.x;
    return res;
}

vec3 diff3(vec3 target, vec3 value)
{
    vec3 res;
    res.x = target.x - value.x;
    res.y = target.y - value.y;
    res.z = target.z - value.z;
    return res;
}

vec2 perp(vec2 v) { return vec2(v.y, -v.x); }
ivec2 iperp(ivec2 v) { return ivec2(v.y, -v.x); }

int iSignedAreaTriangle(ivec2 a, ivec2 b, ivec2 c)
{
    return (a.y - b.y) * c.x + (b.x - a.x) * c.y + (a.x * b.y - a.y * b.x);
}

vec3 barycentric_coordinates(ivec3 weights) {
    float sum = weights.x + weights.y + weights.z;
    return vec3((float)weights.x / sum, (float)weights.y / sum, (float)weights.z / sum);
}

vec3 pointInTriangle(ivec3 weights)
{
    if (!(weights.x <= 0 && weights.y <= 0 && weights.z <= 0) || (weights.x >= 0 && weights.y >= 0 && weights.z >= 0)) return vec3(0, 0, 0);
    float sum = weights.x + weights.y + weights.z;
    if (sum == 0 ) return vec3(0, 0, 0);

   return vec3((float)weights.x / sum, (float)weights.y / sum, (float)weights.z / sum);
}

float toScreen(float coord, int size)
{
    return (float)size/2 + coord * (float)size/2;
}

vec3 project_vertex(RL_Context* context, vec3 vertex) {
    return (vec3){toScreen(vertex.x / vertex.z * context->ratio, context->width), toScreen(-vertex.y /  vertex.z, context->height), vertex.z};
}

int screen_index(int stride, ivec2 pos) {
    return pos.y * stride + pos.x;
}
