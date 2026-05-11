#include "cell.h"
#include "portal.h"
#include <string.h>

Cell  cells[MAX_CELLS];
Cell *currentCell;

void init_world(void) {

    /* ------------------------------------------------------------------ */
    /* Cell 0 — Open corridor with same-room loop portal                  */
    /* ------------------------------------------------------------------ */

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

    /* ------------------------------------------------------------------ */
    /* Cell 1 — Maze with same-room false-passage portals                 */
    /*                                                                     */
    /*  1 1 1 1 1 1 1 1 1 1 1                                             */
    /*  1 . . 1 . . . 1 . . 1                                             */
    /*  1 . . 1 . . . . . . 1                                             */
    /*  1 . . . . . 1 . . . 1                                             */
    /*  1 1 1 . . . 1 . . . 1                                             */
    /*  1 . . . 1 . . . . . 1                                             */
    /*  1 . . . 1 . . . . . 1                                             */
    /*  1 1 1 1 1 1 1 1 1 1 1                                             */
    /*                                                                     */
    /*  Portal E: dead-end top-left corridor -> pops out bottom-right     */
    /*  Portal F: mid corridor heading right -> jumps to left side        */
    /* ------------------------------------------------------------------ */

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

    /* Entry portals from cell 0 */
    Portal *pA1 = &cells[1].portals[0];
    portal_init(pA1, &cells[1], 1.5, 5.5, 180);   /* set A — bottom-left open area    */

    Portal *pB1 = &cells[1].portals[1];
    portal_init(pB1, &cells[1], 9.0, 1.5, 90);    /* set B — top-right pocket, 90°    */

    /* Portal set E — false passage:
       Walking north into the top-left dead-end corridor pops you out
       facing east in the bottom-right open area.
       Feels like the corridor bends impossibly.                           */
    Portal *pE0 = &cells[1].portals[2];
    portal_init(pE0, &cells[1], 1.5, 1.5, 270);   /* top-left dead end, facing north  */

    Portal *pE1 = &cells[1].portals[3];
    portal_init(pE1, &cells[1], 8.5, 5.5, 0);     /* bottom-right area, facing east   */

    /* Portal set F — false passage:
       Walking east along the mid corridor hits this portal and jumps
       to the left side of the same corridor, facing the same direction.
       Makes the maze feel much longer than it is.                         */
    Portal *pF0 = &cells[1].portals[4];
    portal_init(pF0, &cells[1], 5.5, 3.5, 0);     /* mid corridor, facing east        */

    Portal *pF1 = &cells[1].portals[5];
    portal_init(pF1, &cells[1], 1.5, 3.5, 0);     /* left side, facing east           */

    /* ------------------------------------------------------------------ */
    /* Cell 2 — L-shaped room                                             */
    /* ------------------------------------------------------------------ */

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

    /* ------------------------------------------------------------------ */
    /* Link portals                                                        */
    /* ------------------------------------------------------------------ */

    portal_link(pA0, pA1);   /* set A: cell 0 left   <-> cell 1 bottom-left      */
    portal_link(pB0, pB1);   /* set B: cell 0 right  <-> cell 1 top-right (90°)  */
    portal_link(pC0, pC1);   /* set C: cell 0 same-room north <-> south loop     */
    portal_link(pD0, pD1);   /* set D: cell 0 center <-> cell 2 left arm         */
    portal_link(pE0, pE1);   /* set E: maze dead-end <-> bottom-right (false passage) */
    portal_link(pF0, pF1);   /* set F: maze mid corridor loop (feels longer)     */

    currentCell = &cells[0];
}