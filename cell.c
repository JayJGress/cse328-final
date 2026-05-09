#include "cell.h"
#include <string.h>
#include <math.h>

Cell cells[MAX_CELLS];
Cell *currentCell;

double rangle(double angle) {
    return angle * M_PI / 180;
}

void portal_init(Portal *p, Cell *cell, double cx, double cy, double angle) {
    double facingAngle = rangle(angle);
    double segAngle = facingAngle + M_PI / 2.0;
    double half = 0.5;
    double ex = cos(segAngle);
    double ey = sin(segAngle);

    // // Normalize winding so segment always goes in consistent direction
    // if (ex < 0 || (fabs(ex) < 1e-9 && ey < 0)) {
    //     ex = -ex;
    //     ey = -ey;
    // }

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
    a->partner = b;
    b->partner = a;

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

    {
        double aSegX = a->x1 - a->x0, aSegY = a->y1 - a->y0;
        double aLen  = sqrt(aSegX*aSegX + aSegY*aSegY);
        double atx = aSegX/aLen, aty = aSegY/aLen;

        double c = cos(a->rotationAngle), s = sin(a->rotationAngle);
        double rotAtx = atx*c - aty*s;
        double rotAty = atx*s + aty*c;

        double bSegX = b->x1 - b->x0, bSegY = b->y1 - b->y0;
        double bLen  = sqrt(bSegX*bSegX + bSegY*bSegY);
        double btx = bSegX/bLen, bty = bSegY/bLen;

        if (rotAtx*btx + rotAty*bty < 0) {
            // Flip B's segment endpoints so tangents agree
            double tmp;
            tmp = b->x0; b->x0 = b->x1; b->x1 = tmp;
            tmp = b->y0; b->y0 = b->y1; b->y1 = tmp;
        }
    }
}

int crosses_portal(Portal *p,
                   double ox, double oy, double nx, double ny,
                   double *out_x, double *out_y, double *out_angle)
{
    double mx = nx - ox, my = ny - oy;
    double sx = p->x1 - p->x0, sy = p->y1 - p->y0;

    double denom = mx * sy - my * sx;
    if (fabs(denom) < 1e-10) return 0;

    double t = ((p->x0 - ox) * sy - (p->y0 - oy) * sx) / denom;
    double u = ((p->x0 - ox) * my - (p->y0 - oy) * mx) / denom;

    if (t < 0.0 || t > 1.0 || u < 0.0 || u > 1.0) return 0;

    double segLen = sqrt(sx*sx + sy*sy);
    // double nx_normal = sy / segLen;
    // double ny_normal = -sx / segLen;
    // if (mx * nx_normal + my * ny_normal >= 0) return 0;

    double tx = sx / segLen, ty = sy / segLen;
    double hitX = p->x0 + sx * u;
    double hitY = p->y0 + sy * u;
    double midX = (p->x0 + p->x1) / 2.0;
    double midY = (p->y0 + p->y1) / 2.0;
    double localOffset = (hitX - midX) * tx + (hitY - midY) * ty;

    Portal *dst = p->partner;

    if (!dst) {
        *out_x = p->dstMidX;
        *out_y = p->dstMidY;
    } else {
        double dsx = dst->x1 - dst->x0, dsy = dst->y1 - dst->y0;
        double dLen = sqrt(dsx*dsx + dsy*dsy);
        double dtx = dsx/dLen, dty = dsy/dLen;

        *out_x = p->dstMidX + dtx * localOffset;
        *out_y = p->dstMidY + dty * localOffset;
    }

    *out_angle = p->rotationAngle;
    return 1;
}

void init_world() {
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
    portal_init(p0, &cells[0], 2, 4.5, 180);
    Portal *p2 = &cells[0].portals[1];
    portal_init(p2, &cells[0], 10.0, 4.5, 0);
    Portal *p4 = &cells[0].portals[2];
    portal_init(p4, &cells[0], 3.5, 2, 90); // north-facing in room 0
    Portal *p5 = &cells[0].portals[3];
    portal_init(p5, &cells[0], 5, 4.5, 0);  // south-facing in room 0

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
    portal_init(p1, &cells[1], 3, 3.5, 180);
    Portal *p3 = &cells[1].portals[1];
    portal_init(p3, &cells[1], 8, 3.5, 0);

    portal_link(p0, p1);
    portal_link(p2, p3);
    portal_link(p4, p5); // same-room link: north wall to south wall in room 0

    currentCell = &cells[0];
}