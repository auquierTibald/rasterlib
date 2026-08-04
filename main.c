#include "SDL2/SDL.h"

#include "rasterlib.h"
#include "stdbool.h"
//#include "utils.h"
#include <stdio.h>

#include "utils-matrix.h"

#undef main

#define N_TRIANGLES 2

void blue_fs(struct RL_Context_t *context, RL_Fragment *frag, void* user_data) {
    frag->color = (RL_Color){.uint16 = 0xF00F};
}

void white_fs(struct RL_Context_t *context, RL_Fragment *frag, void* user_data) {
    frag->color = (RL_Color){.uint16 = 0xFFFF};
}

void shrek_fs(struct RL_Context_t *context, RL_Fragment *frag, void* user_data) {
    frag->color = texture_sample(context->texture, frag->tex_coord);
}

RL_Triangle triangles[N_TRIANGLES] = {
    {.pos = {
        {100, 100, 0},
        {100, -100, 0},
        {-100, -100, 0}
        },
        .tex = {
            {1, 1},
            {1, 0},
            {0, 0}
        }
    },
    {.pos = {
        {-100, -100, 0},
        {-100, 100, 0},
        {100, 100, 0}
        },
        .tex = {
                {0, 0},
                {0, 1},
                {1, 1}
        }
    }

};

int main(int argc, char* argv[]) {
    float angle = 0.0f;
    SDL_Window *window  = SDL_CreateWindow("rasterlib", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1600,900, SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture* screen_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_STREAMING, 1600/2, 900/2);
    SDL_Event event;

    RL_Context *context = RL_CreateContext(1600/2, 900/2);
    RL_Mesh *mesh = RL_LoadAsset(&context->asset_manager, "../Im3dSoftRenderer/assets/shrek.obj", RL_ASSET_TYPE_MESH);
    RL_Texture *tex = RL_LoadAsset(&context->asset_manager, "../Im3dSoftRenderer/assets/textures/shrek_diffuse.png", RL_ASSET_TYPE_TEXTURE);

    int fps = 0;
    Uint32 last_time = SDL_GetTicks();

    while (true) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                RL_DestroyContext(context);
                SDL_Quit();
                exit(0);
            }
        }
        fps++;
        if (SDL_GetTicks() - last_time >= 1000) {
            printf("fps : %d\n", fps);
            last_time = SDL_GetTicks();
            fps = 0;
        }

        angle += 0.05f;
        SDL_RenderClear(renderer);
        RL_Clear(context, (RL_Color){.uint16 = 0xFFFF});

        //SHREK
         RL_MeshData(context, mesh);
         //RL_SetFragmentShader(context, shrek_fs);
         RL_SetTexture(context, tex);
         matrix model = mat_id(4);
         mat_scale(&model, vec3(30, 30, 30));
         mat_rotate_pitch(&model, angle);
         mat_rotate_roll(&model, -0.8f);
         mat_rotate_pitch(&model, angle);
         mat_translate(&model, vec3(0, -200, 300));
         matrix view = mat_id(4);
         RL_SetModelMatrix(context, model);
         RL_SetViewMatrix(context, view);
         RL_Draw(context);


        //BACKGROUND
        RL_TriangleData(context, triangles, N_TRIANGLES);
        //RL_SetFragmentShader(context, blue_fs);
        model = mat_id(4);
        mat_scale(&model, vec3(100, 100, 100));
        mat_translate(&model, vec3(0, 0, 3000));
        RL_SetModelMatrix(context, model);
        RL_Draw(context);



        free(model.data);
        free(view.data);

        SDL_UpdateTexture(screen_texture, NULL, context->color_buffer, context->width * sizeof(RL_Color));
        SDL_RenderCopy(renderer, screen_texture, NULL, NULL);

        SDL_RenderPresent(renderer);
    }
    RL_DestroyContext(context);
    SDL_Quit();
    return 0;
}