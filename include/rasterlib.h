#ifndef RASTERLIB_RASTERLIB_H
#define RASTERLIB_RASTERLIB_H

#include "threads.h"

#include "typedefs.h"
#include "assets_loaders.h"

#define N_THREADS 64
#define FAR_PLANE 1000000

typedef struct {
    matrix model, view;
} RL_Default_ShaderData;

typedef enum {
    RL_PROJECTION_MODE_NONE,
    RL_PROJECTION_MODE_PERSPECTIVE,
    RL_PROJECTION_MODE_ORTHOGRAPHIC,
} RL_ProjectionMode_Kind;

typedef da(RL_Triangle) da_RL_Triangle;
typedef da(RL_Fragment) da_RL_Fragment;

typedef struct RL_Context_t RL_Context;

RL_Context* RL_CreateContext(int width, int heigth);
void RL_DestroyContext(RL_Context* context);

void RL_ProjectionMode(RL_Context* context, RL_ProjectionMode_Kind mode);

void RL_SetDisplay(RL_Context *context, int width, int heigth);
void RL_GetDisplay(RL_Context *context, int *width, int *heigth);

void RL_SetTexture(RL_Context *context, RL_Texture *tex);
RL_Texture* RL_GetTexture(RL_Context *context);

RL_AssetManager* RL_GetAssetManager(RL_Context *context);

RL_Color* RL_GetColorBuffer(RL_Context *context);

void RL_SetVertexShader(RL_Context* context, RL_VertexShader shader);
void RL_SetFragmentShader(RL_Context* context, RL_FragmentShader shader);

void RL_SetShaderData(RL_Context *context, void *data);

void RL_TriangleData(RL_Context* context, RL_Triangle* data, size_t size);
void RL_MeshData(RL_Context* context, RL_Mesh* mesh);

void RL_Pixel(RL_Context *context, int x, int y, RL_Color c);
void RL_Clear(RL_Context *context, RL_Color c);
void RL_Draw(RL_Context* context);


#endif //RASTERLIB_RASTERLIB_H