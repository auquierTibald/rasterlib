#include <stdbool.h>
#include <SDL2/SDL.h>
#include <utils.h>

#include "rasterlib.h"
#include "utils-matrix.h"

#define N_TRIANGLES 1

RL_Triangle triangles[N_TRIANGLES] = {
    {.pos = {
        {1, 10, 13},
        {10, 1, 13},
        {10, 10, 13}
    }}
};

int main(int argc, char* argv[]) {
    float angle = 0.0f;
    SDL_Window *window  = SDL_CreateWindow("rasterlib", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1600,900, SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Event event;

    RL_Context *context = RL_CreateContext(1600/2, 900/2, renderer);

    RL_Mesh *mesh = RL_LoadAsset(&context->asset_manager, "/home/tibald/CLionProjects/Im3dSoftRenderer/assets/sphere.obj", RL_ASSET_TYPE_MESH);

    RL_MeshData(context, mesh);

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

        angle += 0.1f;
        SDL_RenderClear(renderer);
        RL_Clear(context, (RL_Color){.uint32 = 0xFF000000});

        matrix model = mat_id(4);
        mat_scale(&model, vec3(200, 200, 200));
        mat_rotate_roll(&model, angle);
        mat_rotate_pitch(&model, angle);
        mat_translate(&model, vec3(0, 0, 300));
        matrix view = mat_id(4);

        RL_SetModelMatrix(context, model);
        RL_SetViewMatrix(context, view);

        RL_Render(context);

        free(model.data);
        free(view.data);

        SDL_RenderPresent(renderer);
    }
    RL_DestroyContext(context);
    SDL_Quit();
    return 0;
}