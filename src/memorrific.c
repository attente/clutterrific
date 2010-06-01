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



#define SCHEMES            9

#define FADE_TIME       5000

#define MIN_THEME_TIME 15000

#define MAX_THEME_TIME 25000



#include <clutter/clutter.h>

#include "clutterrific.h"



typedef struct
{
  gfloat h;

  gfloat s;

  gfloat l;

  gfloat a;
}
FineColour;



/*
 * When base is NULL,
 * ColourSchemes should return a suitable base colour based on parameter.
 *
 * When base is not NULL and target is NULL,
 * ColourSchemes should return a background based on base and parameter.
 *
 * When base and target both are not NULL,
 * ColourSchemes should return a random colour based on all three parameters.
 */

typedef FineColour (*ColourScheme) (const FineColour *base,
                                    const FineColour *target,
                                    gfloat            parameter);



static gfloat       width;

static gfloat       height;

static ColourScheme SCHEME[SCHEMES];

static FineColour   stage_colour;

static FineColour   target_colour;

static ColourScheme scheme;

static FineColour   scheme_colour;

static gfloat       scheme_parameter;



static FineColour
get_random_achromatic_colour (const FineColour *base,
                              const FineColour *target,
                              gfloat            parameter)
{
  FineColour colour;

  colour.h = parameter;
  colour.s = 1;
  colour.l = 0.95;
  colour.a = 1;

  return colour;
}



static FineColour
get_random_monochromatic_colour (const FineColour *base,
                                 const FineColour *target,
                                 gfloat            parameter)
{
}



static FineColour
get_random_analogous_colour (const FineColour *base,
                             const FineColour *target,
                             gfloat            parameter)
{
}



static FineColour
get_random_complementary_colour (const FineColour *base,
                                 const FineColour *target,
                                 gfloat            parameter)
{
}



static FineColour
get_random_triadic_colour (const FineColour *base,
                           const FineColour *target,
                           gfloat            parameter)
{
}



static FineColour
get_random_tetradic_colour (const FineColour *base,
                            const FineColour *target,
                            gfloat            parameter)
{
}



static FineColour
get_random_neutral_colour (const FineColour *base,
                           const FineColour *target,
                           gfloat            parameter)
{
}



static FineColour
get_random_warm_colour (const FineColour *base,
                        const FineColour *target,
                        gfloat            parameter)
{
}



static FineColour
get_random_cool_colour (const FineColour *base,
                        const FineColour *target,
                        gfloat            parameter)
{
}



static gdouble
get_delta (GTimeVal *time)
{
  GTimeVal now;
  gdouble  dt = 0;

  g_get_current_time (&now);

  if (time->tv_sec)
    dt = now.tv_sec - time->tv_sec + 1E-6 * (now.tv_usec - time->tv_usec);

  *time = now;

  return dt;
}



static ClutterColor
get_colour (const FineColour *data)
{
  ClutterColor colour;

  clutter_color_from_hls (&colour, 360 * data->h, data->l, data->s);

  colour.alpha = CLAMP (256 * data->a, 0, 255);

  return colour;
}



static void
select_theme (ClutterTimeline *timeline,
              gpointer         data)
{
  scheme           = SCHEME[g_random_int_range (0, 1/*SCHEMES*/)];
  scheme_parameter = g_random_double ();
  stage_colour     = target_colour;
  scheme_colour    = scheme (NULL, NULL, scheme_parameter);
  target_colour    = scheme (&scheme_colour, NULL, scheme_parameter);
}



static void
update_theme (ClutterTimeline *timeline,
              gint             time,
              gpointer         data)
{
  gfloat     dh;
  gfloat     ratio;
  FineColour colour;

  ratio = CLAMP ((gfloat) time / FADE_TIME, 0, 1);
  dh    = target_colour.h - stage_colour.h;

  if (dh < -0.5)
    dh++;
  else if (dh > 0.5)
    dh--;

  colour.h = stage_colour.h + ratio * dh;
  colour.s = stage_colour.s + ratio * (target_colour.s - stage_colour.s);
  colour.l = stage_colour.l + ratio * (target_colour.l - stage_colour.l);

  if (colour.h < 0)
    colour.h++;
  else if (colour.h >= 1)
    colour.h--;

  {
    ClutterColor real = get_colour (&colour);

    clutter_stage_set_color (CLUTTER_STAGE (clutter_stage_get_default ()), &real);
  }
}



static void
repeat_theme (ClutterTimeline *timeline,
              gpointer         data)
{
  g_object_unref (timeline);

  timeline = clutter_timeline_new (g_random_int_range (MIN_THEME_TIME, MAX_THEME_TIME + 1));

  g_signal_connect (timeline, "started",   G_CALLBACK (select_theme), NULL);
  g_signal_connect (timeline, "new-frame", G_CALLBACK (update_theme), NULL);
  g_signal_connect (timeline, "completed", G_CALLBACK (repeat_theme), NULL);

  clutter_timeline_start (timeline);
}



int
main (int   argc,
      char *argv[])
{
  ClutterActor *stage;

  clutter_init      (&argc, &argv);
  clutterrific_init (&argc, &argv);

  SCHEME[0] = get_random_achromatic_colour;
  SCHEME[1] = get_random_monochromatic_colour;
  SCHEME[2] = get_random_analogous_colour;
  SCHEME[3] = get_random_complementary_colour;
  SCHEME[4] = get_random_triadic_colour;
  SCHEME[5] = get_random_tetradic_colour;
  SCHEME[6] = get_random_neutral_colour;
  SCHEME[7] = get_random_warm_colour;
  SCHEME[8] = get_random_cool_colour;

  stage_colour.h = 0;
  stage_colour.s = 0;
  stage_colour.l = 0;
  stage_colour.a = 0;

  stage = clutter_stage_get_default ();

  {
    ClutterColor colour = get_colour (&stage_colour);

    clutter_stage_set_color (CLUTTER_STAGE (stage), &colour);
    clutter_actor_show_all  (stage);
    clutter_actor_get_size  (stage, &width, &height);
  }

  {
    ClutterTimeline *timeline = clutter_timeline_new (g_random_int_range (MIN_THEME_TIME, MAX_THEME_TIME + 1));

    g_signal_connect (timeline, "started",   G_CALLBACK (select_theme), NULL);
    g_signal_connect (timeline, "new-frame", G_CALLBACK (update_theme), NULL);
    g_signal_connect (timeline, "completed", G_CALLBACK (repeat_theme), NULL);

    clutter_timeline_start (timeline);
  }

  clutter_main ();

  return 0;
}
