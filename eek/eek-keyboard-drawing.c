/*
 * Copyright (C) 2006 Sergey V. Udaltsov <svu@gnome.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include <math.h>
#include <pango/pangocairo.h>

#include "eek-types.h"

static gdouble
length (gdouble x, gdouble y)
{
    return sqrt (x * x + y * y);
}

static gdouble
point_line_distance (gdouble ax, gdouble ay, gdouble nx, gdouble ny)
{
    return ax * nx + ay * ny;
}

static void
normal_form (gdouble ax, gdouble ay,
	     gdouble bx, gdouble by,
	     gdouble * nx, gdouble * ny, gdouble * d)
{
    gdouble l;

    *nx = by - ay;
    *ny = ax - bx;

    l = length (*nx, *ny);

    *nx /= l;
    *ny /= l;

    *d = point_line_distance (ax, ay, *nx, *ny);
}

static void
inverse (gdouble a, gdouble b, gdouble c, gdouble d,
	 gdouble * e, gdouble * f, gdouble * g, gdouble * h)
{
    gdouble det;

    det = a * d - b * c;

    *e = d / det;
    *f = -b / det;
    *g = -c / det;
    *h = a / det;
}

static void
multiply (gdouble a, gdouble b, gdouble c, gdouble d,
	  gdouble e, gdouble f, gdouble * x, gdouble * y)
{
    *x = a * e + b * f;
    *y = c * e + d * f;
}

static void
intersect (gdouble n1x, gdouble n1y, gdouble d1,
	   gdouble n2x, gdouble n2y, gdouble d2, gdouble * x, gdouble * y)
{
    gdouble e, f, g, h;

    inverse (n1x, n1y, n2x, n2y, &e, &f, &g, &h);
    multiply (e, f, g, h, d1, d2, x, y);
}


/* draw an angle from the current point to b and then to c,
 * with a rounded corner of the given radius.
 */
static void
rounded_corner (cairo_t * cr,
		gdouble bx, gdouble by,
		gdouble cx, gdouble cy, gdouble radius)
{
    gdouble ax, ay;
    gdouble n1x, n1y, d1;
    gdouble n2x, n2y, d2;
    gdouble pd1, pd2;
    gdouble ix, iy;
    gdouble dist1, dist2;
    gdouble nx, ny, d;
    gdouble a1x, a1y, c1x, c1y;
    gdouble phi1, phi2;

    cairo_get_current_point (cr, &ax, &ay);
#ifdef KBDRAW_DEBUG
    printf ("        current point: (%f, %f), radius %f:\n", ax, ay,
            radius);
#endif

    /* make sure radius is not too large */
    dist1 = length (bx - ax, by - ay);
    dist2 = length (cx - bx, cy - by);

    radius = MIN (radius, MIN (dist1, dist2));

    /* construct normal forms of the lines */
    normal_form (ax, ay, bx, by, &n1x, &n1y, &d1);
    normal_form (bx, by, cx, cy, &n2x, &n2y, &d2);

    /* find which side of the line a,b the point c is on */
    if (point_line_distance (cx, cy, n1x, n1y) < d1)
        pd1 = d1 - radius;
    else
        pd1 = d1 + radius;

    /* find which side of the line b,c the point a is on */
    if (point_line_distance (ax, ay, n2x, n2y) < d2)
        pd2 = d2 - radius;
    else
        pd2 = d2 + radius;

    /* intersect the parallels to find the center of the arc */
    intersect (n1x, n1y, pd1, n2x, n2y, pd2, &ix, &iy);

    nx = (bx - ax) / dist1;
    ny = (by - ay) / dist1;
    d = point_line_distance (ix, iy, nx, ny);

    /* a1 is the point on the line a-b where the arc starts */
    intersect (n1x, n1y, d1, nx, ny, d, &a1x, &a1y);

    nx = (cx - bx) / dist2;
    ny = (cy - by) / dist2;
    d = point_line_distance (ix, iy, nx, ny);

    /* c1 is the point on the line b-c where the arc ends */
    intersect (n2x, n2y, d2, nx, ny, d, &c1x, &c1y);

    /* determine the first angle */
    if (a1x - ix == 0)
        phi1 = (a1y - iy > 0) ? M_PI_2 : 3 * M_PI_2;
    else if (a1x - ix > 0)
        phi1 = atan ((a1y - iy) / (a1x - ix));
    else
        phi1 = M_PI + atan ((a1y - iy) / (a1x - ix));

    /* determine the second angle */
    if (c1x - ix == 0)
        phi2 = (c1y - iy > 0) ? M_PI_2 : 3 * M_PI_2;
    else if (c1x - ix > 0)
        phi2 = atan ((c1y - iy) / (c1x - ix));
    else
        phi2 = M_PI + atan ((c1y - iy) / (c1x - ix));

    /* compute the difference between phi2 and phi1 mod 2pi */
    d = phi2 - phi1;
    while (d < 0)
        d += 2 * M_PI;
    while (d > 2 * M_PI)
        d -= 2 * M_PI;

#ifdef KBDRAW_DEBUG
    printf ("        line 1 to: (%f, %f):\n", a1x, a1y);
#endif
    if (!(isnan (a1x) || isnan (a1y)))
        cairo_line_to (cr, a1x, a1y);

    /* pick the short arc from phi1 to phi2 */
    if (d < M_PI)
        cairo_arc (cr, ix, iy, radius, phi1, phi2);
    else
        cairo_arc_negative (cr, ix, iy, radius, phi1, phi2);

#ifdef KBDRAW_DEBUG
    printf ("        line 2 to: (%f, %f):\n", cx, cy);
#endif
    cairo_line_to (cr, cx, cy);
}

/* renamed from rounded_polygon, use EekPoint instead of GdkPoint not
   to depend on GTK+, and exported */
void
_eek_rounded_polygon (cairo_t  *cr,
                      gdouble   radius,
                      EekPoint *points,
                      gint      num_points)
{
    gint i, j;

    cairo_move_to (cr,
                   (gdouble) (points[num_points - 1].x +
                              points[0].x) / 2,
                   (gdouble) (points[num_points - 1].y +
                              points[0].y) / 2);


#ifdef KBDRAW_DEBUG
    printf ("    rounded polygon of radius %f:\n", radius);
#endif
    for (i = 0; i < num_points; i++) {
        j = (i + 1) % num_points;
        rounded_corner (cr, (gdouble) points[i].x,
                        (gdouble) points[i].y,
                        (gdouble) (points[i].x + points[j].x) / 2,
                        (gdouble) (points[i].y + points[j].y) / 2,
                        radius);
#ifdef KBDRAW_DEBUG
        printf ("      corner (%d, %d) -> (%d, %d):\n",
                points[i].x, points[i].y, points[j].x,
                points[j].y);
#endif
    };
    cairo_close_path (cr);
}
