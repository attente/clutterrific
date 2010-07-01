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



#define EXTENSIONS    "jpg|png"

#define FADE_DEPTH      400.0

#define FADE_DURATION     5.0

#define ITERATIONS       20

#define GRAVITY       30000.0

#define SEGMENTS         20



#include <ode/ode.h>

#include <clutter/clutter.h>

#include "clutterrific.h"



typedef struct
{
  dBodyID outer[2];

  dBodyID inner[SEGMENTS];
}
Rope;



typedef struct
{
  dBodyID       body;

  Rope          rope[2];

  ClutterActor *actor;
}
Photo;



GPtrArray    *path;

gint          file;

gfloat        width;

gfloat        height;

gfloat        offset;

dWorldID      world;

ClutterActor *stage;

ClutterActor *glass;



static void
update_world (ClutterActor *actor,
              gpointer      data)
{
  gdouble delta = clutterrific_delta ();

  dWorldQuickStep (world, delta);

  offset += delta * PAN;
}



static void
update_fade (ClutterTimeline *timeline,
             gint             time,
             gpointer         data)
{
  gfloat       alpha = clutter_timeline_get_progress (timeline);
  ClutterColor black = { 0, 0, 0, 255 };

  black.alpha *= 1 - alpha;

  clutter_rectangle_set_color (CLUTTER_RECTANGLE (glass), &black);
}



static void
finish_fade (ClutterTimeline *timeline,
             gpointer         data)
{
  clutter_actor_destroy (glass);

  g_object_unref (timeline);

  g_signal_connect_after (stage, "paint", G_CALLBACK (update_world), NULL);
}



int
main (int   argc,
      char *argv[])
{
  clutter_init      (&argc, &argv);
  clutterrific_init (&argc, &argv);

  cogl_set_depth_test_enabled (TRUE);

  {
    const gchar *dir = g_get_user_special_dir (G_USER_DIRECTORY_PICTURES);

    path = clutterrific_list (dir, "(?i)\\.(" EXTENSIONS ")$");
    file = 0;

    clutterrific_shuffle (path);
  }

  offset = 0;

  dInitODE ();
  world = dWorldCreate ();
  dWorldSetGravity (world, 0, GRAVITY, 0);
  dWorldSetQuickStepNumIterations (world, ITERATIONS);

  {
    ClutterColor black = {   0,   0,   0, 255 };
    ClutterColor white = { 255, 255, 255, 255 };

    stage = clutter_stage_get_default        ();
    glass = clutter_rectangle_new_with_color (&black);

    clutter_stage_set_color (CLUTTER_STAGE (stage), &white);
    clutter_actor_show_all  (stage);
    clutter_actor_get_size  (stage, &width, &height);

    clutter_actor_set_position (glass, -width, -height);
    clutter_actor_set_size     (glass, 3 * width, 3 * height);
    clutter_actor_set_depth    (glass, FADE_DEPTH);

    clutter_container_add_actor (CLUTTER_CONTAINER (stage), glass);
  }

  {
    ClutterTimeline *fade = clutter_timeline_new (1000 * FADE_DURATION);

    g_signal_connect (fade, "new-frame", G_CALLBACK (update_fade), NULL);
    g_signal_connect (fade, "completed", G_CALLBACK (finish_fade), NULL);

    clutter_timeline_start (fade);
  }

  clutter_main ();

  dWorldDestroy (world);
  dCloseODE ();

  return 0;
}
