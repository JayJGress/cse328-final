#include "render.h"
#include "portal.h"
#include <math.h>
#include <string.h>

int portalFramesEnabled = 0;

/* ------------------------------------------------------------------ */
/* Colour helpers                                                       */
/* ------------------------------------------------------------------ */

static Uint32 wall_color(int tile, int side, double dist) {
    Uint8 r, g, b;
    switch (tile) {
        case 1:  r = 200; g = 200; b = 200; break; /* grey  */
        case 2:  r = 200; g =   0; b =   0; break; /* red   */
        case 3:  r =   0; g =   0; b = 200; break; /* blue  */
        case 4:  r =   0; g = 200; b =   0; break; /* green */
        default: r = 200; g = 200; b = 200; break;
    }

    if (side == 1) { r = (Uint8)(r * 0.7); g = (Uint8)(g * 0.7); b = (Uint8)(b * 0.7); }

    double brightness = 1.0 / (dist * dist * 0.02 + 1.0);
    if (brightness < 0.2) brightness = 0.2;

    r = (Uint8)(r * brightness);
    g = (Uint8)(g * brightness);
    b = (Uint8)(b * brightness);

    return (255u << 24) | ((Uint32)r << 16) | ((Uint32)g << 8) | b;
}

/* ------------------------------------------------------------------ */
/* Minimap                                                              */
/* ------------------------------------------------------------------ */

static void draw_minimap(Uint32 *pixels, const Player *p) {
    const int tileSize = 8;

    /* Tiles */
    for (int y = 0; y < CELL_HEIGHT; y++) {
        for (int x = 0; x < CELL_WIDTH; x++) {
            Uint32 color;
            switch (p->cell->map[y][x]) {
                case 1:  color = 0xFF999999; break;
                case 2:  color = 0xFFFF0000; break;
                case 3:  color = 0xFF0000FF; break;
                case 4:  color = 0xFF00FF00; break;
                case 5:  color = 0xFFFFFF00; break;
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

    /* Portal lines (only when debug overlay is on) */
    if (portalFramesEnabled) {
        for (int i = 0; i < p->cell->portalCount; i++) {
            Portal *portal = &p->cell->portals[i];
            double dx  = portal->x1 - portal->x0;
            double dy  = portal->y1 - portal->y0;
            int    steps = (int)((dx*dx + dy*dy) * tileSize * 4);
            Uint32 color = (i == p->selectedPortal) ? 0xFF00FFFF : 0xFFFF00FF;

            for (int s = 0; s <= steps; s++) {
                double t  = (double)s / steps;
                int    px = (int)((portal->x0 + dx * t) * tileSize);
                int    py = (int)((portal->y0 + dy * t) * tileSize);
                if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT)
                    pixels[py * SCREEN_WIDTH + px] = color;
            }
        }
    }

    /* Player dot */
    int playerX = (int)(p->x * tileSize);
    int playerY = (int)(p->y * tileSize);
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) {
            int px = playerX + dx, py = playerY + dy;
            if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT)
                pixels[py * SCREEN_WIDTH + px] = 0xFFFF0000;
        }
    }

    /* Direction line */
    for (int i = 0; i < 10; i++) {
        int dx = (int)((p->x + p->dirX * i * 0.2) * tileSize);
        int dy = (int)((p->y + p->dirY * i * 0.2) * tileSize);
        if (dx >= 0 && dx < SCREEN_WIDTH && dy >= 0 && dy < SCREEN_HEIGHT)
            pixels[dy * SCREEN_WIDTH + dx] = 0xFF00FFFF;
    }
}

/* ------------------------------------------------------------------ */
/* Portal frame overlay                                                 */
/* ------------------------------------------------------------------ */

static void draw_portal_frames(Uint32 *pixels,
                               int columnPortal[SCREEN_WIDTH][16],
                               double columnPortalDist[SCREEN_WIDTH][16],
                               int columnPortalCount[SCREEN_WIDTH])
{
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        for (int d = 0; d < columnPortalCount[x]; d++) {
            int leftEdge  = (x > 0             && columnPortal[x-1][d] != columnPortal[x][d]);
            int rightEdge = (x < SCREEN_WIDTH-1 && columnPortal[x+1][d] != columnPortal[x][d]);

            double dist = columnPortalDist[x][d];
            if (dist < 0.0001) dist = 0.0001;
            int frameHeight = (int)(SCREEN_HEIGHT / dist);
            int frameStart  = SCREEN_HEIGHT / 2 - frameHeight / 2;
            int frameEnd    = SCREEN_HEIGHT / 2 + frameHeight / 2;
            int clampStart  = (frameStart < 0)            ? 0              : frameStart;
            int clampEnd    = (frameEnd >= SCREEN_HEIGHT)  ? SCREEN_HEIGHT-1 : frameEnd;

            if (leftEdge || rightEdge) {
                for (int y = clampStart; y <= clampEnd; y++) {
                    if (y >= frameStart && y <= frameEnd)
                        pixels[y * SCREEN_WIDTH + x] = 0xFFFF00FF;
                }
            }

            if (frameStart > 0)
                pixels[frameStart * SCREEN_WIDTH + x] = 0xFFFF00FF;
            if (frameEnd < SCREEN_HEIGHT - 1)
                pixels[frameEnd   * SCREEN_WIDTH + x] = 0xFFFF00FF;
        }
    }
}

/* ------------------------------------------------------------------ */
/* Main render                                                          */
/* ------------------------------------------------------------------ */

void render(Uint32 *pixels, const Player *p) {
    /* Clear (ceiling + floor) */
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        Uint32 color = (y < SCREEN_HEIGHT / 2) ? 0xFF202030u : 0xFF404040u;
        for (int x = 0; x < SCREEN_WIDTH; x++)
            pixels[y * SCREEN_WIDTH + x] = color;
    }

    int    columnPortal[SCREEN_WIDTH][16];
    double columnPortalDist[SCREEN_WIDTH][16];
    int    columnPortalCount[SCREEN_WIDTH];
    memset(columnPortal,      -1, sizeof(columnPortal));
    memset(columnPortalCount,  0, sizeof(columnPortalCount));

    /* ---- Per-column raycasting ---- */
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        double cameraX  = 2.0 * x / (double)SCREEN_WIDTH - 1.0;
        double rayDirX  = p->dirX + p->planeX * cameraX;
        double rayDirY  = p->dirY + p->planeY * cameraX;

        double rayPosX  = p->x;
        double rayPosY  = p->y;
        Cell  *rayCell  = p->cell;

        int hit = 0, oob = 0, side = 0;
        int mapX = 0, mapY = 0;
        int stepX = 0, stepY = 0;
        int portalJumps    = 0;
        int maxPortalJumps = 16;
        double totalDist   = 0.0;

        while (!hit && portalJumps <= maxPortalJumps) {
            mapX = (int)rayPosX;
            mapY = (int)rayPosY;

            double deltaDistX = (rayDirX == 0.0) ? 1e30 : fabs(1.0 / rayDirX);
            double deltaDistY = (rayDirY == 0.0) ? 1e30 : fabs(1.0 / rayDirY);

            double sideDistX, sideDistY;

            if (rayDirX < 0) { stepX = -1; sideDistX = (rayPosX - mapX)       * deltaDistX; }
            else              { stepX =  1; sideDistX = (mapX + 1.0 - rayPosX) * deltaDistX; }
            if (rayDirY < 0) { stepY = -1; sideDistY = (rayPosY - mapY)       * deltaDistY; }
            else              { stepY =  1; sideDistY = (mapY + 1.0 - rayPosY) * deltaDistY; }

            /* Find nearest portal in this cell segment */
            double nearestPortalT   = 1e30;
            int    nearestPortalIdx = -1;
            for (int i = 0; i < rayCell->portalCount; i++) {
                Portal *portal = &rayCell->portals[i];
                double t = ray_segment_intersect(rayPosX, rayPosY, rayDirX, rayDirY,
                                                 portal->x0, portal->y0,
                                                 portal->x1, portal->y1);
                if (t > 0 && t < nearestPortalT) {
                    nearestPortalT   = t;
                    nearestPortalIdx = i;
                }
            }

            int portalCrossedThisSegment = 0;

            /* DDA loop */
            while (!hit) {
                if (sideDistX < sideDistY) { sideDistX += deltaDistX; mapX += stepX; side = 0; }
                else                        { sideDistY += deltaDistY; mapY += stepY; side = 1; }

                double currentT = (side == 0)
                    ? (mapX - rayPosX + (1 - stepX) / 2.0) / rayDirX
                    : (mapY - rayPosY + (1 - stepY) / 2.0) / rayDirY;

                if (nearestPortalIdx >= 0 && nearestPortalT < currentT) {
                    totalDist += nearestPortalT;
                    Portal *portal = &rayCell->portals[nearestPortalIdx];

                    /* Local offset on the source portal */
                    double hitX      = rayPosX + rayDirX * nearestPortalT;
                    double hitY      = rayPosY + rayDirY * nearestPortalT;
                    double srcMidX   = (portal->x0 + portal->x1) / 2.0;
                    double srcMidY   = (portal->y0 + portal->y1) / 2.0;
                    double localOffset = 0.0;
                    {
                        double segX = portal->x1 - portal->x0;
                        double segY = portal->y1 - portal->y0;
                        double segLen = sqrt(segX*segX + segY*segY);
                        if (segLen > 1e-9) {
                            double tx = segX / segLen, ty = segY / segLen;
                            localOffset = (hitX - srcMidX) * tx + (hitY - srcMidY) * ty;
                        }
                    }

                    /* Rotate ray direction */
                    if (fabs(portal->rotationAngle) > 1e-9) {
                        double c = cos(portal->rotationAngle), s = sin(portal->rotationAngle);
                        double newDirX = rayDirX * c - rayDirY * s;
                        double newDirY = rayDirX * s + rayDirY * c;
                        rayDirX = newDirX;
                        rayDirY = newDirY;
                    }

                    /* Reposition ray at destination portal */
                    Portal *dst = portal->partner;
                    if (dst) {
                        double dSegX   = dst->x1 - dst->x0, dSegY = dst->y1 - dst->y0;
                        double dSegLen = sqrt(dSegX*dSegX + dSegY*dSegY);
                        double dtx     = (dSegLen > 1e-9) ? dSegX/dSegLen : 1.0;
                        double dty     = (dSegLen > 1e-9) ? dSegY/dSegLen : 0.0;
                        rayPosX = portal->dstMidX + dtx * localOffset;
                        rayPosY = portal->dstMidY + dty * localOffset;
                    } else {
                        rayPosX = portal->dstMidX;
                        rayPosY = portal->dstMidY;
                    }

                    /* Nudge past the portal surface to avoid self-intersection */
                    rayPosX += rayDirX * 1e-6;
                    rayPosY += rayDirY * 1e-6;

                    rayCell = portal->destination;
                    portalJumps++;
                    portalCrossedThisSegment = 1;

                    int depth = columnPortalCount[x];
                    if (depth < 16) {
                        columnPortal[x][depth]     = nearestPortalIdx;
                        columnPortalDist[x][depth] = totalDist;
                        columnPortalCount[x]++;
                    }
                    break;
                }

                if (mapX < 0 || mapY < 0 || mapX >= CELL_WIDTH || mapY >= CELL_HEIGHT) {
                    hit = 1; oob = 1; break;
                }

                if (rayCell->map[mapY][mapX] != 0)
                    hit = 1;
            }

            if (!portalCrossedThisSegment) break;
        }

        if (!hit && portalJumps > maxPortalJumps) oob = 1;

        if (oob) {
            for (int y = 0; y < SCREEN_HEIGHT; y++)
                pixels[y * SCREEN_WIDTH + x] = (y < SCREEN_HEIGHT / 2) ? 0xFF202030u : 0xFF404040u;
            continue;
        }

        /* Perspective-correct wall distance */
        double perpWallDist;
        if (side == 0)
            perpWallDist = totalDist + (mapX - rayPosX + (1 - stepX) / 2.0) / rayDirX;
        else
            perpWallDist = totalDist + (mapY - rayPosY + (1 - stepY) / 2.0) / rayDirY;
        if (perpWallDist < 0.0001) perpWallDist = 0.0001;

        int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist);
        int drawStart  = -lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawStart < 0) drawStart = 0;
        int drawEnd    =  lineHeight / 2 + SCREEN_HEIGHT / 2;
        if (drawEnd >= SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT - 1;

        Uint32 color = wall_color(rayCell->map[mapY][mapX], side, perpWallDist);
        for (int y = drawStart; y < drawEnd; y++)
            pixels[y * SCREEN_WIDTH + x] = color;
    }

    if (portalFramesEnabled)
        draw_portal_frames(pixels, columnPortal, columnPortalDist, columnPortalCount);

    draw_minimap(pixels, p);
}