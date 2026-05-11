#include "cell.h"
#include "portal.h"
#include <string.h>

Cell  cells[MAX_CELLS];
Cell *currentCell;

void init_world(void) {
    int map0[CELL_HEIGHT][CELL_WIDTH] = {
        {1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,1},
        {1,0,4,0,0,0,0,0,4,0,1},
        {1,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,1},
        {1,0,2,0,0,0,0,0,2,0,1},
        {1,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1}
    };
    memcpy(cells[0].map, map0, sizeof(map0));
    cells[0].portalCount = 5;

    Portal *pA0 = &cells[0].portals[0];
    portal_init(pA0, &cells[0], 2.5, 3.5, 180);

    Portal *pB0 = &cells[0].portals[1];
    portal_init(pB0, &cells[0], 8.5, 3.5, 0);

    Portal *pC0 = &cells[0].portals[2];
    portal_init(pC0, &cells[0], 5.5, 1.5, 270);

    Portal *pC1 = &cells[0].portals[3];
    portal_init(pC1, &cells[0], 5.5, 6.0, 90);

    Portal *pD0 = &cells[0].portals[4];
    portal_init(pD0, &cells[0], 5.5, 3.5, 0);

    int map1[CELL_HEIGHT][CELL_WIDTH] = {
        {1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,1,0,0,0,1,0,0,1},
        {1,0,0,1,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,1,0,0,0,1},
        {1,1,1,0,0,0,1,0,0,0,1},
        {1,0,0,0,1,0,0,0,0,0,1},
        {1,0,0,0,1,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1}
    };
    memcpy(cells[1].map, map1, sizeof(map1));
    cells[1].portalCount = 6;

    Portal *pA1 = &cells[1].portals[0];
    portal_init(pA1, &cells[1], 1.5, 5.5, 180);  

    Portal *pB1 = &cells[1].portals[1];
    portal_init(pB1, &cells[1], 9.0, 1.5, 90);   
    Portal *pE0 = &cells[1].portals[2];
    portal_init(pE0, &cells[1], 1.5, 1.5, 270);  

    Portal *pE1 = &cells[1].portals[3];
    portal_init(pE1, &cells[1], 8.5, 5.5, 0);    

    
    Portal *pF0 = &cells[1].portals[4];
    portal_init(pF0, &cells[1], 5.5, 3.5, 0);     

    Portal *pF1 = &cells[1].portals[5];
    portal_init(pF1, &cells[1], 1.5, 3.5, 0);   

    int map2[CELL_HEIGHT][CELL_WIDTH] = {
        {1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,1,1,1,1,1},
        {1,0,0,0,0,0,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,3,0,0,0,0,3,0,1},
        {1,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1}
    };
    memcpy(cells[2].map, map2, sizeof(map2));
    cells[2].portalCount = 1;

    Portal *pD1 = &cells[2].portals[0];
    portal_init(pD1, &cells[2], 2.5, 4.5, 0);

    portal_link(pA0, pA1);
    portal_link(pB0, pB1);
    portal_link(pC0, pC1);
    portal_link(pD0, pD1);
    portal_link(pE0, pE1);
    portal_link(pF0, pF1);

    currentCell = &cells[0];
}