#ifndef PORTAL_H
#define PORTAL_H

#include "cell.h"

double rangle(double angle);
void   portal_init(Portal *p, Cell *cell, double cx, double cy, double angle);
void   portal_relink(Portal *a, Portal *b);
void   portal_link(Portal *a, Portal *b);
int    crosses_portal(Portal *p,
                      double ox, double oy, double nx, double ny,
                      double *out_x, double *out_y, double *out_angle);
double ray_segment_intersect(double rox, double roy, double rdx, double rdy,
                             double x0, double y0, double x1, double y1);

#endif