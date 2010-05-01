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



#include <clutter/clutter.h>



static ClutterActor *group;
static ClutterActor *child;



static void
tick (ClutterTimeline *timeline,
      gint             frame,
      gpointer         data)
{
  static gfloat angle = 0;

  angle += 0.5;

  clutter_actor_set_rotation (group, CLUTTER_X_AXIS, angle, 50, 50, 50);
  clutter_actor_set_rotation (group, CLUTTER_Y_AXIS,    45, 50, 50, 50);
  clutter_actor_set_rotation (child, CLUTTER_Y_AXIS, angle, 50, 50, 50);
}



int
main (int   argc,
      char *argv[])
{
  ClutterActor    *stage;
  ClutterTimeline *timeline;
  ClutterColor     bg  = { 0, 0, 0, 255 };
  ClutterColor     col = { 255, 255, 255, 255 };

  clutter_init (&argc, &argv);

  stage = clutter_stage_get_default ();
  clutter_stage_set_color (CLUTTER_STAGE (stage), &bg);
  clutter_actor_set_size  (stage, 800, 600);

  child = clutter_rectangle_new_with_color (&col);
  clutter_actor_set_size      (child, 100, 100);

  /* THIS LINE IS THE PROBLEM */
  clutter_actor_set_clip      (child, 25, 25, 50, 50);
  /* THIS LINE IS THE PROBLEM */

  group = clutter_group_new ();
  clutter_actor_set_position  (group, 350, 250);

  clutter_container_add_actor (CLUTTER_CONTAINER (group), child);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), group);
  clutter_actor_show_all      (stage);

  timeline = clutter_timeline_new (1000);
  g_signal_connect (timeline, "new-frame", G_CALLBACK (tick), NULL);
  clutter_timeline_set_loop (timeline, TRUE);
  clutter_timeline_start (timeline);

  clutter_main ();

  return 0;
}
