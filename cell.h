#ifndef CELL_H
#define CELL_H

#define MAX_PORTALS 16
#define MAX_CELLS 8
#define CELL_WIDTH 11
#define CELL_HEIGHT 8
 
typedef struct Cell Cell;
 
typedef struct {
    int x, y;
    double width;
    int axis;
    double offsetX, offsetY;
    double angle;
    Cell *destination;
} Portal;
 
struct Cell {
    int map[CELL_HEIGHT][CELL_WIDTH];
    Portal portals[MAX_PORTALS];
    int portalCount;
};
 
extern Cell cells[MAX_CELLS];
extern Cell *currentCell;
 
void init_world();
 
#endif