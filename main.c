#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include "cell.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

double posX = 7.5, posY = 5.5;
double dirX = 0, dirY = -1;
double planeX = 0.66, planeY = 0;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    bool running;
} App;

bool init(App *app) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL init error: %s\n", SDL_GetError());
        return false;
    }

    app->window = SDL_CreateWindow(
        "Raycaster",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        0
    );

    if (!app->window) return false;

    app->renderer = SDL_CreateRenderer(app->window, -1, SDL_RENDERER_ACCELERATED);
    if (!app->renderer) return false;

    app->texture = SDL_CreateTexture(
        app->renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    );

    if (!app->texture) return false;

    app->running = true;
    return true;
}

int is_wall(double x, double y) {
    if (x < 0 || y < 0 || x >= CELL_WIDTH || y >= CELL_HEIGHT) return -1;
    return currentCell->map[(int)y][(int)x] != 0;
}

void rotate(double angle) {
    double oldDirX = dirX;
    dirX = dirX * cos(angle) - dirY * sin(angle);
    dirY = oldDirX * sin(angle) + dirY * cos(angle);

    double oldPlaneX = planeX;
    planeX = planeX * cos(angle) - planeY * sin(angle);
    planeY = oldPlaneX * sin(angle) + planeY * cos(angle);
}

void handle_input(bool *running) {
    SDL_Event event;

    const Uint8 *keystate = SDL_GetKeyboardState(NULL);

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            *running = false;
        }
    }

    double moveSpeed = 0.005;
    double rotSpeed = 0.003;

    if (keystate[SDL_SCANCODE_W]) {
        double newX = posX + dirX * moveSpeed;
        double newY = posY + dirY * moveSpeed;

        if (!is_wall(newX, posY)) posX = newX;
        if (!is_wall(posX, newY)) posY = newY;
    }
    if (keystate[SDL_SCANCODE_S]) {
        double newX = posX - dirX * moveSpeed;
        double newY = posY - dirY * moveSpeed;

        if (!is_wall(newX, posY)) posX = newX;
        if (!is_wall(posX, newY)) posY = newY;
    }

    if (keystate[SDL_SCANCODE_D]) rotate(rotSpeed);
    if (keystate[SDL_SCANCODE_A]) rotate(-rotSpeed);
}

void render(Uint32 *pixels) {
    // --- Clear screen (ceiling + floor) ---
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        Uint32 color = (y < SCREEN_HEIGHT / 2) ? 0xFF202030 : 0xFF404040;
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            pixels[y * SCREEN_WIDTH + x] = color;
        }
    }

    // --- Raycasting ---
    for (int x = 0; x < SCREEN_WIDTH; x++) {

        double cameraX = 2 * x / (double)SCREEN_WIDTH - 1;
        double rayDirX = dirX + planeX * cameraX;
        double rayDirY = dirY + planeY * cameraX;

        int mapX = (int)posX;
        int mapY = (int)posY;

        double deltaDistX = (rayDirX == 0) ? 1e30 : fabs(1 / rayDirX);
        double deltaDistY = (rayDirY == 0) ? 1e30 : fabs(1 / rayDirY);

        double sideDistX, sideDistY;
        int stepX, stepY;

        if (rayDirX < 0) {
            stepX = -1;
            sideDistX = (posX - mapX) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX + 1.0 - posX) * deltaDistX;
        }

        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (posY - mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY + 1.0 - posY) * deltaDistY;
        }

        // --- DDA ---
        int hit = 0;
        int side;

        while (!hit) {
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }

            if (mapX < 0 || mapY < 0 || mapX >= CELL_WIDTH || mapY >= CELL_HEIGHT) {
                hit = 1;
                break;
            }
            if (currentCell->map[mapY][mapX] != 0) hit = 1;
        }

        // --- Distance ---
        double perpWallDist;
        if (side == 0)
            perpWallDist = (mapX - posX + (1 - stepX) / 2.0) / rayDirX;
        else
            perpWallDist = (mapY - posY + (1 - stepY) / 2.0) / rayDirY;
        if (perpWallDist < 0.0001) perpWallDist = 0.0001;

        // --- Line height ---
        int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist);

        int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawStart < 0) drawStart = 0;

        int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawEnd >= SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT - 1;

        Uint8 r, g, b;
        switch (currentCell->map[mapY][mapX]) {
            case 1: r = 0;   g = 200; b = 0;   break; // green
            case 2: r = 200; g = 0;   b = 0;   break; // red
            case 3: r = 0;   g = 0;   b = 200; break; // blue
            default: r = 200; g = 200; b = 200; break;
        }

        if (side == 1) {
            r *= 0.7;
            g *= 0.7;
            b *= 0.7;
        }

        double brightness = 1.0 / (perpWallDist * perpWallDist * 0.02 + 1.0);
        if (brightness < 0.2) brightness = 0.2;

        r = (Uint8)(r * brightness);
        g = (Uint8)(g * brightness);
        b = (Uint8)(b * brightness);

        Uint32 color = (255 << 24) | (r << 16) | (g << 8) | b;

        for (int y = drawStart; y < drawEnd; y++) {
            pixels[y * SCREEN_WIDTH + x] = color;
        }
    }

    // --- Minimap ---
    int tileSize = 8;

    for (int y = 0; y < CELL_HEIGHT; y++) {
        for (int x = 0; x < CELL_WIDTH; x++) {
            Uint32 color;
            switch (currentCell->map[y][x]) {
                case 1: color = 0xFF00FF00; break;
                case 2: color = 0xFFFF0000; break;
                case 3: color = 0xFF0000FF; break;
                default: color = 0xFF000000; break;
            }

            for (int py = 0; py < tileSize; py++) {
                for (int px = 0; px < tileSize; px++) {
                    int sx = x * tileSize + px;
                    int sy = y * tileSize + py;
                    if (sx < SCREEN_WIDTH && sy < SCREEN_HEIGHT)
                        pixels[sy * SCREEN_WIDTH + sx] = color;
                }
            }
        }
    }

    // --- Player on minimap ---
    int playerX = (int)(posX * tileSize);
    int playerY = (int)(posY * tileSize);

    for (int y = -2; y <= 2; y++) {
        for (int x = -2; x <= 2; x++) {
            int px = playerX + x;
            int py = playerY + y;
            if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT)
                pixels[py * SCREEN_WIDTH + px] = 0xFFFF0000;
        }
    }

    // --- Direction line ---
    for (int i = 0; i < 10; i++) {
        int dx = (int)((posX + dirX * i * 0.2) * tileSize);
        int dy = (int)((posY + dirY * i * 0.2) * tileSize);
        if (dx >= 0 && dx < SCREEN_WIDTH && dy >= 0 && dy < SCREEN_HEIGHT)
            pixels[dy * SCREEN_WIDTH + dx] = 0xFF00FFFF;
    }
}

int main() {
    App app;
    if (!init(&app)) return 1;

    init_world();

    Uint32 pixels[SCREEN_WIDTH * SCREEN_HEIGHT];

    while (app.running) {
        handle_input(&app.running);
        render(pixels);

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