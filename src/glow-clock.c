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



#define TRIM

#define A         4

#define ROWS     60
#define COLS     80

#define SPACE     4

#define GRID     15

#define HOUR      0.6
#define MINUTE    0.8
#define SECOND    0.9

#define TEXTURE   8

#define EMIT_IN   0.3
#define EMIT_OUT  0.5



#include <math.h>
#include <time.h>

#include <clutter/clutter.h>

#include "clutterrific.h"



#ifndef M_PI
  #define M_PI 3.141592653589793238
#endif



typedef struct
{
  gfloat hue;

  gfloat x;

  gfloat y;
}
light;



static gfloat      width;
static gfloat      height;

static gint        rows;
static gint        cols;

static gfloat      grid;

static light      *pixel;

static CoglHandle  emit_texture;
static CoglHandle  glow_texture;



static gint     sign         (gint      x);

static gint     pack         (gint      i,
                              gint      j);

static gfloat   norm         (gfloat    x0,
                              gfloat    y0,
                              gfloat    x1,
                              gfloat    y1,
                              gfloat    x,
                              gfloat    y);

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

static void     paint_line   (gfloat    x0,
                              gfloat    y0,
                              gfloat    x1,
                              gfloat    y1);

static void     paint_back   (void);

static void     paint_front  (void);

static void     paint        (void);

static gboolean queue        (gpointer  data);



static gint
sign (gint x)
{
  return x < 0 ? -1 : !!x;
}



static gint
pack (gint i,
      gint j)
{
  return i * cols + j;
}



static gfloat
norm (gfloat x0,
      gfloat y0,
      gfloat x1,
      gfloat y1,
      gfloat x,
      gfloat y)
{
  return ABS ((y0 - y1) * (x - x0) + (x1 - x0) * (y - y0));
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
  ClutterColor c = { 0, 0, 0, 128 };
  CoglHandle   m = cogl_material_new ();

  clutter_color_from_hls     (&c, h, 0.8 * l, 1);
  cogl_material_set_color4ub (m, c.red, c.green, c.blue, c.alpha);
  cogl_material_set_layer    (m, 0, glow_texture);

  cogl_set_source (m);

  cogl_rectangle (x + (width  - 2.0 * grid) / 2.0,
                  y + (height - 2.0 * grid) / 2.0,
                  x + (width  + 2.0 * grid) / 2.0,
                  y + (height + 2.0 * grid) / 2.0);

  cogl_handle_unref (m);

  m = cogl_material_new ();

  clutter_color_from_hls     (&c, h, CLAMP (l * 2, 0, 1), 1);
  cogl_material_set_color4ub (m, c.red, c.green, c.blue, c.alpha);
  cogl_material_set_layer    (m, 0, emit_texture);

  cogl_set_source (m);

  cogl_rectangle_with_texture_coords (x + (width  - 1.0 * grid) / 2.0,
                                      y + (height - 1.0 * grid) / 2.0,
                                      x + (width  + 1.0 * grid) / 2.0,
                                      y + (height + 1.0 * grid) / 2.0,
                                      0, 0, 1, 1);

  cogl_handle_unref (m);
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

#ifndef TRIM
      pixel[pack (y + 0, x + 1)].x = 0;
      pixel[pack (y + 0, x + 6)].x = 0;
      pixel[pack (y + 9, x + 1)].x = 0;
      pixel[pack (y + 9, x + 6)].x = 0;
#endif

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

#ifndef TRIM
      pixel[pack (y + 0, x + 6)].x = 0;
      pixel[pack (y + 4, x + 1)].x = 0;
      pixel[pack (y + 5, x + 6)].x = 0;
#endif

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

#ifndef TRIM
      pixel[pack (y + 0, x + 6)].x = 0;
      pixel[pack (y + 4, x + 6)].x = 0;
      pixel[pack (y + 5, x + 6)].x = 0;
      pixel[pack (y + 9, x + 6)].x = 0;
#endif

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

#ifndef TRIM
      pixel[pack (y + 5, x + 1)].x = 0;
#endif

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

#ifndef TRIM
      pixel[pack (y + 4, x + 6)].x = 0;
      pixel[pack (y + 9, x + 6)].x = 0;
#endif

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

#ifndef TRIM
      pixel[pack (y + 0, x + 1)].x = 0;
      pixel[pack (y + 4, x + 6)].x = 0;
      pixel[pack (y + 9, x + 1)].x = 0;
      pixel[pack (y + 9, x + 6)].x = 0;
#endif

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

#ifndef TRIM
      pixel[pack (y + 0, x + 1)].x = 0;
      pixel[pack (y + 0, x + 6)].x = 0;
      pixel[pack (y + 4, x + 1)].x = 0;
      pixel[pack (y + 4, x + 6)].x = 0;
      pixel[pack (y + 5, x + 1)].x = 0;
      pixel[pack (y + 5, x + 6)].x = 0;
      pixel[pack (y + 9, x + 1)].x = 0;
      pixel[pack (y + 9, x + 6)].x = 0;
#endif

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

#ifndef TRIM
      pixel[pack (y + 0, x + 1)].x = 0;
      pixel[pack (y + 0, x + 6)].x = 0;
      pixel[pack (y + 5, x + 1)].x = 0;
#endif

      break;
  }
}



static void
paint_line (gfloat x0,
            gfloat y0,
            gfloat x1,
            gfloat y1)
{
  gint x2 = (gint) (x0 + 0.5);
  gint y2 = (gint) (y0 + 0.5);
  gint x3 = (gint) (x1 + 0.5);
  gint y3 = (gint) (y1 + 0.5);
  gint dx = sign (x3 - x2);
  gint dy = sign (y3 - y2);
  gint x  = x2;
  gint y  = y2;

  pixel[pack (y, x)].x = 0;

  while (x != x3 || y != y3)
  {
    gfloat d1 = norm (x0, y0, x1, y1, x + dx, y +  0);
    gfloat d2 = norm (x0, y0, x1, y1, x +  0, y + dy);

    if (!dy || d1 <= d2)
      x += dx;
    if (!dx || d1 >= d2)
      y += dy;

    pixel[pack (y, x)].x = 0;
  }
}



static void
paint_back (void)
{
  gint i, j;

  for (i = 0; i < rows; i++)
  for (j = 0; j < cols; j++)
  {
    pixel[pack (i, j)].hue = 3 * (i + j) % 360;
    pixel[pack (i, j)].x   = 1;
  }
}



static void
paint_front (void)
{
  gint  len = 0;
  gchar text[16];

  {
    gfloat r = (rows - get_height (0) - 3 * SPACE) / 2.0;
    gfloat x = cols >> 1;
    gfloat y = (gint) (SPACE + r + 0.5);

    GTimeVal now;
    gfloat h, m, s;

    g_get_current_time (&now);

    g_printf ("%ld\n", now.tv_sec);

    s = now.tv_sec % 60 + now.tv_usec * 1E-6;
    m = now.tv_sec / 60 % 60 + s / 60;
    h = now.tv_sec / 60 / 60 % 24 + m / 60;

    paint_line (x, y, x + HOUR   * r * sin (M_PI * h / 6),
                      y - HOUR   * r * cos (M_PI * h / 6));
    paint_line (x, y, x + MINUTE * r * sin (M_PI * m / 30),
                      y - MINUTE * r * cos (M_PI * m / 30));
    paint_line (x, y, x + SECOND * r * sin (M_PI * s / 30),
                      y - SECOND * r * cos (M_PI * s / 30));

    {
      gint i;

      for (i = 0; i < 12; i++)
        paint_line (x + (r - !(i % 3)) * sin (M_PI * i / 6),
                    y - (r - !(i % 3)) * cos (M_PI * i / 6),
                    x + r * sin (M_PI * i / 6),
                    y - r * cos (M_PI * i / 6));

    }
  }

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

  {
    guint8 emit_data[4 * TEXTURE * TEXTURE];
    guint8 glow_data[4 * TEXTURE * TEXTURE];
    gint   i, j;

    for (i = 0; i < TEXTURE * TEXTURE; i++)
    {
      emit_data[4 * i + 0] = 255;
      emit_data[4 * i + 1] = 255;
      emit_data[4 * i + 2] = 255;
      emit_data[4 * i + 3] = 0;

      glow_data[4 * i + 0] = 255;
      glow_data[4 * i + 1] = 255;
      glow_data[4 * i + 2] = 255;
      glow_data[4 * i + 3] = 0;
    }

    for (i = 0; i < TEXTURE; i++)
    for (j = 0; j < TEXTURE; j++)
    {
      gint   k = i * TEXTURE + j;
      gfloat c = (TEXTURE - 1) / 2.0;
      gfloat x = j - c;
      gfloat y = i - c;

      if (!(TEXTURE & 1))
      {
        x += x < 0 ? 0.5 : -0.5;
        y += y < 0 ? 0.5 : -0.5;
      }

      x /= (TEXTURE - 1) / 2;
      y /= (TEXTURE - 1) / 2;

      {
        gfloat two = sqrt (x * x + y * y);
        gfloat inf = MAX (ABS (x), ABS (y));

        if (inf < EMIT_IN)
          emit_data[4 * k + 3] = 255;
        else if (inf < EMIT_OUT)
          emit_data[4 * k + 3] =  32;

        if (two < 1)
          glow_data[4 * k + 3] = 128 * (1 - two) * (1 - two);
      }
    }

    emit_texture = cogl_texture_new_from_data (TEXTURE, TEXTURE,
                                               COGL_TEXTURE_NONE,
                                               COGL_PIXEL_FORMAT_RGBA_8888,
                                               COGL_PIXEL_FORMAT_ANY,
                                               4 * TEXTURE, emit_data);
    glow_texture = cogl_texture_new_from_data (TEXTURE, TEXTURE,
                                               COGL_TEXTURE_NONE,
                                               COGL_PIXEL_FORMAT_RGBA_8888,
                                               COGL_PIXEL_FORMAT_ANY,
                                               4 * TEXTURE, glow_data);
  }

  g_signal_connect_after (stage, "paint", paint, NULL);

  g_timeout_add (10, queue, NULL);

  clutter_main ();

  cogl_handle_unref (glow_texture);
  cogl_handle_unref (emit_texture);

  g_free (pixel);

  return 0;
}
