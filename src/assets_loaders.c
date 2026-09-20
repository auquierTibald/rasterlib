#include "assets_loaders.h"

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "stb_image.h"

static char *get_directory(const char *filePath) {
    const char *fileName = strrchr(filePath, '/');
    if (fileName) {
        fileName++;
        const size_t size = strlen(filePath) - strlen(fileName);
        char *res = malloc(size+1);
        strncpy(res, filePath, size);
        res[size] = '\0';
        return res;
    }
    return NULL;
}

static char *cat_directory(const char *dir, const char *fileName) {
    const size_t dir_size = strlen(dir), filename_size = strlen(fileName);
    char *fullPath = malloc (dir_size + filename_size + 1);
    strncpy(fullPath, dir, dir_size);
    fullPath[dir_size] = '\0';
    strncat(fullPath, fileName, filename_size);
    fullPath[dir_size + filename_size] = '\0';
    return fullPath;
}

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
    mesh->materials = (da_RL_Material)da_alloc(RL_Material, 1);

    mesh->i_coords = (da_ivec3)da_alloc(ivec3, 2);
    mesh->i_tex_coords = (da_ivec3)da_alloc(ivec3, 2);
    mesh->i_normals = (da_ivec3)da_alloc(ivec3, 2);
    mesh->i_materials = (da_ssize_t)da_alloc(ssize_t, 1);

    return mesh;
}

static da_RL_Material parse_mtl(char * fileName, const char *wdir) {
    da_RL_Material materials = da_alloc(RL_Material, 1);
    FILE* fp = NULL;
    if (wdir) {
        char *fullPath = cat_directory(wdir, fileName);
        fp = fopen(fullPath, "r");
        free(fullPath);
    } else fp = fopen(fileName, "r");
    if(fp != NULL)
    {
        char line[2048];
        while (fgets(line, 2048, fp)) {
            if (!strncmp(line, "newmtl ", 7)) {
                RL_Material mat = { .name = malloc(512) };
                sscanf(line, "newmtl %s", mat.name);
                while (fgets(line, 2048, fp)) {
                    if (!strncmp(line, "newmtl ", 7)) {
                        fseek(fp, strlen(line)*-1, SEEK_CUR);
                        break;
                    }
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
                        char tex_name[512];
                        sscanf(line, "map_Kd %s", tex_name);
                        if (!strchr(tex_name, '/')) {
                            char *fullPath = cat_directory(wdir, tex_name);
                            printf("loading texture %s\n", fullPath);
                            mat.texture = load_texture(fullPath);
                            free(fullPath);
                        } else {
                            printf("loading texture %s\n", tex_name);
                            mat.texture = load_texture(tex_name);
                        }
                        if (!mat.texture) printf("caca\n");
                        printf("texture ptr : %p\n", mat.texture);
                    }
                }
                da_append(&materials, RL_Material, mat);
            }
        }
        fclose(fp);
    }
    else
    {

        printf( "unable to read from file : %s\n", fileName);
    }
    return materials;
}

RL_Mesh *load_mesh(const char* filePath)
{
    printf("loading mesh at : %s\n", filePath);
    RL_Mesh *mesh = init_mesh();
    FILE* fp = fopen(filePath, "r");
    if(fp != NULL)
    {
        ssize_t current_mtl_idx = -1;
        bool material = false;
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
                if(material) da_append(&mesh->i_materials, ssize_t, current_mtl_idx);
                else da_append(&mesh->i_materials ,ssize_t, -1);
            }
            else if(!strncmp(line, "mtllib ", 7)) {
                material = true;
                char mat_fileName[512];
                sscanf(line, "mtllib %s", mat_fileName);
                char *dir = get_directory(filePath);
                if (dir) {
                    printf("get_directory: %s\n", dir);
                    mesh->materials = parse_mtl(mat_fileName, dir);
                    free(dir);
                } else mesh->materials = parse_mtl(mat_fileName, NULL);
            }
            else if (!strncmp(line, "usemtl ", 7) && material) {
                char mat_name[512];
                sscanf(line, "usemtl %s", mat_name);
                ssize_t i = 0;
                da_foreach(&mesh->materials, RL_Material) {
                    if (!strcmp(mat_name, element->name)) {
                        current_mtl_idx = i;
                        break;
                    }
                    i++;
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