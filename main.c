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
    SDL_Window *window  = SDL_CreateWindow("rasterlib", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 900,600, SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Event event;

    RL_Context *context = RL_CreateContext(900, 600, renderer);

    RL_TriangleData(context, triangles, N_TRIANGLES);

    while (true) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                RL_DestroyContext(context);
                SDL_Quit();
                exit(0);
            }
        }
        angle += 0.1f;
        SDL_RenderClear(renderer);
        RL_Clear(context, (RL_Color){.uint32 = 0xFF000000});
        matrix model = mat_id(4);
        mat_rotate_yaw(&model, angle);
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