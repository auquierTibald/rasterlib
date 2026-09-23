#include "SDL2/SDL.h"

#include "rasterlib.h"
#include "stdbool.h"
//#include "utils.h"
#include <stdio.h>

#include "utils-matrix.h"

#undef main

#define N_TRIANGLES 2

static vec2 look = vec2(0, 0);
static vec3 pos = vec3(0, 0, 0);
static float speed = 0.5f;
static bool running = true;
static RL_Default_ShaderData shader_data;

void blue_fs(struct RL_Context_t *context, RL_Fragment *frag, void* user_data) {
    frag->color = (RL_Color){.uint16 = 0xF00F};
}

void white_fs(struct RL_Context_t *context, RL_Fragment *frag, void* user_data) {
    frag->color = (RL_Color){.uint16 = 0xFFFF};
}

void tex_fs(struct RL_Context_t *context, RL_Fragment *frag, void* user_data) {
    frag->color = texture_sample(RL_GetTexture(context), frag->tex_coord);
}

void mat_tex_fs(struct RL_Context_t *context, RL_Fragment *frag, void* user_data) {
    if (frag->tri->mtl) {
        if (frag->tri->mtl->texture) frag->color = texture_sample(frag->tri->mtl->texture, frag->tex_coord);
    }
    else {
        frag->color = texture_sample(RL_GetTexture(context), frag->tex_coord);
    }
}

static void update_look(SDL_Window *window) {
    int x, y, w, h;

    SDL_GetMouseState(&x, &y);
    SDL_GetWindowSize(window, &w, &h);

    h /= 2; w /= 2;

    look.x += (float)(x - w) / (float)w;
    look.y += (float)(y - h) / (float)h; if (look.y > M_PI/2) look.y = M_PI/2; if (look.y < -M_PI/2) look.y = -M_PI/2;

    SDL_WarpMouseInWindow(window, w, h);
}

static void update_pos() {
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);

    if (keystate[SDL_SCANCODE_W]) {
        pos.x += sinf(look.x) * speed;
        pos.z += cosf(look.x) * speed;
    } if (keystate[SDL_SCANCODE_S]) {
        pos.x -= sinf(look.x) * speed;
        pos.z -= cosf(look.x) * speed;
    } if (keystate[SDL_SCANCODE_A]) {
        pos.x -= sinf(look.x + M_PI / 2) * speed;
        pos.z -= cosf(look.x + M_PI / 2) * speed;
    } if (keystate[SDL_SCANCODE_D]) {
        pos.x += sinf(look.x + M_PI / 2) * speed;
        pos.z += cosf(look.x + M_PI / 2) * speed;
    } if (keystate[SDL_SCANCODE_SPACE]) {
        pos.y += speed;
    } if (keystate[SDL_SCANCODE_LCTRL]) {
        pos.y -= speed;
    } if (keystate[SDL_SCANCODE_X] || keystate[SDL_SCANCODE_ESCAPE]) {
        running = false;
    }
}

static void update_view(matrix *view, SDL_Window *window) {

    update_look(window);
    update_pos();

    mat_id_no_alloc(view, 4);

    mat_translate_invert(view, pos);
    mat_rotate_pitch(view, -look.x);
    mat_rotate_roll(view, -look.y);

    shader_data.view = *view;

}

RL_Triangle triangles[N_TRIANGLES] = {
    {.pos = {
        {-100, -100, 0},
        {100, -100, 0},
        {100, 100, 0}
        },
        .tex = {
            {0, 0},
            {1, 0},
            {1, 1},
        }
    },
    {.pos = {
        {100, 100, 0},
        {-100, 100, 0},
        {-100, -100, 0}
        },
        .tex = {
                {1, 1},
                {0, 1},
                {0, 0}
        }
    }

};

int main(int argc, char* argv[]) {
    float d = 0.0f;
    SDL_Window *window  = SDL_CreateWindow("rasterlib", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1600,900, SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture *screen_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_STREAMING, 1600/8, 900/8);
    SDL_Event event;

    RL_Context *context = RL_CreateContext(1600/8, 900/8);
    RL_AssetManager *am = RL_GetAssetManager(context);

    RL_Mesh *skybox_mesh = RL_LoadAsset(am, "../Im3dSoftRenderer/assets/sphere.obj", RL_ASSET_TYPE_MESH);
    RL_Texture *skybox_tex = RL_LoadAsset(am, "../Im3dSoftRenderer/assets/textures/skybox2.png", RL_ASSET_TYPE_TEXTURE);

    RL_Mesh *mesh = RL_LoadAsset(am, "../Im3dSoftRenderer/assets/shrek.obj", RL_ASSET_TYPE_MESH);
    RL_Texture *tex = RL_LoadAsset(am, "../Im3dSoftRenderer/assets/textures/shrek_diffuse.png", RL_ASSET_TYPE_TEXTURE);

    RL_Texture *placeholder_tex = RL_LoadAsset(am, "../Im3dSoftRenderer/assets/textures/placeholder.png", RL_ASSET_TYPE_TEXTURE);

    RL_Mesh *doom_map = RL_LoadAsset(am, "../Im3dSoftRenderer/assets/DOOM/DOOM.obj", RL_ASSET_TYPE_MESH);
    RL_Texture *doom_tex = RL_LoadAsset(am, "../Im3dSoftRenderer/assets/textures/doom_diffuse.png", RL_ASSET_TYPE_TEXTURE);

    int fps = 0;
    Uint32 last_time = SDL_GetTicks();

    int width, height; RL_GetDisplay(context, &width, &height);
    matrix view = mat_id(4), model = mat_id(4);

    shader_data = (RL_Default_ShaderData){.view = view};
    RL_SetShaderData(context, &shader_data);
    RL_ProjectionMode(context, RL_PROJECTION_MODE_PERSPECTIVE);

    SDL_SetRelativeMouseMode(SDL_TRUE);

    while (running) {
        //HANDLING SDL EVENTS
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }
        //HANDLING FPS COUNTER
        fps++;
        if (SDL_GetTicks() - last_time >= 1000) {
            printf("fps : %d\n", fps);
            last_time = SDL_GetTicks();
            fps = 0;
        }

        d += 0.1f;
        update_view(&view, window);

        //RENDERING
        SDL_RenderClear(renderer);
        RL_Clear(context, (RL_Color){.uint16 = 0x0000});
            /*
            RL_MeshData(context, doom_map);
            RL_SetFragmentShader(context, mat_tex_fs);
            RL_SetTexture(context, doom_tex);
            mat_id_no_alloc(&model, 4);
            mat_scale(&model, vec3(1000, 1000, 1000));
            shader_data.model = model;
            RL_Draw(context);
            */
            //SHREK
            RL_MeshData(context, mesh);

            RL_SetFragmentShader(context, tex_fs);
            RL_SetTexture(context, tex);

            mat_id_no_alloc(&model, 4);
            mat_rotate_pitch(&model, d);
            shader_data.model = model;
            RL_Draw(context);

            //BACKGROUND
            RL_TriangleData(context, triangles, N_TRIANGLES);
            RL_SetFragmentShader(context, tex_fs);
            RL_SetTexture(context, placeholder_tex);

            mat_id_no_alloc(&model, 4);
            mat_scale(&model, vec3(0.1f, 0.1f, 0.1f));
            mat_translate(&model, vec3(0, 0, 10));

            shader_data.model = model;
            RL_Draw(context);

            //SKYBOX
            RL_MeshData(context, skybox_mesh);
            RL_SetFragmentShader(context, tex_fs);
            RL_SetTexture(context, skybox_tex);
            mat_id_no_alloc(&model, 4);
            mat_scale(&model, vec3(10000, 10000, 10000));
            shader_data.model = model;
            RL_Draw(context);

        //UPDATING SCREEN TEXTURE FROM CONTEXT'S COLOR BUFFER
        SDL_UpdateTexture(screen_texture, NULL, RL_GetColorBuffer(context), width * sizeof(RL_Color));
        SDL_RenderCopy(renderer, screen_texture, NULL, NULL);

        SDL_RenderPresent(renderer);
    }

    free(model.data);
    free(view.data);

    RL_UnloadAsset(am, "../Im3dSoftRenderer/assets/sphere.obj", RL_ASSET_TYPE_MESH);
    RL_UnloadAsset(am, "../Im3dSoftRenderer/assets/textures/skybox2.png", RL_ASSET_TYPE_TEXTURE);

    RL_UnloadAsset(am, "../Im3dSoftRenderer/assets/shrek.obj", RL_ASSET_TYPE_MESH);
    RL_UnloadAsset(am, "../Im3dSoftRenderer/assets/textures/shrek_diffuse.png", RL_ASSET_TYPE_TEXTURE);

    RL_DestroyContext(context);

    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_Quit();

    return 0;
}