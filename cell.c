#include "cell.h"
#include <string.h>
 
Cell cells[MAX_CELLS];
Cell *currentCell;
 
void init_world() {
    // --- Cell 0 ---
    int map0[CELL_HEIGHT][CELL_WIDTH] = {
        {0,0,0,0,0,0,0,1,0,0,0},
        {0,0,0,0,0,0,0,0,0,2,0},
        {0,0,3,0,1,1,0,0,0,2,0},
        {0,0,3,0,1,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0},
        {0,0,2,2,2,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0}
    };
    memcpy(cells[0].map, map0, sizeof(map0));
    cells[0].portalCount = 0;
 
    currentCell = &cells[0];
}
 
// Returns the portal at a given tile in a cell, or NULL if none
Portal *get_portal(Cell *cell, int tileX, int tileY) {
    for (int i = 0; i < cell->portalCount; i++) {
        if (cell->portals[i].tileX == tileX &&
            cell->portals[i].tileY == tileY) {
            return &cell->portals[i];
        }
    }
    return NULL;
}
 
