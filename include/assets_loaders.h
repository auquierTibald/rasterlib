#ifndef RASTERLIB_ASSETS_LOADERS_H
#define RASTERLIB_ASSETS_LOADERS_H

#include "typedefs.h"

typedef enum {
    RL_ASSET_TYPE_UNDEFINED,
    RL_ASSET_TYPE_TEXTURE,
    RL_ASSET_TYPE_MESH
} RL_AssetType;

typedef struct {
    const char* source;
    RL_AssetType type;
    void* data;
} RL_Asset;

typedef da(RL_Asset) RL_AssetManager;

void  *RL_LoadAsset(RL_AssetManager *am, const char* filePath, RL_AssetType type);
void RL_UnloadAsset(RL_AssetManager *am, const char* filePath, RL_AssetType type);

RL_Texture *load_texture(const char* filePath);
void free_texture(RL_Texture tex);
RL_Color texture_sample(RL_Texture *tex, vec2 tex_coord);

RL_Mesh *init_mesh();
RL_Mesh *load_mesh(const char* filePath);
void free_mesh(RL_Mesh *mesh);



#endif //RASTERLIB_ASSETS_LOADERS_H