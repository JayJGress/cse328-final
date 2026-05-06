#include "cell.h"
#include <string.h>
 
Cell cells[MAX_CELLS];
Cell *currentCell;
 
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
    cells[0].portals[0].x = 10.0;
    cells[0].portals[0].y = 3.0;
    cells[0].portals[0].width = 1.0;
    cells[0].portals[0].axis = 0;
    cells[0].portals[0].offsetX = -10.0; // shift ray from x=10 to x=0
    cells[0].portals[0].offsetY = 0.0;
    cells[0].portals[0].angle = 0.0;
    cells[0].portals[0].destination = &cells[1];

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
    cells[1].portals[0].x = 0.0;
    cells[1].portals[0].y = 3.0;
    cells[1].portals[0].width = 1.0;
    cells[1].portals[0].axis = 0;
    cells[1].portals[0].offsetX = 10.0; // shift ray back from x=0 to x=10
    cells[1].portals[0].offsetY = 0.0;
    cells[1].portals[0].angle = 0.0;
    cells[1].portals[0].destination = &cells[0];

    currentCell = &cells[0];
}