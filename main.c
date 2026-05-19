#include <stdbool.h>
#include <SDL2/SDL.h>
#include <utils.h>

#include "rasterlib.h"

int main(int argc, char* argv[]) {
    SDL_Window *window  = SDL_CreateWindow("rasterlib", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 900,600, SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Event event;

    RL_Context *context = RL_CreateContext(900, 600, renderer);

    while (true) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                RL_DestroyContext(context);
                SDL_Quit();
                exit(0);
            }
        }
        SDL_RenderClear(renderer);
        RL_Clear(context, (RL_Color){.uint32 = 0xFF8814EF});
        RL_Render(context);
        SDL_RenderPresent(renderer);
    }
    RL_DestroyContext(context);
    SDL_Quit();
    return 0;
}