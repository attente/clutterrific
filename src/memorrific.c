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



#define SCHEMES 9



#include <clutter/clutter.h>

#include "clutterrific.h"



typedef ClutterColor (*ColourScheme) (const ClutterColor *colour,
                                      gfloat              parameter);



static ColourScheme SCHEME[SCHEMES];

static ClutterColor stage_colour;

static ClutterColor target_colour;

static ColourScheme scheme;

static ClutterColor scheme_colour;

static gfloat       scheme_parameter;



static ClutterColor
get_random_achromatic_colour (const ClutterColor *colour,
                              gfloat              parameter)
{
}



static ClutterColor
get_random_monochromatic_colour (const ClutterColor *colour,
                                 gfloat              parameter)
{
}



static ClutterColor
get_random_analogous_colour (const ClutterColor *colour,
                             gfloat              parameter)
{
}



static ClutterColor
get_random_complementary_colour (const ClutterColor *colour,
                                 gfloat              parameter)
{
}



static ClutterColor
get_random_triadic_colour (const ClutterColor *colour,
                           gfloat              parameter)
{
}



static ClutterColor
get_random_tetradic_colour (const ClutterColor *colour,
                            gfloat              parameter)
{
}



static ClutterColor
get_random_neutral_colour (const ClutterColor *colour,
                           gfloat              parameter)
{
}



static ClutterColor
get_random_warm_colour (const ClutterColor *colour,
                        gfloat              parameter)
{
}



static ClutterColor
get_random_cool_colour (const ClutterColor *colour,
                        gfloat              parameter)
{
}



int
main (int   argc,
      char *argv[])
{
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

  clutter_main ();

  return 0;
}
