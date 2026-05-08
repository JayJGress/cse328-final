#include "cell.h"
#include <string.h>
#include <math.h>
 
Cell cells[MAX_CELLS];
Cell *currentCell;
 
void portal_init(Portal *p, Cell *cell, double cx, double cy, double angle) {
    double segAngle = angle + M_PI / 2.0;
    double half = 0.5;

    p->x0 = cx - cos(segAngle) * half;
    p->y0 = cy - sin(segAngle) * half;
    p->x1 = cx + cos(segAngle) * half;
    p->y1 = cy + sin(segAngle) * half;

    p->angle = angle;
    p->cell = cell;
    p->offsetX = 0.0;
    p->offsetY = 0.0;
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
    portal_init(p0, &cells[0], 9.99, 5.0, 0.0);

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
    portal_init(p1, &cells[1], 2.0, 3.5, M_PI / 2.0);

    portal_link(p0, p1);
    currentCell = &cells[0];
}