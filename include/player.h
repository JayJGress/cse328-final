#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include "cell.h"

typedef struct {
    double x, y;
    double dirX, dirY;
    double planeX, planeY;
    Cell  *cell;
    Cell  *prevCell;      /* block return-teleport for one frame */
    int    selectedPortal;
} Player;

extern Player player;

void player_init(double x, double y, double dirX, double dirY,
                 double planeX, double planeY, Cell *cell);
void player_rotate(Player *p, double angle);
int  player_is_wall(const Player *p, double x, double y);
void player_update(Player *p, bool *running, double dt);

#endif