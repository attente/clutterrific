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



#define WIDTH  800
#define HEIGHT 600



#include <clutter/clutter.h>



typedef struct
{
  ClutterColor color;

  gdouble      angle;
  gdouble      size;
}
atom;



typedef struct
{
  ClutterActor **actor;
  atom          *state;

  gint           start;
  gint           length;
}
tail;



static gboolean tick (gpointer data);



static gboolean
tick (gpointer data)
{
}



int
main (int   argc,
      char *argv[])
{
  gint width  = WIDTH;
  gint height = HEIGHT;

  ClutterActor *stage;

  clutter_init (&argc, &argv);

  {
    ClutterColor bg = { 0, 0, 0, 255 };
    stage = clutter_stage_get_default ();

    clutter_stage_set_color (CLUTTER_STAGE (stage), &bg);
    clutter_actor_set_size  (stage, width, height);
  }

  clutter_actor_show_all (stage);

  g_timeout_add (15, tick, NULL);

  clutter_main ();

  return 0;
}
