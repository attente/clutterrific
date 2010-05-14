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



#define A     1

#define ROWS 60
#define COLS 80

#define GRID 15



#include <math.h>

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



static void     update_light (light    *l,
                              gdouble   dt);

static void     paint_pixel  (gfloat    x,
                              gfloat    y,
                              gfloat    h,
                              gfloat    l);

static gdouble  get_delta    (void);

static void     paint        (void);

static gboolean queue        (gpointer  data);



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
  cogl_set_source_color4f (x * x / 250000, y * y / 250000, 0.8, 1);

  cogl_rectangle (x + (width - grid + 1) / 2.0,
                  y + (width - grid + 1) / 2.0,
                  x + (width + grid - 1) / 2.0,
                  y + (width + grid - 1) / 2.0);
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



static void
paint (void)
{
  gdouble dt = get_delta ();

  {
    gint i, j;

    for (i = 0; i < rows; i++)
    for (j = 0; j < cols; j++)
    {
      light *l = pixel + i * cols + j;

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

  pixel = g_new (light, rows * cols);

  g_signal_connect_after (stage, "paint", paint, NULL);

  g_timeout_add (10, queue, NULL);

  clutter_main ();

  g_free (pixel);

  return 0;
}
