#include "assets_loaders.h"

#include <string.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stdbool.h>

#include "stb_image.h"

RL_Texture *load_texture(const char* filePath)
{
    RL_Texture *tex = malloc(sizeof(RL_Texture));
    tex->pixels = stbi_load(filePath, &tex->w, &tex->h, &tex->comps, 4);
    return tex;
}

void free_texture(RL_Texture tex)
{
    stbi_image_free(tex.pixels);
    tex.pixels = NULL;
}

RL_Color texture_sample(RL_Texture *tex, vec2 tex_coord)
{
    tex_coord.x *= tex->w-1; tex_coord.x = abs((int)tex_coord.x % tex->w);
    tex_coord.y = tex->h - tex_coord.y * tex->h-1; tex_coord.y = abs((int)tex_coord.y % tex->h);
    int idx = (int)tex_coord.y * tex->w + (int)tex_coord.x;
    idx *= 4;
    RL_Color pix;
    pix.argb.r = (float)tex->pixels[idx] / 256 * 16;
    pix.argb.g = (float)tex->pixels[idx + 1] / 256 * 16;
    pix.argb.b = (float)tex->pixels[idx + 2] / 256 * 16;
    if(tex->comps == 4) pix.argb.a = tex->pixels[idx + 3] / 256 * 16;
    else pix.argb.a = 15;
    return pix;
}

RL_Mesh *init_mesh()
{
    RL_Mesh *mesh = malloc(sizeof(RL_Mesh));

    mesh->v_coords = (da_vec3)da_alloc(vec3, 2);
    mesh->v_tex_coords = (da_vec2)da_alloc(vec2, 2);
    mesh->v_normals = (da_vec3)da_alloc(vec3, 2);

    mesh->i_coords = (da_ivec3)da_alloc(ivec3, 2);
    mesh->i_tex_coords = (da_ivec3)da_alloc(ivec3, 2);
    mesh->i_normals = (da_ivec3)da_alloc(ivec3, 2);

    return mesh;
}

static da_RL_Material parse_mtl(char * filePath) {
    da_RL_Material materials = da_alloc(RL_Material, 1);
    FILE* fp = fopen(filePath, "r");
    if(fp != NULL)
    {
        char line[2048];

        while (fgets(line, 2048, fp)) {
            if (!strncmp(line, "newmtl ", 7)) {
                RL_Material mat = { .name = malloc(512) };
                sscanf(line, "newmtl %s", mat.name);
                while (fgets(line, 2048, fp)) {
                    if (!strncmp(line, "\n", 1)) break;
                    if (!strncmp(line, "Ns ", 3)) {
                        sscanf(line, "Ns %f", &mat.Ns);
                    } else if (!strncmp(line, "Ni ", 3)) {
                        sscanf(line, "Ni %f", &mat.Ni);
                    } else if (!strncmp(line, "d ", 2)) {
                        sscanf(line, "d %f", &mat.d);
                    }

                    else if (!strncmp(line, "Ka ", 3)) {
                        sscanf(line, "Ka %f %f %f", &mat.Ka.x,  &mat.Ka.y,  &mat.Ka.z);
                    } else if (!strncmp(line, "Kd ", 3)) {
                        sscanf(line, "Kd %f %f %f", &mat.Kd.x,  &mat.Kd.y,  &mat.Kd.z);
                    } else if (!strncmp(line, "Ks ", 3)) {
                        sscanf(line, "Ks %f %f %f", &mat.Ks.x,  &mat.Ks.y,  &mat.Ks.z);
                    } else if (!strncmp(line, "Ke ", 3)) {
                        sscanf(line, "Ke %f %f %f", &mat.Ke.x,  &mat.Ke.y,  &mat.Ke.z);
                    }

                    else if (!strncmp(line, "map_Kd ", 7)) {
                        char tex_path[512];
                        sscanf(line, "map_Kd %s", tex_path);
                        mat.texture = load_texture(tex_path);
                    }
                }
                da_append(&materials, RL_Material, mat);
            }
        }
    }
    fclose(fp);
    return materials;
}

RL_Mesh *load_mesh(const char* filePath)
{
    bool material = false;
    RL_Mesh *mesh = init_mesh();
    FILE* fp = fopen(filePath, "r");
    if(fp != NULL)
    {
        char line[2048];

        while (fgets(line, 2048, fp)) {
            if (!strncmp(line, "v ", 2))
            {
                vec3 v;
                sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
                da_append(&mesh->v_coords, vec3, v);
            }
            else if (!strncmp(line, "vt ", 3))
            {
                vec2 vt;
                sscanf(line, "vt %f %f", &vt.x, &vt.y);
                da_append(&mesh->v_tex_coords, vec3, vt);
            }
            else if (!strncmp(line, "vn ", 3))
            {
                vec3 vn;

                sscanf(line, "vn %f %f %f", &vn.x, &vn.y, &vn.z);

                da_append(&mesh->v_normals, vec3, vn);
            }
            else if (!strncmp(line, "f ", 2))
            {

                ivec3 face1, face2, face3;

                if(sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &face1.x, &face2.x, &face3.x, &face1.y, &face2.y, &face3.y, &face1.z, &face2.z, &face3.z) == 1)
                {
                    sscanf(line, "f %d", &face1.x);
                    sscanf(line, "f %*d//%*d %d", &face1.y);
                    sscanf(line, "f %*d//%*d %*d//%*d %d", &face1.z);
                }

                face1 = (ivec3){face1.x-1, face1.y-1, face1.z-1};
                face2 = (ivec3){face2.x-1, face2.y-1, face2.z-1};
                face3 = (ivec3){face3.x-1, face3.y-1, face3.z-1};

                da_append(&mesh->i_coords, vec3, face1);
                da_append(&mesh->i_tex_coords, vec3, face2);
                da_append(&mesh->i_normals, vec3, face3);
            }
            else if(!strncmp(line, "mtllib ", 7)) {
                material = true;
                char mat_path[512];
                sscanf(line, "mtllib %s", mat_path);
                mesh->materials = parse_mtl(mat_path);
            }
            else if (!strncmp(line, "usemtl ", 7) && material) {
                char mat_name[512];
                sscanf(line, "usemtl %s", mat_name);
                da_foreach(&mesh->materials, RL_Material) {

                }
            }
        }
        fclose(fp);
    }
    else
    {

        printf("unable to read from file : %s\n", filePath);
    }
    return mesh;
}

void free_mesh(RL_Mesh *mesh)
{
    da_free(&(mesh->v_coords));
    da_free(&(mesh->v_tex_coords));
    da_free(&(mesh->v_normals));

    da_free(&(mesh->i_coords));
    da_free(&(mesh->i_tex_coords));
    da_free(&(mesh->i_normals));
}


void *RL_LoadAsset(RL_AssetManager *am, const char* filePath, RL_AssetType type)
{
    da_foreach(am, RL_Asset) {
        if (!strcmp(filePath, element->source) && type == element->type) return element->data;
    }
    void* data;
    switch(type) {
        case RL_ASSET_TYPE_MESH:
            data = load_mesh(filePath);
            break;
        case RL_ASSET_TYPE_TEXTURE:
            data = load_texture(filePath);
            break;
        default:
            data = NULL;
            break;
    }
    RL_Asset asset = {.source = filePath, .type = type, .data = data};
    da_append(am, RL_Asset, asset);
    return data;
}

void RL_UnloadAsset(RL_AssetManager *am, const char* filePath, RL_AssetType type) {
    da_foreach(am, RL_Asset) {
        if (!strcmp(filePath, element->source) && type == element->type) {
            switch(type) {
                case RL_ASSET_TYPE_MESH:
                    free_mesh(element->data);
                    break;
                case RL_ASSET_TYPE_TEXTURE:
                    free_texture(*(RL_Texture *)element->data);
                    break;
            }
        }
    }
}