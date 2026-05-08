#include "cell.h"
#include <string.h>
#include <math.h>

Cell cells[MAX_CELLS];
Cell *currentCell;

double angle(double angle) {
    return angle * M_PI / 180;
}

void portal_init(Portal *p, Cell *cell, double cx, double cy, double facingAngle) {
    double segAngle = facingAngle + M_PI / 2.0;
    double half = 0.5;
    double ex = cos(segAngle);
    double ey = sin(segAngle);

    // Normalize winding so segment always goes in consistent direction
    if (ex < 0 || (fabs(ex) < 1e-9 && ey < 0)) {
        ex = -ex;
        ey = -ey;
    }

    p->x0 = cx - ex * half;
    p->y0 = cy - ey * half;
    p->x1 = cx + ex * half;
    p->y1 = cy + ey * half;
    p->facingAngle   = facingAngle;
    p->rotationAngle = 0.0;
    p->cell        = cell;
    p->dstMidX     = 0.0;
    p->dstMidY     = 0.0;
    p->destination = NULL;
}

void portal_link(Portal *a, Portal *b) {
    a->destination = b->cell;
    b->destination = a->cell;

    double aMidX = (a->x0 + a->x1) / 2.0;
    double aMidY = (a->y0 + a->y1) / 2.0;
    double bMidX = (b->x0 + b->x1) / 2.0;
    double bMidY = (b->y0 + b->y1) / 2.0;

    // Each portal stores its partner's midpoint as the exit anchor
    a->dstMidX = bMidX;
    a->dstMidY = bMidY;
    b->dstMidX = aMidX;
    b->dstMidY = aMidY;

    // Rotation: re-orient ray from A's facing into B's facing
    // +PI because the ray exits out B's front, which is opposite to B's inward normal
    a->rotationAngle = b->facingAngle - a->facingAngle + M_PI;
    b->rotationAngle = a->facingAngle - b->facingAngle + M_PI;
}

void init_world() {
    int map0[CELL_HEIGHT][CELL_WIDTH] = {
        {1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,1},
        {1,0,3,0,4,4,0,0,0,0,1},
        {1,0,3,0,4,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,1},
        {1,0,2,2,2,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1}
    };
    memcpy(cells[0].map, map0, sizeof(map0));
    cells[0].portalCount = 1;
    Portal *p0 = &cells[0].portals[0];
    portal_init(p0, &cells[0], 9.99, 4.0, 0.0);

    int map1[CELL_HEIGHT][CELL_WIDTH] = {
        {1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,4,0,0,0,3,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,3,0,0,0,2,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1}
    };
    memcpy(cells[1].map, map1, sizeof(map1));
    cells[1].portalCount = 1;
    Portal *p1 = &cells[1].portals[0];
    portal_init(p1, &cells[1], 2.5, 3.5, angle(125));

    portal_link(p0, p1);
    currentCell = &cells[0];
}