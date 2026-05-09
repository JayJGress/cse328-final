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

void handle_input(bool *running, double dt) {
    SDL_Event event;
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);

    while (SDL_PollEvent(&event))
        if (event.type == SDL_QUIT) *running = false;

    double moveSpeed = 2.5*dt;
    double rotSpeed  = 1.5*dt;
    double margin    = 0.2;

    double newX = posX, newY = posY;

    if (keystate[SDL_SCANCODE_W]) {
        newX = posX + dirX * moveSpeed;
        newY = posY + dirY * moveSpeed;
    }
    if (keystate[SDL_SCANCODE_S]) {
        newX = posX - dirX * moveSpeed;
        newY = posY - dirY * moveSpeed;
    }

    if (newX != posX || newY != posY) {
        int teleported = 0;
        for (int i = 0; i < currentCell->portalCount; i++) {
            double ex, ey, angle;
            if (crosses_portal(&currentCell->portals[i],
                posX, posY, newX, newY,
                &ex, &ey, &angle)) {

                double origDirX = dirX, origDirY = dirY;
                double origPlaneX = planeX, origPlaneY = planeY;
                Cell *origCell = currentCell;

                double c = cos(angle), s = sin(angle);
                dirX   = origDirX*c   - origDirY*s;
                dirY   = origDirX*s   + origDirY*c;
                planeX = origPlaneX*c - origPlaneY*s;
                planeY = origPlaneX*s + origPlaneY*c;

                currentCell = currentCell->portals[i].destination;

                if (!is_wall(ex, ey)) {
                    posX = ex;
                    posY = ey;
                    double c2 = cos(angle), s2 = sin(angle);
                    double rmx = (newX - posX)*c2 - (newY - posY)*s2;
                    double rmy = (newX - posX)*s2 + (newY - posY)*c2;
                    double ml = sqrt(rmx*rmx + rmy*rmy);
                    if (ml > 1e-9) {
                        rmx /= ml; rmy /= ml;
                        if (is_wall(posX + rmx*margin, posY)) posX -= rmx*margin*0.5;
                        if (is_wall(posX, posY + rmy*margin)) posY -= rmy*margin*0.5;
                    }
                } else {
                    dirX = origDirX; dirY = origDirY;
                    planeX = origPlaneX; planeY = origPlaneY;
                    currentCell = origCell;
                }
                teleported = 1;
                break;
            }
        }

        if (!teleported) {
            double moveX = newX - posX;
            double moveY = newY - posY;
            double movLen = sqrt(moveX*moveX + moveY*moveY);
            if (movLen > 1e-9) {
                double mdx = moveX / movLen;
                double mdy = moveY / movLen;
                if (!is_wall(newX + mdx * margin, posY)) posX = newX;
                if (!is_wall(posX, newY + mdy * margin)) posY = newY;
            }

            // Drift check only when not teleporting this frame
            for (int i = 0; i < currentCell->portalCount; i++) {
                Portal *p = &currentCell->portals[i];
                double sx = p->x1 - p->x0, sy = p->y1 - p->y0;
                double segLen = sqrt(sx*sx + sy*sy);
                double nx_normal = sy / segLen;
                double ny_normal = -sx / segLen;

                double dx0 = posX - p->x0, dy0 = posY - p->y0;
                double dist = dx0 * nx_normal + dy0 * ny_normal;

                double tx = sx / segLen, ty = sy / segLen;
                double lateral = dx0 * tx + dy0 * ty;
                double half = segLen / 2.0;

                if (dist < 0.0 && dist > -0.05 && lateral >= -half && lateral <= half) {
                    double ex, ey, angle;
                    double dummy_nx = posX + nx_normal * 0.001;
                    double dummy_ny = posY + ny_normal * 0.001;
                    if (crosses_portal(p, dummy_nx, dummy_ny, posX, posY, &ex, &ey, &angle)) {
                        double origDirX = dirX, origDirY = dirY;
                        double origPlaneX = planeX, origPlaneY = planeY;
                        Cell *origCell = currentCell;

                        double c = cos(angle), s = sin(angle);
                        dirX   = origDirX*c - origDirY*s;
                        dirY   = origDirX*s + origDirY*c;
                        planeX = origPlaneX*c - origPlaneY*s;
                        planeY = origPlaneX*s + origPlaneY*c;

                        currentCell = p->destination;

                        if (!is_wall(ex, ey)) {
                            posX = ex;
                            posY = ey;
                        } else {
                            dirX = origDirX; dirY = origDirY;
                            planeX = origPlaneX; planeY = origPlaneY;
                            currentCell = origCell;
                        }
                    }
                    break;
                }
            }
        }
    }

    // After movement, check if player has drifted past any portal plane
    for (int i = 0; i < currentCell->portalCount; i++) {
        Portal *p = &currentCell->portals[i];
        double sx = p->x1 - p->x0, sy = p->y1 - p->y0;
        double segLen = sqrt(sx*sx + sy*sy);
        double nx_normal = sy / segLen;
        double ny_normal = -sx / segLen;

        // Signed distance from player to portal plane
        double dx0 = posX - p->x0, dy0 = posY - p->y0;
        double dist = dx0 * nx_normal + dy0 * ny_normal;

        // Check player is within the portal's lateral extent
        double tx = sx / segLen, ty = sy / segLen;
        double lateral = dx0 * tx + dy0 * ty;
        double half = segLen / 2.0;

        if (dist < 0.0 && lateral >= -half && lateral <= half) {
            // Player is on the back side of the portal — teleport them
            double ex, ey, angle;
            double dummy_nx = posX + nx_normal * 0.001;
            double dummy_ny = posY + ny_normal * 0.001;
            if (crosses_portal(p, dummy_nx, dummy_ny, posX, posY, &ex, &ey, &angle)) {
                double origDirX = dirX, origDirY = dirY;
                double origPlaneX = planeX, origPlaneY = planeY;
                Cell *origCell = currentCell;

                double c = cos(angle), s = sin(angle);
                dirX   = origDirX*c - origDirY*s;
                dirY   = origDirX*s + origDirY*c;
                planeX = origPlaneX*c - origPlaneY*s;
                planeY = origPlaneX*s + origPlaneY*c;

                currentCell = p->destination;

                if (!is_wall(ex, ey)) {
                    posX = ex;
                    posY = ey;
                } else {
                    dirX = origDirX; dirY = origDirY;
                    planeX = origPlaneX; planeY = origPlaneY;
                    currentCell = origCell;
                }
            }
            break;
        }
    }

    if (keystate[SDL_SCANCODE_D]) rotate(rotSpeed);
    if (keystate[SDL_SCANCODE_A]) rotate(-rotSpeed);
}

double ray_segment_intersect(double rox, double roy, double rdx, double rdy, double x0, double y0, double x1, double y1) {
    double dx = x1 - x0;
    double dy = y1 - y0;

    double denom = rdx * dy - rdy * dx;
    if (fabs(denom) < 1e-10) return -1.0;
    
    double t = ((x0 - rox) * dy - (y0 - roy) * dx) / denom;
    double u = ((x0 - rox) * rdy - (y0 - roy) * rdx) / denom;

    if (t > 1e-4 && u >= 0.0 && u <= 1.0) return t;
    return -1.0;
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
        
        double rayPosX = posX;
        double rayPosY = posY;
        Cell *rayCell = currentCell;
        int hit = 0;
        int oob = 0;
        int side;
        int mapX, mapY;

        int maxPortalJumps = 8;
        int portalJumps = 0;
        
        int stepX = 0, stepY = 0;
        double totalDist = 0.0;

        while (!hit && portalJumps <= maxPortalJumps) {
            mapX = (int)rayPosX;
            mapY = (int)rayPosY;

            double deltaDistX = (rayDirX == 0) ? 1e30 : fabs(1.0/rayDirX);
            double deltaDistY = (rayDirY == 0) ? 1e30 : fabs(1.0/rayDirY);

            double sideDistX, sideDistY; 

            if (rayDirX < 0) {
                stepX = -1;
                sideDistX = (rayPosX - mapX) * deltaDistX;
            } else {
                stepX = 1;
                sideDistX = (mapX + 1.0 - rayPosX) * deltaDistX;
            }

            if (rayDirY < 0) {
                stepY = -1;
                sideDistY = (rayPosY - mapY) * deltaDistY;
            } else {
                stepY = 1;
                sideDistY = (mapY + 1.0 - rayPosY) * deltaDistY;
            }

            double nearestPortalT = 1e30;
            int nearestPortalIdx = -1;

            for (int i = 0; i < rayCell->portalCount; i++) {
                Portal *p = &rayCell->portals[i];
                double t = ray_segment_intersect(rayPosX, rayPosY, rayDirX, rayDirY, p->x0, p->y0, p->x1, p->y1);
                if (t > 0 && t < nearestPortalT) {
                    nearestPortalT = t;
                    nearestPortalIdx = i;
                }
            }

            int portalCrossedThisSegment = 0;

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

                double currentT = (side == 0) 
                    ? (mapX - rayPosX + (1 - stepX) / 2.0) / rayDirX 
                    : (mapY - rayPosY + (1 - stepY) / 2.0) / rayDirY;

                if (nearestPortalIdx >= 0 && nearestPortalT < currentT) {
                    totalDist += nearestPortalT;
                    Portal *p = &rayCell->portals[nearestPortalIdx];

                    // Compute hit point relative to portal's midpoint
                    double hitX = rayPosX + rayDirX * nearestPortalT;
                    double hitY = rayPosY + rayDirY * nearestPortalT;
                    double srcMidX = (p->x0 + p->x1) / 2.0;
                    double srcMidY = (p->y0 + p->y1) / 2.0;

                    // Local offset along the portal segment at the hit point
                    double localOffset = 0.0;
                    {
                        double segX = p->x1 - p->x0;
                        double segY = p->y1 - p->y0;
                        double segLen = sqrt(segX*segX + segY*segY);
                        if (segLen > 1e-9) {
                            double tx = segX / segLen, ty = segY / segLen;
                            localOffset = (hitX - srcMidX) * tx + (hitY - srcMidY) * ty;
                        }
                    }

                    // Rotate the ray direction
                    if (fabs(p->rotationAngle) > 1e-9) {
                        double c = cos(p->rotationAngle);
                        double s = sin(p->rotationAngle);
                        double newDirX = rayDirX * c - rayDirY * s;
                        double newDirY = rayDirX * s + rayDirY * c;
                        rayDirX = newDirX;
                        rayDirY = newDirY;
                    }

                    // Place ray at equivalent position on destination portal
                    // using the rotated segment direction of the destination portal
                    Portal *dst = NULL;
                    for (int j = 0; j < p->destination->portalCount; j++) {
                        if (p->destination->portals[j].destination == rayCell) {
                            dst = &p->destination->portals[j];
                            break;
                        }
                    }

                    if (dst) {
                        double dSegX = dst->x1 - dst->x0;
                        double dSegY = dst->y1 - dst->y0;
                        double dSegLen = sqrt(dSegX*dSegX + dSegY*dSegY);
                        double dtx = (dSegLen > 1e-9) ? dSegX/dSegLen : 1.0;
                        double dty = (dSegLen > 1e-9) ? dSegY/dSegLen : 0.0;

                        // Check if source and destination tangents agree after rotation
                        // If the rotated source tangent opposes dst tangent, flip the offset
                        double c = cos(p->rotationAngle), s = sin(p->rotationAngle);
                        double srcSegX = p->x1 - p->x0;
                        double srcSegY = p->y1 - p->y0;
                        double srcLen  = sqrt(srcSegX*srcSegX + srcSegY*srcSegY);
                        double stx = (srcLen > 1e-9) ? srcSegX/srcLen : 1.0;
                        double sty = (srcLen > 1e-9) ? srcSegY/srcLen : 0.0;
                        double rotStx = stx * c - sty * s;
                        double rotSty = stx * s + sty * c;
                        double dot = rotStx * dtx + rotSty * dty;
                        if (dot < 0) localOffset = -localOffset;  // flip if tangents oppose

                        rayPosX = p->dstMidX + dtx * localOffset;
                        rayPosY = p->dstMidY + dty * localOffset;
                    } else {
                        // Fallback: just teleport to destination midpoint
                        rayPosX = p->dstMidX;
                        rayPosY = p->dstMidY;
                    }
                    double nudge = 1e-6;
                    rayPosX += rayDirX * nudge;
                    rayPosY += rayDirY * nudge;

                    rayCell = p->destination;
                    portalJumps++;
                    portalCrossedThisSegment = 1;
                    break;
                }

                if (mapX < 0 || mapY < 0 || mapX >= CELL_WIDTH || mapY >= CELL_HEIGHT) {
                    hit = 1;
                    oob = 1;
                    break;
                }

                if (rayCell->map[mapY][mapX] != 0) {
                    hit = 1;
                }
            }

            if (!portalCrossedThisSegment) break;
        }

        if (oob) {
            for (int y = 0; y < SCREEN_HEIGHT; y++) {
                pixels[y * SCREEN_WIDTH + x] = (y < SCREEN_HEIGHT / 2) ? 0xFF202030 : 0xFF404040;
            }
            continue;
        }

        // --- Distance ---
        double perpWallDist;
        if (side == 0)
            perpWallDist = totalDist + (mapX - rayPosX + (1 - stepX) / 2.0) / rayDirX;
        else
            perpWallDist = totalDist + (mapY - rayPosY + (1 - stepY) / 2.0) / rayDirY;
        if (perpWallDist < 0.0001) perpWallDist = 0.0001;

        // --- Line height ---
        int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist);

        int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawStart < 0) drawStart = 0;

        int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawEnd >= SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT - 1;

        Uint8 r, g, b;
        switch (rayCell->map[mapY][mapX]) {
            case 1: r = 200; g = 200; b = 200;  break; // grey
            case 2: r = 200; g = 0;   b = 0;    break; // red
            case 3: r = 0;   g = 0;   b = 200;  break; // blue
            case 4: r = 0;   g = 200; b = 0;    break; // green
            case 5: r = 100; g = 100; b = 200;  break; // portal, shouldnt render
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
                case 1: color = 0xFF999999; break;
                case 2: color = 0xFFFF0000; break;
                case 3: color = 0xFF0000FF; break;
                case 4: color = 0xFF00FF00; break;
                case 5: color = 0xFFFFFF00; break;
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

    // --- Portals on minimap ---
    for (int i = 0; i < currentCell->portalCount; i++) {
        Portal *p = &currentCell->portals[i];

        double dx = p->x1 - p->x0;
        double dy = p->y1 - p->y0;
        double len = (dx * dx + dy * dy);
        int steps = (int)(len * tileSize * 4);

        for (int s = 0; s <= steps; s++) {
            double t = (double)s / steps;
            int px = (int)((p->x0 + dx * t) * tileSize);
            int py = (int)((p->y0 + dy * t) * tileSize);
            if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT)
                pixels[py * SCREEN_WIDTH + px] = 0xFFFF00FF;
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
    Uint32 lastTime = SDL_GetTicks();

    while (app.running) {
        Uint32 now = SDL_GetTicks();
        double dt = (now - lastTime) / 1000.0;
        lastTime = now;
        if (dt > 0.05) dt = 0.05;

        handle_input(&app.running, dt);
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