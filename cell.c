#include "cell.h"
#include <string.h>
#include <math.h>

Cell cells[MAX_CELLS];
Cell *currentCell;

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
    p->offsetX     = 0.0;
    p->offsetY     = 0.0;
    p->destination = NULL;
}

void portal_link(Portal *a, Portal *b) {
    a->destination = b->cell;
    b->destination = a->cell;

    double aMidX = (a->x0 + a->x1) / 2.0;
    double aMidY = (a->y0 + a->y1) / 2.0;
    double bMidX = (b->x0 + b->x1) / 2.0;
    double bMidY = (b->y0 + b->y1) / 2.0;

    a->offsetX = bMidX - aMidX;
    a->offsetY = bMidY - aMidY;
    b->offsetX = aMidX - bMidX;
    b->offsetY = aMidY - bMidY;

    // Ray rotation delta: when crossing a into b, the ray turns by the
    // difference in facing angles + 180 (exit out the back of b)
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
    portal_init(p1, &cells[1], 0.0, 3.5, M_PI);

    portal_link(p0, p1);
    currentCell = &cells[0];
}