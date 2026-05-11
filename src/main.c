#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include "cell.h"
#include "player.h"
#include "render.h"

typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    SDL_Texture  *texture;
    bool          running;
} App;

static bool init(App *app) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL init error: %s\n", SDL_GetError());
        return false;
    }

    app->window = SDL_CreateWindow(
        "Raycaster",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!app->window) return false;

    app->renderer = SDL_CreateRenderer(app->window, -1, SDL_RENDERER_ACCELERATED);
    if (!app->renderer) return false;

    app->texture = SDL_CreateTexture(
        app->renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!app->texture) return false;

    app->running = true;
    return true;
}

int main(void) {
    App app;
    if (!init(&app)) return 1;

    init_world();

    player_init(3.5, 3.5, 1.0, 0.0, 0.0, 0.66, currentCell);

    Uint32 pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
    Uint32 lastTime = SDL_GetTicks();

    while (app.running) {
        Uint32 now = SDL_GetTicks();
        double dt  = (now - lastTime) / 1000.0;
        lastTime   = now;
        if (dt > 0.05) dt = 0.05;

        player_update(&player, &app.running, dt);

        render(pixels, &player);

        SDL_UpdateTexture(app.texture, NULL, pixels, SCREEN_WIDTH * sizeof(Uint32));
        SDL_RenderClear(app.renderer);
        SDL_RenderCopy(app.renderer, app.texture, NULL, NULL);
        SDL_RenderPresent(app.renderer);
    }

    SDL_DestroyTexture(app.texture);
    SDL_DestroyRenderer(app.renderer);
    SDL_DestroyWindow(app.window);
    SDL_Quit();
    return 0;
}