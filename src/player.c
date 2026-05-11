#include "player.h"
#include "portal.h"
#include "render.h"
#include <SDL2/SDL.h>
#include <math.h>

Player player;

void player_init(double x, double y, double dirX, double dirY,
                 double planeX, double planeY, Cell *cell)
{
    player.x              = x;
    player.y              = y;
    player.dirX           = dirX;
    player.dirY           = dirY;
    player.planeX         = planeX;
    player.planeY         = planeY;
    player.cell           = cell;
    player.prevCell       = NULL;
    player.selectedPortal = -1;
}

void player_rotate(Player *p, double angle) {
    double c = cos(angle), s = sin(angle);

    double oldDirX = p->dirX;
    p->dirX = p->dirX * c - p->dirY * s;
    p->dirY = oldDirX  * s + p->dirY * c;

    double oldPlaneX = p->planeX;
    p->planeX = p->planeX * c - p->planeY * s;
    p->planeY = oldPlaneX  * s + p->planeY * c;
}

int player_is_wall(const Player *p, double x, double y) {
    if (x < 0 || y < 0 || x >= CELL_WIDTH || y >= CELL_HEIGHT) return -1;
    return p->cell->map[(int)y][(int)x] != 0;
}

/* Move the player, handling portal crossings. */
static void player_move(Player *p, double newX, double newY) {
    double margin       = 0.2;
    Cell  *justFromCell = p->prevCell;
    p->prevCell         = NULL;

    int teleported = 0;
    for (int i = 0; i < p->cell->portalCount; i++) {
        Portal *portal = &p->cell->portals[i];
        if (portal->destination == justFromCell) continue;

        double ex, ey, angle;
        if (crosses_portal(portal, p->x, p->y, newX, newY, &ex, &ey, &angle)) {
            double savedDirX   = p->dirX,   savedDirY   = p->dirY;
            double savedPlaneX = p->planeX, savedPlaneY = p->planeY;
            Cell  *savedCell   = p->cell;

            double c = cos(angle), s = sin(angle);
            p->dirX   = savedDirX   * c - savedDirY   * s;
            p->dirY   = savedDirX   * s + savedDirY   * c;
            p->planeX = savedPlaneX * c - savedPlaneY * s;
            p->planeY = savedPlaneX * s + savedPlaneY * c;
            p->cell   = portal->destination;

            if (!player_is_wall(p, ex, ey)) {
                p->x = ex;
                p->y = ey;

                /* Carry remaining movement through the portal. */
                double rmx = (newX - p->x) * cos(angle) - (newY - p->y) * sin(angle);
                double rmy = (newX - p->x) * sin(angle) + (newY - p->y) * cos(angle);
                double ml  = sqrt(rmx * rmx + rmy * rmy);
                if (ml > 1e-9) {
                    rmx /= ml; rmy /= ml;
                    if (player_is_wall(p, p->x + rmx * margin, p->y)) p->x -= rmx * margin * 0.5;
                    if (player_is_wall(p, p->x, p->y + rmy * margin)) p->y -= rmy * margin * 0.5;
                }

                p->prevCell = savedCell;
            } else {
                /* Destination blocked — roll back. */
                p->dirX   = savedDirX;   p->dirY   = savedDirY;
                p->planeX = savedPlaneX; p->planeY = savedPlaneY;
                p->cell   = savedCell;
            }

            teleported = 1;
            break;
        }
    }

    if (!teleported) {
        double moveX = newX - p->x, moveY = newY - p->y;
        double movLen = sqrt(moveX * moveX + moveY * moveY);
        if (movLen > 1e-9) {
            double mdx = moveX / movLen, mdy = moveY / movLen;
            if (!player_is_wall(p, newX + mdx * margin, p->y)) p->x = newX;
            if (!player_is_wall(p, p->x, newY + mdy * margin)) p->y = newY;
        }
    }
}

/* Rotate + relink the selected portal's facing angle. */
static void portal_rotate_selected(Player *p, double delta) {
    Portal *portal = &p->cell->portals[p->selectedPortal];
    portal->facingAngle += delta;

    double midX     = (portal->x0 + portal->x1) / 2.0;
    double midY     = (portal->y0 + portal->y1) / 2.0;
    double segAngle = portal->facingAngle + M_PI / 2.0;
    double ex       = cos(segAngle) * 0.5;
    double ey       = sin(segAngle) * 0.5;
    portal->x0 = midX - ex;
    portal->y0 = midY - ey;
    portal->x1 = midX + ex;
    portal->y1 = midY + ey;

    portal_relink(portal, portal->partner);
}

/* Translate the selected portal and update midpoint anchors. */
static void portal_translate_selected(Player *p, double pmx, double pmy) {
    Portal *portal = &p->cell->portals[p->selectedPortal];

    double midX     = (portal->x0 + portal->x1) / 2.0 + pmx;
    double midY     = (portal->y0 + portal->y1) / 2.0 + pmy;
    double segAngle = portal->facingAngle + M_PI / 2.0;
    double ex       = cos(segAngle) * 0.5;
    double ey       = sin(segAngle) * 0.5;
    portal->x0 = midX - ex;
    portal->y0 = midY - ey;
    portal->x1 = midX + ex;
    portal->y1 = midY + ey;

    /* This portal moved; update both sides' exit anchors. */
    portal->dstMidX         = (portal->partner->x0 + portal->partner->x1) / 2.0;
    portal->dstMidY         = (portal->partner->y0 + portal->partner->y1) / 2.0;
    portal->partner->dstMidX = midX;
    portal->partner->dstMidY = midY;
}

void player_update(Player *p, bool *running, double dt) {
    SDL_Event event;
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
            *running = false;
        if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_TAB) {
            if (p->cell->portalCount > 0)
                p->selectedPortal = (p->selectedPortal + 1) % p->cell->portalCount;
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_F)
            portalFramesEnabled = !portalFramesEnabled;
    }

    /* --- Movement --- */
    double moveSpeed = 2.5 * dt;
    double rotSpeed  = 1.5 * dt;
    double newX = p->x, newY = p->y;

    if (keys[SDL_SCANCODE_W]) { newX += p->dirX * moveSpeed; newY += p->dirY * moveSpeed; }
    if (keys[SDL_SCANCODE_S]) { newX -= p->dirX * moveSpeed; newY -= p->dirY * moveSpeed; }

    if (newX != p->x || newY != p->y)
        player_move(p, newX, newY);

    if (keys[SDL_SCANCODE_D]) player_rotate(p,  rotSpeed);
    if (keys[SDL_SCANCODE_A]) player_rotate(p, -rotSpeed);

    /* --- Portal editor (only when a portal is selected) --- */
    if (p->selectedPortal < 0 || p->selectedPortal >= p->cell->portalCount) return;
    if (!p->cell->portals[p->selectedPortal].partner) return;

    double moveStep = 2.0 * dt;
    double pmx = 0.0, pmy = 0.0;
    if (keys[SDL_SCANCODE_UP])    pmy -= moveStep;
    if (keys[SDL_SCANCODE_DOWN])  pmy += moveStep;
    if (keys[SDL_SCANCODE_LEFT])  pmx -= moveStep;
    if (keys[SDL_SCANCODE_RIGHT]) pmx += moveStep;

    if (keys[SDL_SCANCODE_COMMA])  portal_rotate_selected(p, -1.0 * dt);
    if (keys[SDL_SCANCODE_PERIOD]) portal_rotate_selected(p,  1.0 * dt);

    if (pmx != 0.0 || pmy != 0.0)
        portal_translate_selected(p, pmx, pmy);
}