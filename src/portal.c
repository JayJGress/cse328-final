#include "portal.h"
#include <math.h>
#include <stddef.h>

double rangle(double angle) {
    return angle * M_PI / 180.0;
}

void portal_init(Portal *p, Cell *cell, double cx, double cy, double angle) {
    double facingAngle = rangle(angle);
    double segAngle    = facingAngle + M_PI / 2.0;
    double half        = 0.5;
    double ex          = cos(segAngle);
    double ey          = sin(segAngle);

    p->x0 = cx - ex * half;
    p->y0 = cy - ey * half;
    p->x1 = cx + ex * half;
    p->y1 = cy + ey * half;

    p->facingAngle   = facingAngle;
    p->rotationAngle = 0.0;
    p->cell          = cell;
    p->dstMidX       = 0.0;
    p->dstMidY       = 0.0;
    p->destination   = NULL;
    p->partner       = NULL;
}

/* Recompute rotationAngle for both portals and fix endpoint winding so
   their tangent directions agree after rotation.  Call this any time
   either portal's facingAngle or segment endpoints change. */
void portal_relink(Portal *a, Portal *b) {
    a->rotationAngle = b->facingAngle - a->facingAngle + M_PI;
    b->rotationAngle = a->facingAngle - b->facingAngle + M_PI;

    double aSegX = a->x1 - a->x0, aSegY = a->y1 - a->y0;
    double aLen  = sqrt(aSegX * aSegX + aSegY * aSegY);
    double atx   = aSegX / aLen, aty = aSegY / aLen;

    double c = cos(a->rotationAngle), s = sin(a->rotationAngle);
    double rotAtx = atx * c - aty * s;
    double rotAty = atx * s + aty * c;

    double bSegX = b->x1 - b->x0, bSegY = b->y1 - b->y0;
    double bLen  = sqrt(bSegX * bSegX + bSegY * bSegY);
    double btx   = bSegX / bLen, bty = bSegY / bLen;

    if (rotAtx * btx + rotAty * bty < 0) {
        double tmp;
        tmp = b->x0; b->x0 = b->x1; b->x1 = tmp;
        tmp = b->y0; b->y0 = b->y1; b->y1 = tmp;
    }
}

void portal_link(Portal *a, Portal *b) {
    a->destination = b->cell;
    b->destination = a->cell;
    a->partner     = b;
    b->partner     = a;

    double aMidX = (a->x0 + a->x1) / 2.0;
    double aMidY = (a->y0 + a->y1) / 2.0;
    double bMidX = (b->x0 + b->x1) / 2.0;
    double bMidY = (b->y0 + b->y1) / 2.0;

    /* Each portal stores its partner's midpoint as the exit anchor. */
    a->dstMidX = bMidX;
    a->dstMidY = bMidY;
    b->dstMidX = aMidX;
    b->dstMidY = aMidY;

    portal_relink(a, b);
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

    double segLen    = sqrt(sx * sx + sy * sy);
    double tx        = sx / segLen, ty = sy / segLen;
    double hitX      = p->x0 + sx * u;
    double hitY      = p->y0 + sy * u;
    double midX      = (p->x0 + p->x1) / 2.0;
    double midY      = (p->y0 + p->y1) / 2.0;
    double localOffset = (hitX - midX) * tx + (hitY - midY) * ty;

    Portal *dst = p->partner;

    if (!dst) {
        *out_x = p->dstMidX;
        *out_y = p->dstMidY;
    } else {
        double dsx = dst->x1 - dst->x0, dsy = dst->y1 - dst->y0;
        double dLen = sqrt(dsx * dsx + dsy * dsy);
        double dtx  = dsx / dLen, dty = dsy / dLen;

        *out_x = p->dstMidX + dtx * localOffset;
        *out_y = p->dstMidY + dty * localOffset;
    }

    *out_angle = p->rotationAngle;
    return 1;
}

double ray_segment_intersect(double rox, double roy, double rdx, double rdy,
                             double x0,  double y0,  double x1,  double y1)
{
    double dx = x1 - x0, dy = y1 - y0;
    double denom = rdx * dy - rdy * dx;
    if (fabs(denom) < 1e-10) return -1.0;

    double t = ((x0 - rox) * dy - (y0 - roy) * dx) / denom;
    double u = ((x0 - rox) * rdy - (y0 - roy) * rdx) / denom;

    if (t > 1e-4 && u >= 0.0 && u <= 1.0) return t;
    return -1.0;
}