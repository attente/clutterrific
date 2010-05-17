/*
 * Clutterrific
 * Copyright (C) 2010 William Hua
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */



#define A      4

#define ROWS  60
#define COLS  80

#define SPACE  4

#define GRID  15

#define TRIM

#ifdef TRIM
  #define OPTIONAL(x)
#else
  #define OPTIONAL(x) x
#endif



#include <math.h>
#include <time.h>

#include <clutter/clutter.h>

#include "clutterrific.h"



typedef struct
{
  gfloat hue;

  gfloat x;

  gfloat y;
}
light;



static gfloat  width;
static gfloat  height;

static gint    rows;
static gint    cols;

static gfloat  grid;

static light  *pixel;



static gint     pack         (gint      i,
                              gint      j);

static void     update_light (light    *l,
                              gdouble   dt);

static void     paint_pixel  (gfloat    x,
                              gfloat    y,
                              gfloat    h,
                              gfloat    l);

static gdouble  get_delta    (void);

static gint     get_width    (gunichar  c);

static gint     get_height   (gunichar  c);

static void     paint_char   (gint      x,
                              gint      y,
                              gunichar  c);

static void     paint_line   (gfloat    x1,
                              gfloat    y1,
                              gfloat    x2,
                              gfloat    y2);

static void     paint_back   (void);

static void     paint_front  (void);

static void     paint        (void);

static gboolean queue        (gpointer  data);



static gint
pack (gint i,
      gint j)
{
  return i * cols + j;
}



static void
update_light (light   *l,
              gdouble  dt)
{
  l->y = l->x + (l->y - l->x) * exp (-A * dt);
}



static void
paint_pixel (gfloat x,
             gfloat y,
             gfloat h,
             gfloat l)
{
  ClutterColor c = { 0, 0, 0, 255 };

  clutter_color_from_hls (&c, h, l, 1);

  cogl_set_source_color4ub (c.red, c.green, c.blue, c.alpha);

  cogl_rectangle (x + (width  - grid + 2) / 2.0,
                  y + (height - grid + 2) / 2.0,
                  x + (width  + grid - 2) / 2.0,
                  y + (height + grid - 2) / 2.0);
}



static gdouble
get_delta (void)
{
  static GTimeVal then = { 0 };

  gdouble  dt = 0;
  GTimeVal now;

  g_get_current_time (&now);

  if (then.tv_sec)
    dt = now.tv_sec - then.tv_sec + 1E-6 * (now.tv_usec - then.tv_usec);

  then = now;

  return dt;
}



static gint
get_width (gunichar c)
{
  switch (c)
  {
    case ':':
      return 4;

    case '0':
      return 8;

    case '1':
      return 4;

    case '2':
      return 8;

    case '3':
      return 8;

    case '4':
      return 8;

    case '5':
      return 8;

    case '6':
      return 8;

    case '7':
      return 8;

    case '8':
      return 8;

    case '9':
      return 8;
  }

  return 0;
}



static gint
get_height (gunichar c)
{
  return 10;
}



static void
paint_char (gint     x,
            gint     y,
            gunichar c)
{
  switch (c)
  {
    case ':':
      pixel[pack (y + 2, x + 1)].x = 0;
      pixel[pack (y + 2, x + 2)].x = 0;
      pixel[pack (y + 3, x + 1)].x = 0;
      pixel[pack (y + 3, x + 2)].x = 0;
      pixel[pack (y + 6, x + 1)].x = 0;
      pixel[pack (y + 6, x + 2)].x = 0;
      pixel[pack (y + 7, x + 1)].x = 0;
      pixel[pack (y + 7, x + 2)].x = 0;

      break;

    case '0':
      pixel[pack (y + 0, x + 2)].x = 0;
      pixel[pack (y + 0, x + 3)].x = 0;
      pixel[pack (y + 0, x + 4)].x = 0;
      pixel[pack (y + 0, x + 5)].x = 0;
      pixel[pack (y + 1, x + 1)].x = 0;
      pixel[pack (y + 1, x + 2)].x = 0;
      pixel[pack (y + 1, x + 3)].x = 0;
      pixel[pack (y + 1, x + 4)].x = 0;
      pixel[pack (y + 1, x + 5)].x = 0;
      pixel[pack (y + 1, x + 6)].x = 0;
      pixel[pack (y + 2, x + 1)].x = 0;
      pixel[pack (y + 2, x + 2)].x = 0;
      pixel[pack (y + 2, x + 5)].x = 0;
      pixel[pack (y + 2, x + 6)].x = 0;
      pixel[pack (y + 3, x + 1)].x = 0;
      pixel[pack (y + 3, x + 2)].x = 0;
      pixel[pack (y + 3, x + 5)].x = 0;
      pixel[pack (y + 3, x + 6)].x = 0;
      pixel[pack (y + 4, x + 1)].x = 0;
      pixel[pack (y + 4, x + 2)].x = 0;
      pixel[pack (y + 4, x + 5)].x = 0;
      pixel[pack (y + 4, x + 6)].x = 0;
      pixel[pack (y + 5, x + 1)].x = 0;
      pixel[pack (y + 5, x + 2)].x = 0;
      pixel[pack (y + 5, x + 5)].x = 0;
      pixel[pack (y + 5, x + 6)].x = 0;
      pixel[pack (y + 6, x + 1)].x = 0;
      pixel[pack (y + 6, x + 2)].x = 0;
      pixel[pack (y + 6, x + 5)].x = 0;
      pixel[pack (y + 6, x + 6)].x = 0;
      pixel[pack (y + 7, x + 1)].x = 0;
      pixel[pack (y + 7, x + 2)].x = 0;
      pixel[pack (y + 7, x + 5)].x = 0;
      pixel[pack (y + 7, x + 6)].x = 0;
      pixel[pack (y + 8, x + 1)].x = 0;
      pixel[pack (y + 8, x + 2)].x = 0;
      pixel[pack (y + 8, x + 3)].x = 0;
      pixel[pack (y + 8, x + 4)].x = 0;
      pixel[pack (y + 8, x + 5)].x = 0;
      pixel[pack (y + 8, x + 6)].x = 0;
      pixel[pack (y + 9, x + 2)].x = 0;
      pixel[pack (y + 9, x + 3)].x = 0;
      pixel[pack (y + 9, x + 4)].x = 0;
      pixel[pack (y + 9, x + 5)].x = 0;

      OPTIONAL (pixel[pack (y + 0, x + 1)].x = 0);
      OPTIONAL (pixel[pack (y + 0, x + 6)].x = 0);
      OPTIONAL (pixel[pack (y + 9, x + 1)].x = 0);
      OPTIONAL (pixel[pack (y + 9, x + 6)].x = 0);

      break;

    case '1':
      pixel[pack (y + 0, x + 1)].x = 0;
      pixel[pack (y + 0, x + 2)].x = 0;
      pixel[pack (y + 1, x + 1)].x = 0;
      pixel[pack (y + 1, x + 2)].x = 0;
      pixel[pack (y + 2, x + 1)].x = 0;
      pixel[pack (y + 2, x + 2)].x = 0;
      pixel[pack (y + 3, x + 1)].x = 0;
      pixel[pack (y + 3, x + 2)].x = 0;
      pixel[pack (y + 4, x + 1)].x = 0;
      pixel[pack (y + 4, x + 2)].x = 0;
      pixel[pack (y + 5, x + 1)].x = 0;
      pixel[pack (y + 5, x + 2)].x = 0;
      pixel[pack (y + 6, x + 1)].x = 0;
      pixel[pack (y + 6, x + 2)].x = 0;
      pixel[pack (y + 7, x + 1)].x = 0;
      pixel[pack (y + 7, x + 2)].x = 0;
      pixel[pack (y + 8, x + 1)].x = 0;
      pixel[pack (y + 8, x + 2)].x = 0;
      pixel[pack (y + 9, x + 1)].x = 0;
      pixel[pack (y + 9, x + 2)].x = 0;

      break;

    case '2':
      pixel[pack (y + 0, x + 1)].x = 0;
      pixel[pack (y + 0, x + 2)].x = 0;
      pixel[pack (y + 0, x + 3)].x = 0;
      pixel[pack (y + 0, x + 4)].x = 0;
      pixel[pack (y + 0, x + 5)].x = 0;
      pixel[pack (y + 1, x + 1)].x = 0;
      pixel[pack (y + 1, x + 2)].x = 0;
      pixel[pack (y + 1, x + 3)].x = 0;
      pixel[pack (y + 1, x + 4)].x = 0;
      pixel[pack (y + 1, x + 5)].x = 0;
      pixel[pack (y + 1, x + 6)].x = 0;
      pixel[pack (y + 2, x + 5)].x = 0;
      pixel[pack (y + 2, x + 6)].x = 0;
      pixel[pack (y + 3, x + 5)].x = 0;
      pixel[pack (y + 3, x + 6)].x = 0;
      pixel[pack (y + 4, x + 2)].x = 0;
      pixel[pack (y + 4, x + 3)].x = 0;
      pixel[pack (y + 4, x + 4)].x = 0;
      pixel[pack (y + 4, x + 5)].x = 0;
      pixel[pack (y + 4, x + 6)].x = 0;
      pixel[pack (y + 5, x + 1)].x = 0;
      pixel[pack (y + 5, x + 2)].x = 0;
      pixel[pack (y + 5, x + 3)].x = 0;
      pixel[pack (y + 5, x + 4)].x = 0;
      pixel[pack (y + 5, x + 5)].x = 0;
      pixel[pack (y + 6, x + 1)].x = 0;
      pixel[pack (y + 6, x + 2)].x = 0;
      pixel[pack (y + 7, x + 1)].x = 0;
      pixel[pack (y + 7, x + 2)].x = 0;
      pixel[pack (y + 8, x + 1)].x = 0;
      pixel[pack (y + 8, x + 2)].x = 0;
      pixel[pack (y + 8, x + 3)].x = 0;
      pixel[pack (y + 8, x + 4)].x = 0;
      pixel[pack (y + 8, x + 5)].x = 0;
      pixel[pack (y + 8, x + 6)].x = 0;
      pixel[pack (y + 9, x + 1)].x = 0;
      pixel[pack (y + 9, x + 2)].x = 0;
      pixel[pack (y + 9, x + 3)].x = 0;
      pixel[pack (y + 9, x + 4)].x = 0;
      pixel[pack (y + 9, x + 5)].x = 0;
      pixel[pack (y + 9, x + 6)].x = 0;

      OPTIONAL (pixel[pack (y + 0, x + 6)].x = 0);
      OPTIONAL (pixel[pack (y + 4, x + 1)].x = 0);
      OPTIONAL (pixel[pack (y + 5, x + 6)].x = 0);

      break;

    case '3':
      pixel[pack (y + 0, x + 1)].x = 0;
      pixel[pack (y + 0, x + 2)].x = 0;
      pixel[pack (y + 0, x + 3)].x = 0;
      pixel[pack (y + 0, x + 4)].x = 0;
      pixel[pack (y + 0, x + 5)].x = 0;
      pixel[pack (y + 1, x + 1)].x = 0;
      pixel[pack (y + 1, x + 2)].x = 0;
      pixel[pack (y + 1, x + 3)].x = 0;
      pixel[pack (y + 1, x + 4)].x = 0;
      pixel[pack (y + 1, x + 5)].x = 0;
      pixel[pack (y + 1, x + 6)].x = 0;
      pixel[pack (y + 2, x + 5)].x = 0;
      pixel[pack (y + 2, x + 6)].x = 0;
      pixel[pack (y + 3, x + 5)].x = 0;
      pixel[pack (y + 3, x + 6)].x = 0;
      pixel[pack (y + 4, x + 3)].x = 0;
      pixel[pack (y + 4, x + 4)].x = 0;
      pixel[pack (y + 4, x + 5)].x = 0;
      pixel[pack (y + 5, x + 3)].x = 0;
      pixel[pack (y + 5, x + 4)].x = 0;
      pixel[pack (y + 5, x + 5)].x = 0;
      pixel[pack (y + 6, x + 5)].x = 0;
      pixel[pack (y + 6, x + 6)].x = 0;
      pixel[pack (y + 7, x + 5)].x = 0;
      pixel[pack (y + 7, x + 6)].x = 0;
      pixel[pack (y + 8, x + 1)].x = 0;
      pixel[pack (y + 8, x + 2)].x = 0;
      pixel[pack (y + 8, x + 3)].x = 0;
      pixel[pack (y + 8, x + 4)].x = 0;
      pixel[pack (y + 8, x + 5)].x = 0;
      pixel[pack (y + 8, x + 6)].x = 0;
      pixel[pack (y + 9, x + 1)].x = 0;
      pixel[pack (y + 9, x + 2)].x = 0;
      pixel[pack (y + 9, x + 3)].x = 0;
      pixel[pack (y + 9, x + 4)].x = 0;
      pixel[pack (y + 9, x + 5)].x = 0;

      OPTIONAL (pixel[pack (y + 0, x + 6)].x = 0);
      OPTIONAL (pixel[pack (y + 4, x + 6)].x = 0);
      OPTIONAL (pixel[pack (y + 5, x + 6)].x = 0);
      OPTIONAL (pixel[pack (y + 9, x + 6)].x = 0);

      break;

    case '4':
      pixel[pack (y + 0, x + 1)].x = 0;
      pixel[pack (y + 0, x + 2)].x = 0;
      pixel[pack (y + 0, x + 5)].x = 0;
      pixel[pack (y + 0, x + 6)].x = 0;
      pixel[pack (y + 1, x + 1)].x = 0;
      pixel[pack (y + 1, x + 2)].x = 0;
      pixel[pack (y + 1, x + 5)].x = 0;
      pixel[pack (y + 1, x + 6)].x = 0;
      pixel[pack (y + 2, x + 1)].x = 0;
      pixel[pack (y + 2, x + 2)].x = 0;
      pixel[pack (y + 2, x + 5)].x = 0;
      pixel[pack (y + 2, x + 6)].x = 0;
      pixel[pack (y + 3, x + 1)].x = 0;
      pixel[pack (y + 3, x + 2)].x = 0;
      pixel[pack (y + 3, x + 5)].x = 0;
      pixel[pack (y + 3, x + 6)].x = 0;
      pixel[pack (y + 4, x + 1)].x = 0;
      pixel[pack (y + 4, x + 2)].x = 0;
      pixel[pack (y + 4, x + 3)].x = 0;
      pixel[pack (y + 4, x + 4)].x = 0;
      pixel[pack (y + 4, x + 5)].x = 0;
      pixel[pack (y + 4, x + 6)].x = 0;
      pixel[pack (y + 5, x + 2)].x = 0;
      pixel[pack (y + 5, x + 3)].x = 0;
      pixel[pack (y + 5, x + 4)].x = 0;
      pixel[pack (y + 5, x + 5)].x = 0;
      pixel[pack (y + 5, x + 6)].x = 0;
      pixel[pack (y + 6, x + 5)].x = 0;
      pixel[pack (y + 6, x + 6)].x = 0;
      pixel[pack (y + 7, x + 5)].x = 0;
      pixel[pack (y + 7, x + 6)].x = 0;
      pixel[pack (y + 8, x + 5)].x = 0;
      pixel[pack (y + 8, x + 6)].x = 0;
      pixel[pack (y + 9, x + 5)].x = 0;
      pixel[pack (y + 9, x + 6)].x = 0;

      OPTIONAL (pixel[pack (y + 5, x + 1)].x = 0);

      break;

    case '5':
      pixel[pack (y + 0, x + 1)].x = 0;
      pixel[pack (y + 0, x + 2)].x = 0;
      pixel[pack (y + 0, x + 3)].x = 0;
      pixel[pack (y + 0, x + 4)].x = 0;
      pixel[pack (y + 0, x + 5)].x = 0;
      pixel[pack (y + 0, x + 6)].x = 0;
      pixel[pack (y + 1, x + 1)].x = 0;
      pixel[pack (y + 1, x + 2)].x = 0;
      pixel[pack (y + 1, x + 3)].x = 0;
      pixel[pack (y + 1, x + 4)].x = 0;
      pixel[pack (y + 1, x + 5)].x = 0;
      pixel[pack (y + 1, x + 6)].x = 0;
      pixel[pack (y + 2, x + 1)].x = 0;
      pixel[pack (y + 2, x + 2)].x = 0;
      pixel[pack (y + 3, x + 1)].x = 0;
      pixel[pack (y + 3, x + 2)].x = 0;
      pixel[pack (y + 4, x + 1)].x = 0;
      pixel[pack (y + 4, x + 2)].x = 0;
      pixel[pack (y + 4, x + 3)].x = 0;
      pixel[pack (y + 4, x + 4)].x = 0;
      pixel[pack (y + 4, x + 5)].x = 0;
      pixel[pack (y + 5, x + 1)].x = 0;
      pixel[pack (y + 5, x + 2)].x = 0;
      pixel[pack (y + 5, x + 3)].x = 0;
      pixel[pack (y + 5, x + 4)].x = 0;
      pixel[pack (y + 5, x + 5)].x = 0;
      pixel[pack (y + 5, x + 6)].x = 0;
      pixel[pack (y + 6, x + 5)].x = 0;
      pixel[pack (y + 6, x + 6)].x = 0;
      pixel[pack (y + 7, x + 5)].x = 0;
      pixel[pack (y + 7, x + 6)].x = 0;
      pixel[pack (y + 8, x + 1)].x = 0;
      pixel[pack (y + 8, x + 2)].x = 0;
      pixel[pack (y + 8, x + 3)].x = 0;
      pixel[pack (y + 8, x + 4)].x = 0;
      pixel[pack (y + 8, x + 5)].x = 0;
      pixel[pack (y + 8, x + 6)].x = 0;
      pixel[pack (y + 9, x + 1)].x = 0;
      pixel[pack (y + 9, x + 2)].x = 0;
      pixel[pack (y + 9, x + 3)].x = 0;
      pixel[pack (y + 9, x + 4)].x = 0;
      pixel[pack (y + 9, x + 5)].x = 0;

      OPTIONAL (pixel[pack (y + 4, x + 6)].x = 0);
      OPTIONAL (pixel[pack (y + 9, x + 6)].x = 0);

      break;

    case '6':
      pixel[pack (y + 0, x + 2)].x = 0;
      pixel[pack (y + 0, x + 3)].x = 0;
      pixel[pack (y + 0, x + 4)].x = 0;
      pixel[pack (y + 0, x + 5)].x = 0;
      pixel[pack (y + 0, x + 6)].x = 0;
      pixel[pack (y + 1, x + 1)].x = 0;
      pixel[pack (y + 1, x + 2)].x = 0;
      pixel[pack (y + 1, x + 3)].x = 0;
      pixel[pack (y + 1, x + 4)].x = 0;
      pixel[pack (y + 1, x + 5)].x = 0;
      pixel[pack (y + 1, x + 6)].x = 0;
      pixel[pack (y + 2, x + 1)].x = 0;
      pixel[pack (y + 2, x + 2)].x = 0;
      pixel[pack (y + 3, x + 1)].x = 0;
      pixel[pack (y + 3, x + 2)].x = 0;
      pixel[pack (y + 4, x + 1)].x = 0;
      pixel[pack (y + 4, x + 2)].x = 0;
      pixel[pack (y + 4, x + 3)].x = 0;
      pixel[pack (y + 4, x + 4)].x = 0;
      pixel[pack (y + 4, x + 5)].x = 0;
      pixel[pack (y + 5, x + 1)].x = 0;
      pixel[pack (y + 5, x + 2)].x = 0;
      pixel[pack (y + 5, x + 3)].x = 0;
      pixel[pack (y + 5, x + 4)].x = 0;
      pixel[pack (y + 5, x + 5)].x = 0;
      pixel[pack (y + 5, x + 6)].x = 0;
      pixel[pack (y + 6, x + 1)].x = 0;
      pixel[pack (y + 6, x + 2)].x = 0;
      pixel[pack (y + 6, x + 5)].x = 0;
      pixel[pack (y + 6, x + 6)].x = 0;
      pixel[pack (y + 7, x + 1)].x = 0;
      pixel[pack (y + 7, x + 2)].x = 0;
      pixel[pack (y + 7, x + 5)].x = 0;
      pixel[pack (y + 7, x + 6)].x = 0;
      pixel[pack (y + 8, x + 1)].x = 0;
      pixel[pack (y + 8, x + 2)].x = 0;
      pixel[pack (y + 8, x + 3)].x = 0;
      pixel[pack (y + 8, x + 4)].x = 0;
      pixel[pack (y + 8, x + 5)].x = 0;
      pixel[pack (y + 8, x + 6)].x = 0;
      pixel[pack (y + 9, x + 2)].x = 0;
      pixel[pack (y + 9, x + 3)].x = 0;
      pixel[pack (y + 9, x + 4)].x = 0;
      pixel[pack (y + 9, x + 5)].x = 0;

      OPTIONAL (pixel[pack (y + 0, x + 1)].x = 0);
      OPTIONAL (pixel[pack (y + 4, x + 6)].x = 0);
      OPTIONAL (pixel[pack (y + 9, x + 1)].x = 0);
      OPTIONAL (pixel[pack (y + 9, x + 6)].x = 0);

      break;

    case '7':
      pixel[pack (y + 0, x + 1)].x = 0;
      pixel[pack (y + 0, x + 2)].x = 0;
      pixel[pack (y + 0, x + 3)].x = 0;
      pixel[pack (y + 0, x + 4)].x = 0;
      pixel[pack (y + 0, x + 5)].x = 0;
      pixel[pack (y + 0, x + 6)].x = 0;
      pixel[pack (y + 1, x + 1)].x = 0;
      pixel[pack (y + 1, x + 2)].x = 0;
      pixel[pack (y + 1, x + 3)].x = 0;
      pixel[pack (y + 1, x + 4)].x = 0;
      pixel[pack (y + 1, x + 5)].x = 0;
      pixel[pack (y + 1, x + 6)].x = 0;
      pixel[pack (y + 2, x + 5)].x = 0;
      pixel[pack (y + 2, x + 6)].x = 0;
      pixel[pack (y + 3, x + 5)].x = 0;
      pixel[pack (y + 3, x + 6)].x = 0;
      pixel[pack (y + 4, x + 5)].x = 0;
      pixel[pack (y + 4, x + 6)].x = 0;
      pixel[pack (y + 5, x + 5)].x = 0;
      pixel[pack (y + 5, x + 6)].x = 0;
      pixel[pack (y + 6, x + 5)].x = 0;
      pixel[pack (y + 6, x + 6)].x = 0;
      pixel[pack (y + 7, x + 5)].x = 0;
      pixel[pack (y + 7, x + 6)].x = 0;
      pixel[pack (y + 8, x + 5)].x = 0;
      pixel[pack (y + 8, x + 6)].x = 0;
      pixel[pack (y + 9, x + 5)].x = 0;
      pixel[pack (y + 9, x + 6)].x = 0;

      break;

    case '8':
      pixel[pack (y + 0, x + 2)].x = 0;
      pixel[pack (y + 0, x + 3)].x = 0;
      pixel[pack (y + 0, x + 4)].x = 0;
      pixel[pack (y + 0, x + 5)].x = 0;
      pixel[pack (y + 1, x + 1)].x = 0;
      pixel[pack (y + 1, x + 2)].x = 0;
      pixel[pack (y + 1, x + 3)].x = 0;
      pixel[pack (y + 1, x + 4)].x = 0;
      pixel[pack (y + 1, x + 5)].x = 0;
      pixel[pack (y + 1, x + 6)].x = 0;
      pixel[pack (y + 2, x + 1)].x = 0;
      pixel[pack (y + 2, x + 2)].x = 0;
      pixel[pack (y + 2, x + 5)].x = 0;
      pixel[pack (y + 2, x + 6)].x = 0;
      pixel[pack (y + 3, x + 1)].x = 0;
      pixel[pack (y + 3, x + 2)].x = 0;
      pixel[pack (y + 3, x + 5)].x = 0;
      pixel[pack (y + 3, x + 6)].x = 0;
      pixel[pack (y + 4, x + 2)].x = 0;
      pixel[pack (y + 4, x + 3)].x = 0;
      pixel[pack (y + 4, x + 4)].x = 0;
      pixel[pack (y + 4, x + 5)].x = 0;
      pixel[pack (y + 5, x + 2)].x = 0;
      pixel[pack (y + 5, x + 3)].x = 0;
      pixel[pack (y + 5, x + 4)].x = 0;
      pixel[pack (y + 5, x + 5)].x = 0;
      pixel[pack (y + 6, x + 1)].x = 0;
      pixel[pack (y + 6, x + 2)].x = 0;
      pixel[pack (y + 6, x + 5)].x = 0;
      pixel[pack (y + 6, x + 6)].x = 0;
      pixel[pack (y + 7, x + 1)].x = 0;
      pixel[pack (y + 7, x + 2)].x = 0;
      pixel[pack (y + 7, x + 5)].x = 0;
      pixel[pack (y + 7, x + 6)].x = 0;
      pixel[pack (y + 8, x + 1)].x = 0;
      pixel[pack (y + 8, x + 2)].x = 0;
      pixel[pack (y + 8, x + 3)].x = 0;
      pixel[pack (y + 8, x + 4)].x = 0;
      pixel[pack (y + 8, x + 5)].x = 0;
      pixel[pack (y + 8, x + 6)].x = 0;
      pixel[pack (y + 9, x + 2)].x = 0;
      pixel[pack (y + 9, x + 3)].x = 0;
      pixel[pack (y + 9, x + 4)].x = 0;
      pixel[pack (y + 9, x + 5)].x = 0;

      OPTIONAL (pixel[pack (y + 0, x + 1)].x = 0);
      OPTIONAL (pixel[pack (y + 0, x + 6)].x = 0);
      OPTIONAL (pixel[pack (y + 4, x + 1)].x = 0);
      OPTIONAL (pixel[pack (y + 4, x + 6)].x = 0);
      OPTIONAL (pixel[pack (y + 5, x + 1)].x = 0);
      OPTIONAL (pixel[pack (y + 5, x + 6)].x = 0);
      OPTIONAL (pixel[pack (y + 9, x + 1)].x = 0);
      OPTIONAL (pixel[pack (y + 9, x + 6)].x = 0);

      break;

    case '9':
      pixel[pack (y + 0, x + 2)].x = 0;
      pixel[pack (y + 0, x + 3)].x = 0;
      pixel[pack (y + 0, x + 4)].x = 0;
      pixel[pack (y + 0, x + 5)].x = 0;
      pixel[pack (y + 1, x + 1)].x = 0;
      pixel[pack (y + 1, x + 2)].x = 0;
      pixel[pack (y + 1, x + 3)].x = 0;
      pixel[pack (y + 1, x + 4)].x = 0;
      pixel[pack (y + 1, x + 5)].x = 0;
      pixel[pack (y + 1, x + 6)].x = 0;
      pixel[pack (y + 2, x + 1)].x = 0;
      pixel[pack (y + 2, x + 2)].x = 0;
      pixel[pack (y + 2, x + 5)].x = 0;
      pixel[pack (y + 2, x + 6)].x = 0;
      pixel[pack (y + 3, x + 1)].x = 0;
      pixel[pack (y + 3, x + 2)].x = 0;
      pixel[pack (y + 3, x + 5)].x = 0;
      pixel[pack (y + 3, x + 6)].x = 0;
      pixel[pack (y + 4, x + 1)].x = 0;
      pixel[pack (y + 4, x + 2)].x = 0;
      pixel[pack (y + 4, x + 3)].x = 0;
      pixel[pack (y + 4, x + 4)].x = 0;
      pixel[pack (y + 4, x + 5)].x = 0;
      pixel[pack (y + 4, x + 6)].x = 0;
      pixel[pack (y + 5, x + 2)].x = 0;
      pixel[pack (y + 5, x + 3)].x = 0;
      pixel[pack (y + 5, x + 4)].x = 0;
      pixel[pack (y + 5, x + 5)].x = 0;
      pixel[pack (y + 5, x + 6)].x = 0;
      pixel[pack (y + 6, x + 5)].x = 0;
      pixel[pack (y + 6, x + 6)].x = 0;
      pixel[pack (y + 7, x + 5)].x = 0;
      pixel[pack (y + 7, x + 6)].x = 0;
      pixel[pack (y + 8, x + 5)].x = 0;
      pixel[pack (y + 8, x + 6)].x = 0;
      pixel[pack (y + 9, x + 5)].x = 0;
      pixel[pack (y + 9, x + 6)].x = 0;

      OPTIONAL (pixel[pack (y + 0, x + 1)].x = 0);
      OPTIONAL (pixel[pack (y + 0, x + 6)].x = 0);
      OPTIONAL (pixel[pack (y + 5, x + 1)].x = 0);

      break;
  }
}



static void
paint_line (gfloat x1,
            gfloat y1,
            gfloat x2,
            gfloat y2)
{
}



static void
paint_back (void)
{
  gint i, j;

  for (i = 0; i < rows; i++)
  for (j = 0; j < cols; j++)
  {
    pixel[pack (i, j)].hue = 3 * (i + j) % 360;
    pixel[pack (i, j)].x   = 0.6;
  }
}



static void
paint_front (void)
{
  gint  len = 0;
  gchar text[16];

  {
    time_t     now   = time (NULL);
    struct tm *local = localtime (&now);

    strftime (text, sizeof (text) / sizeof (text[0]), "%H:%M:%S", local);
  }

  {
    gchar *ptr;

    for (ptr = text; g_utf8_get_char (ptr); ptr = g_utf8_next_char (ptr))
      len += get_width (g_utf8_get_char (ptr));
  }

  {
    gchar *ptr;
    gint   x = (cols - len) >> 1;

    for (ptr = text; g_utf8_get_char (ptr); ptr = g_utf8_next_char (ptr))
    {
      gunichar c = g_utf8_get_char (ptr);

      paint_char (x, rows - SPACE - get_height (c), c);

      x += get_width (c);
    }
  }
}



static void
paint (void)
{
  gdouble dt = get_delta ();

  paint_back  ();
  paint_front ();

  {
    gint i, j;

    for (i = 0; i < rows; i++)
    for (j = 0; j < cols; j++)
    {
      light *l = pixel + pack (i, j);

      update_light (l, dt);

      paint_pixel ((j - cols / 2) * grid,
                   (i - rows / 2) * grid,
                   l->hue, l->y);
    }
  }
}



static gboolean
queue (gpointer data)
{
  clutter_actor_queue_redraw (clutter_stage_get_default ());

  return TRUE;
}



int
main (int   argc,
      char *argv[])
{
  ClutterActor *stage;

  clutter_init      (&argc, &argv);
  clutterrific_init (&argc, &argv);

  {
    ClutterColor bg = { 0, 0, 0, 255 };

    stage = clutter_stage_get_default ();

    clutter_stage_set_color (CLUTTER_STAGE (stage), &bg);
    clutter_actor_show_all  (stage);
    clutter_actor_get_size  (stage, &width, &height);
  }

  rows = ROWS;
  cols = COLS;
  grid = GRID;

  if (rows * grid > height)
    grid = (gfloat) height / rows;

  if (cols * grid > width)
    grid = (gfloat) width / cols;

  rows = (gint) (height / grid + 0.5) | 1;
  cols = (gint) (width  / grid + 0.5) | 1;

  pixel = g_new0 (light, rows * cols);

  g_signal_connect_after (stage, "paint", paint, NULL);

  g_timeout_add (10, queue, NULL);

  clutter_main ();

  g_free (pixel);

  return 0;
}
