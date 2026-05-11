#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include "player.h"

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

extern int portalFramesEnabled;

void render(Uint32 *pixels, const Player *p);

#endif