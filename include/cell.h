#ifndef CELL_H
#define CELL_H

#define MAX_PORTALS 16
#define MAX_CELLS   8
#define CELL_WIDTH  11
#define CELL_HEIGHT 8

typedef struct Cell   Cell;
typedef struct Portal Portal;

struct Portal {
    double x0, y0;
    double x1, y1;
    double dstMidX, dstMidY;
    double facingAngle;
    double rotationAngle;
    Cell   *cell;
    Cell   *destination;
    Portal *partner;
};

struct Cell {
    int    map[CELL_HEIGHT][CELL_WIDTH];
    Portal portals[MAX_PORTALS];
    int    portalCount;
};

extern Cell  cells[MAX_CELLS];
extern Cell *currentCell;

void init_world(void);

#endif