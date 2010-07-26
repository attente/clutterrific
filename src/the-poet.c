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



#define CAPS   3

#define STEPS 64



#include <math.h>

#include <clutter/clutter.h>

#include "clutterrific.h"



#ifndef M_PI
  #define M_PI 3.141592653589793238
#endif



typedef struct
{
  gfloat x;

  gfloat y;

  gfloat z;
}
Point;



typedef struct
{
  gint    n;

  Point  *p;

  gfloat  cap[2];
}
Curve;



static gfloat        shift;

static ClutterActor *stage;



static void
paint_curve (const Curve *c,
             gfloat       t)
{
  Point   para, perp, q;
  gint    n, i, j;
  gfloat *data;
  gfloat  l;
  Point  *p;

  if (c == NULL || !c->n)
    return;

  t    = CLAMP (t, 0, 1) * c->n;
  n    = CAPS + 1 + STEPS * t + CAPS;
  data = g_new (gfloat, 3 * (1 + 2 * n + 1));

  q.x = 0;
  q.y = 0;
  q.z = 0;

  para.x = c->p[0].x - c->p[1].x;
  para.y = c->p[0].y - c->p[1].y;

  l = sqrt (para.x * para.x + para.y * para.y);

  perp.x = -para.y * c->p[0].z / l;
  perp.y =  para.x * c->p[0].z / l;
  para.x *= c->cap[0] / l;
  para.y *= c->cap[0] / l;

  data[0] = c->p[0].x + para.x;
  data[1] = c->p[0].y + para.y;
  data[2] = 0;

  for (i = 0; i <= CAPS; i++)
  {
    gfloat x = sin (M_PI / 2 * (i + 1) / (CAPS + 1));
    gfloat y = cos (M_PI / 2 * (i + 1) / (CAPS + 1));

    data[3 * (1 + 2 * i + 0) + 0] = c->p[0].x + x * perp.x + y * para.x;
    data[3 * (1 + 2 * i + 0) + 1] = c->p[0].y + x * perp.y + y * para.y;
    data[3 * (1 + 2 * i + 0) + 2] = 0;
    data[3 * (1 + 2 * i + 1) + 0] = c->p[0].x - x * perp.x + y * para.x;
    data[3 * (1 + 2 * i + 1) + 1] = c->p[0].y - x * perp.y + y * para.y;
    data[3 * (1 + 2 * i + 1) + 2] = 0;
  }

  for (i = 0; i < t; i++)
  {
    p = c->p + 3 * i;

    for (j = 1; j <= STEPS && j <= STEPS * (t - i) + 1; j++)
    {
      gfloat s = MIN (1.0 * j / STEPS, t - i);
      gfloat r = 1 - s;

      gfloat r2 = r * r;
      gfloat r3 = r * r2;

      gfloat s2 = s * s;
      gfloat s3 = s * s2;

      gfloat r2s = r2 * s;
      gfloat rs2 = r * s2;

      q.x = r3 * p[0].x + 3 * r2s * p[1].x + 3 * rs2 * p[2].x + s3 * p[3].x;
      q.y = r3 * p[0].y + 3 * r2s * p[1].y + 3 * rs2 * p[2].y + s3 * p[3].y;
      q.z = r3 * p[0].z + 3 * r2s * p[1].z + 3 * rs2 * p[2].z + s3 * p[3].z;

      para.x = 3 * (-r2             * p[0].x +
                    r * (1 - 3 * s) * p[1].x +
                    s * (2 - 3 * s) * p[2].x +
                    s2              * p[3].x);
      para.y = 3 * (-r2             * p[0].y +
                    r * (1 - 3 * s) * p[1].y +
                    s * (2 - 3 * s) * p[2].y +
                    s2              * p[3].y);
      para.z = 3 * (-r2             * p[0].z +
                    r * (1 - 3 * s) * p[1].z +
                    s * (2 - 3 * s) * p[2].z +
                    s2              * p[3].z);

      l = sqrt (para.x * para.x + para.y * para.y);

      perp.x = -para.y * q.z / l;
      perp.y =  para.x * q.z / l;

      data[3 * (1 + 2 * (CAPS + STEPS * i + j) + 0) + 0] = q.x - perp.x;
      data[3 * (1 + 2 * (CAPS + STEPS * i + j) + 0) + 1] = q.y - perp.y;
      data[3 * (1 + 2 * (CAPS + STEPS * i + j) + 0) + 2] = 0;
      data[3 * (1 + 2 * (CAPS + STEPS * i + j) + 1) + 0] = q.x + perp.x;
      data[3 * (1 + 2 * (CAPS + STEPS * i + j) + 1) + 1] = q.y + perp.y;
      data[3 * (1 + 2 * (CAPS + STEPS * i + j) + 1) + 2] = 0;
    }
  }

  para.x *= c->cap[1] / l;
  para.y *= c->cap[1] / l;

  for (i = 0; i <= CAPS; i++)
  {
    gfloat x = cos (M_PI / 2 * i / (CAPS + 1));
    gfloat y = sin (M_PI / 2 * i / (CAPS + 1));

    data[3 * (1 + 2 * (n - CAPS - 1 + i) + 0) + 0] = q.x - x * perp.x + y * para.x;
    data[3 * (1 + 2 * (n - CAPS - 1 + i) + 0) + 1] = q.y - x * perp.y + y * para.y;
    data[3 * (1 + 2 * (n - CAPS - 1 + i) + 0) + 2] = 0;
    data[3 * (1 + 2 * (n - CAPS - 1 + i) + 1) + 0] = q.x + x * perp.x + y * para.x;
    data[3 * (1 + 2 * (n - CAPS - 1 + i) + 1) + 1] = q.y + x * perp.y + y * para.y;
    data[3 * (1 + 2 * (n - CAPS - 1 + i) + 1) + 2] = 0;
  }

  data[3 * (1 + 2 * n) + 0] = q.x + para.x;
  data[3 * (1 + 2 * n) + 1] = q.y + para.y;
  data[3 * (1 + 2 * n) + 2] = 0;

  {
    CoglHandle        buffer = cogl_vertex_buffer_new (1 + 2 * n + 1);
    CoglVerticesMode  mode   = COGL_VERTICES_MODE_TRIANGLE_STRIP;
    CoglAttributeType type   = COGL_ATTRIBUTE_TYPE_FLOAT;
    gint              stride = 3 * sizeof (gfloat);

    cogl_vertex_buffer_add (buffer, "gl_Vertex", 3, type, TRUE, stride, data);
    cogl_vertex_buffer_draw (buffer, mode, 0, 1 + 2 * n + 1);
    cogl_handle_unref (buffer);
  }

  g_free (data);
}



static void
paint_XXX (void)
{
  static gfloat t = 0;
  Curve         c;

  t += 0.01;

  c.n = 2;
  c.p = g_new (Point, 7);
  c.p[0].x = 100;
  c.p[0].y = 100;
  c.p[0].z = 40;
  c.p[1].x = 100;
  c.p[1].y = 300;
  c.p[1].z = 40;
  c.p[2].x = 800;
  c.p[2].y = 300;
  c.p[2].z = 40;
  c.p[3].x = 800;
  c.p[3].y = 500;
  c.p[3].z = 20;
  c.p[4].x = 800;
  c.p[4].y = -200;
  c.p[4].z = 30;
  c.p[5].x = 500;
  c.p[5].y = 500;
  c.p[5].z = 40;
  c.p[6].x = 100;
  c.p[6].y = 500;
  c.p[6].z = 10;
  c.cap[0] = 20;
  c.cap[1] = 40;

  cogl_set_source_color4f (0.2, 0.2, 0.4, 1.0);

  paint_curve (&c, t);
}

static gboolean
redraw_XXX (gpointer data)
{
  clutter_actor_queue_redraw (stage);

  return TRUE;
}



int
main (int   argc,
      char *argv[])
{
  shift = 0;

  clutter_init      (&argc, &argv);
  clutterrific_init (&argc, &argv);

  {
    ClutterColor bg = { 255, 255, 255, 255 };

    stage = clutter_stage_get_default ();
    clutter_stage_set_color (CLUTTER_STAGE (stage), &bg);
    clutter_actor_set_size (stage, 1000, 700);
    clutter_actor_show_all (stage);
  }

  g_timeout_add (10, redraw_XXX, NULL);
  g_signal_connect_after (stage, "paint", paint_XXX, NULL);

  clutter_main ();

  return 0;
}
