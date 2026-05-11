#include "cell.h"
#include "portal.h"
#include <string.h>

Cell  cells[MAX_CELLS];
Cell *currentCell;

void init_world(void) {
    int map0[CELL_HEIGHT][CELL_WIDTH] = {
        {1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,1},
        {1,0,3,0,4,4,0,0,0,0,1},
        {1,0,3,0,4,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0},
        {1,0,2,2,2,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1}
    };
    memcpy(cells[0].map, map0, sizeof(map0));
    cells[0].portalCount = 4;

    Portal *p0 = &cells[0].portals[0];
    portal_init(p0, &cells[0], 2.0,  4.5, 180);
    Portal *p2 = &cells[0].portals[1];
    portal_init(p2, &cells[0], 10.0, 4.5, 0);
    Portal *p4 = &cells[0].portals[2];
    portal_init(p4, &cells[0], 3.5,  2.0, 90);  
    Portal *p5 = &cells[0].portals[3];
    portal_init(p5, &cells[0], 5.0,  4.5, 0);    

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
    cells[1].portalCount = 2;

    Portal *p1 = &cells[1].portals[0];
    portal_init(p1, &cells[1], 3.0, 3.5, 180);
    Portal *p3 = &cells[1].portals[1];
    portal_init(p3, &cells[1], 8.0, 3.5, 0);

    portal_link(p0, p1);            
    portal_link(p2, p3);             
    portal_link(p4, p5);       

    currentCell = &cells[0];
}