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



#define FADE_DEPTH    400.0

#define FADE_DURATION   5.0

#define FLIP_DURATION   0.4

#define QUOTE(x)      #x

#define EXPAND(x)     QUOTE(x)

#define SVG           "flip-clock.svg"

#define SVG_DATA_0    SVG

#define SVG_DATA_1    "data/" SVG

#define SVG_DATA_2    "../" SVG_DATA_1

#define SVG_DATA_3    EXPAND(DATA_DIR) "/" SVG



#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>
#include <clutter/clutter.h>

#include "clutterrific.h"



static const gchar *TILE[] =
{
  "#0",
  "#1",
  "#2",
  "#3",
  "#4",
  "#5",
  "#6",
  "#7",
  "#8",
  "#9",
  "#am",
  "#pm",
  "#jan",
  "#feb",
  "#mar",
  "#apr",
  "#may",
  "#jun",
  "#jul",
  "#aug",
  "#sep",
  "#oct",
  "#nov",
  "#dec",
  "#sun",
  "#mon",
  "#tue",
  "#wed",
  "#thu",
  "#fri",
  "#sat"
};

static const gchar *ABOVE[] =
{
  "#year_3_a",
  "#year_2_a",
  "#year_1_a",
  "#year_0_a",
  "#month_a",
  "#day_1_a",
  "#day_0_a",
  "#hour_1_a",
  "#hour_0_a",
  "#minute_1_a",
  "#minute_0_a",
  "#second_1_a",
  "#second_0_a",
  "#weekday_a",
  "#ampm_a"
};

static const gchar *BELOW[] =
{
  "#year_3_b",
  "#year_2_b",
  "#year_1_b",
  "#year_0_b",
  "#month_b",
  "#day_1_b",
  "#day_0_b",
  "#hour_1_b",
  "#hour_0_b",
  "#minute_1_b",
  "#minute_0_b",
  "#second_1_b",
  "#second_0_b",
  "#weekday_b",
  "#ampm_b"
};



typedef struct
{
  gfloat x, y;

  gfloat w, h;
}
Rectangle;



static gfloat         width;

static gfloat         height;

static struct tm      before;

static ClutterActor  *glass;

static ClutterActor  *group;

static ClutterActor  *panel;

static ClutterActor  *tile[31];

static ClutterActor **digit;

static ClutterActor **ampm;

static ClutterActor **month;

static ClutterActor **weekday;

static Rectangle      above[15];

static Rectangle      below[15];

static ClutterActor  *upper[15];

static ClutterActor  *lower[15];

static ClutterActor  *middle[15];

static ClutterActor  *prev[15];

static ClutterActor  *next[15];



static gint           get_delay   (void);

static void           orient      (Rectangle       *r);

static void           align       (ClutterActor    *actor,
                                   const Rectangle *first,
                                   const Rectangle *second,
                                   gfloat           rotate);

static ClutterActor * render      (RsvgHandle      *svg,
                                   const gchar     *id);

static gboolean       import      (void);

static void           set_time    (void);

static void           start       (ClutterTimeline *timeline,
                                   gpointer         data);

static void           update      (ClutterTimeline *timeline,
                                   gint             time,
                                   gpointer         data);

static void           restart     (ClutterTimeline *timeline,
                                   gpointer         data);

static void           start_fade  (ClutterTimeline *timeline,
                                   gpointer         data);

static void           update_fade (ClutterTimeline *timeline,
                                   gint             time,
                                   gpointer         data);

static void           finish_fade (ClutterTimeline *timeline,
                                   gpointer         data);



static gint
get_delay (void)
{
  GTimeVal now;

  g_get_current_time (&now);

  return (1499 - now.tv_usec / 1000 % 1000) % 1000;
}



static void
orient (Rectangle *r)
{
  if (r->w < 0)
  {
    r->x += r->w;
    r->w *= -1;
  }

  if (r->h < 0)
  {
    r->y += r->h;
    r->h *= -1;
  }
}



static ClutterActor *
render (RsvgHandle  *svg,
        const gchar *id)
{
  gboolean             success;
  RsvgPositionData     anchor;
  RsvgDimensionData    size;
  ClutterActor        *actor;
  ClutterCairoTexture *texture;
  cairo_t             *context;

  if (!rsvg_handle_get_dimensions_sub (svg, &size, id))
    return NULL;

  anchor.x = 0;
  anchor.y = 0;

  rsvg_handle_get_position_sub (svg, &anchor, id);

  actor   = clutter_cairo_texture_new (size.width, size.height);
  texture = CLUTTER_CAIRO_TEXTURE (actor);
  context = clutter_cairo_texture_create (texture);

  cairo_translate (context, -anchor.x, -anchor.y);

  success = rsvg_handle_render_cairo_sub (svg, context, id);

  cairo_destroy (context);

  if (success)
    return actor;

  clutter_actor_destroy (actor);

  return NULL;
}



static gboolean
import (void)
{
  ClutterActor *stage;
  RsvgHandle   *svg;

  rsvg_init ();

  if ((svg = rsvg_handle_new_from_file (SVG_DATA_0, NULL)) == NULL)
  if ((svg = rsvg_handle_new_from_file (SVG_DATA_1, NULL)) == NULL)
  if ((svg = rsvg_handle_new_from_file (SVG_DATA_2, NULL)) == NULL)
  if ((svg = rsvg_handle_new_from_file (SVG_DATA_3, NULL)) == NULL)
    return FALSE;

  stage = clutter_stage_get_default ();
  group = clutter_group_new ();

  {
    gint i;

    for (i = 0; i < sizeof (TILE) / sizeof (TILE[0]); i++)
    {
      if ((tile[i] = render (svg, TILE[i])) != NULL)
      {
        clutter_actor_set_position  (tile[i], 2 * width, 2 * height);
        clutter_container_add_actor (CLUTTER_CONTAINER (stage), tile[i]);
      }
    }
  }

  if ((panel = render (svg, "#panel")) != NULL)
  {
    gfloat w, h;
    gfloat scale;

    clutter_actor_get_size (panel, &w, &h);

    if (w * height < h * width)
      scale = height / h;
    else
      scale = width  / w;

    w *= scale;
    h *= scale;

    clutter_actor_set_depth     (panel, -1E-3);
    clutter_actor_set_scale     (group, scale, scale);
    clutter_actor_set_position  (group, (width - w) / 2, (height - h) / 2);
    clutter_container_add_actor (CLUTTER_CONTAINER (group), panel);
  }

  {
    RsvgPositionData  origin;
    RsvgPositionData  anchor;
    RsvgDimensionData size;
    gfloat            y, h;
    gint              i;

    rsvg_handle_get_position_sub (svg, &origin, "#panel");

    for (i = 0; i < sizeof (ABOVE) / sizeof (ABOVE[0]); i++)
    {
      rsvg_handle_get_position_sub   (svg, &anchor, ABOVE[i]);
      rsvg_handle_get_dimensions_sub (svg, &size,   ABOVE[i]);

      above[i].x = anchor.x - origin.x;
      above[i].y = anchor.y - origin.y;
      above[i].w = size.width;
      above[i].h = size.height;

      rsvg_handle_get_position_sub   (svg, &anchor, BELOW[i]);
      rsvg_handle_get_dimensions_sub (svg, &size,   BELOW[i]);

      below[i].x = anchor.x - origin.x;
      below[i].y = anchor.y - origin.y;
      below[i].w = size.width;
      below[i].h = size.height;

      orient (above + i);
      orient (below + i);

      y = below[i].y - above[i].y;
      h = y + below[i].h;

      upper[i] = clutter_clone_new (NULL);
      lower[i] = clutter_clone_new (NULL);

      align (upper[i], above + i, below + i, 0);
      align (lower[i], below + i, above + i, 0);

      clutter_container_add (CLUTTER_CONTAINER (group), upper[i], lower[i], NULL);
    }
  }

  rsvg_term ();

  clutter_container_add_actor (CLUTTER_CONTAINER (stage), group);

  return TRUE;
}



static void
set_time (void)
{
  time_t     t_t = time (NULL);
  struct tm *now = localtime (&t_t);
  gint       hh0 = before.tm_hour % 12;
  gint       hh1 = now->tm_hour   % 12;

  now->tm_year += 1900;

  if (!hh0)
    hh0 = 12;
  if (!hh1)
    hh1 = 12;

  clutter_clone_set_source (CLUTTER_CLONE (upper[ 0]),
                            digit[now->tm_year / 1000 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (upper[ 1]),
                            digit[now->tm_year /  100 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (upper[ 2]),
                            digit[now->tm_year /   10 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (upper[ 3]),
                            digit[now->tm_year /    1 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (upper[ 4]),
                            month[now->tm_mon]);
  clutter_clone_set_source (CLUTTER_CLONE (upper[ 5]),
                            digit[now->tm_mday /   10 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (upper[ 6]),
                            digit[now->tm_mday /    1 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (upper[ 7]),
                            digit[hh1          /   10 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (upper[ 8]),
                            digit[hh1          /    1 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (upper[ 9]),
                            digit[now->tm_min  /   10 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (upper[10]),
                            digit[now->tm_min  /    1 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (upper[11]),
                            digit[now->tm_sec  /   10 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (upper[12]),
                            digit[now->tm_sec  /    1 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (upper[13]),
                            weekday[now->tm_wday]);
  clutter_clone_set_source (CLUTTER_CLONE (upper[14]),
                            ampm[now->tm_hour / 12]);

  clutter_clone_set_source (CLUTTER_CLONE (lower[ 0]),
                            digit[before.tm_year / 1000 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (lower[ 1]),
                            digit[before.tm_year /  100 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (lower[ 2]),
                            digit[before.tm_year /   10 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (lower[ 3]),
                            digit[before.tm_year /    1 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (lower[ 4]),
                            month[before.tm_mon]);
  clutter_clone_set_source (CLUTTER_CLONE (lower[ 5]),
                            digit[before.tm_mday /   10 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (lower[ 6]),
                            digit[before.tm_mday /    1 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (lower[ 7]),
                            digit[hh0            /   10 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (lower[ 8]),
                            digit[hh0            /    1 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (lower[ 9]),
                            digit[before.tm_min  /   10 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (lower[10]),
                            digit[before.tm_min  /    1 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (lower[11]),
                            digit[before.tm_sec  /   10 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (lower[12]),
                            digit[before.tm_sec  /    1 % 10]);
  clutter_clone_set_source (CLUTTER_CLONE (lower[13]),
                            weekday[before.tm_wday]);
  clutter_clone_set_source (CLUTTER_CLONE (lower[14]),
                            ampm[before.tm_hour / 12]);
}



static void
align (ClutterActor    *actor,
       const Rectangle *first,
       const Rectangle *second,
       gfloat           rotate)
{
  Rectangle whole;
  gfloat    x, y, z;

  whole.x = MIN (first->x, second->x);
  whole.y = MIN (first->y, second->y);
  whole.w = MAX (first->x + first->w, second->x + second->w) - whole.x;
  whole.h = MAX (first->y + first->h, second->y + second->h) - whole.y;

  x = whole.w / 2;
  y = whole.h / 2;
  z = clutter_actor_get_depth (actor);

  clutter_actor_set_position (actor, whole.x, whole.y);
  clutter_actor_set_size     (actor, whole.w, whole.h);
  clutter_actor_set_clip     (actor, first->x - whole.x,
                              first->y - whole.y,
                              first->w, first->h);
  clutter_actor_set_rotation (actor, CLUTTER_X_AXIS,
                              -180 * rotate,
                              x, y, z);
}



static void
start (ClutterTimeline *timeline,
       gpointer         data)
{
  time_t     t_t = time (NULL);
  struct tm *now = localtime (&t_t);
  gint       hh0 = before.tm_hour % 12;
  gint       hh1 = now->tm_hour   % 12;

  now->tm_year += 1900;

  if (!hh0)
    hh0 = 12;
  if (!hh1)
    hh1 = 12;

  prev[ 0] = digit[before.tm_year / 1000 % 10];
  prev[ 1] = digit[before.tm_year /  100 % 10];
  prev[ 2] = digit[before.tm_year /   10 % 10];
  prev[ 3] = digit[before.tm_year /    1 % 10];
  prev[ 4] = month[before.tm_mon];
  prev[ 5] = digit[before.tm_mday /   10 % 10];
  prev[ 6] = digit[before.tm_mday /    1 % 10];
  prev[ 7] = digit[hh0            /   10 % 10];
  prev[ 8] = digit[hh0            /    1 % 10];
  prev[ 9] = digit[before.tm_min  /   10 % 10];
  prev[10] = digit[before.tm_min  /    1 % 10];
  prev[11] = digit[before.tm_sec  /   10 % 10];
  prev[12] = digit[before.tm_sec  /    1 % 10];
  prev[13] = weekday[before.tm_wday];
  prev[14] = ampm[before.tm_hour / 12];

  next[ 0] = digit[now->tm_year / 1000 % 10];
  next[ 1] = digit[now->tm_year /  100 % 10];
  next[ 2] = digit[now->tm_year /   10 % 10];
  next[ 3] = digit[now->tm_year /    1 % 10];
  next[ 4] = month[now->tm_mon];
  next[ 5] = digit[now->tm_mday /   10 % 10];
  next[ 6] = digit[now->tm_mday /    1 % 10];
  next[ 7] = digit[hh1           /   10 % 10];
  next[ 8] = digit[hh1           /    1 % 10];
  next[ 9] = digit[now->tm_min  /   10 % 10];
  next[10] = digit[now->tm_min  /    1 % 10];
  next[11] = digit[now->tm_sec  /   10 % 10];
  next[12] = digit[now->tm_sec  /    1 % 10];
  next[13] = weekday[now->tm_wday];
  next[14] = ampm[now->tm_hour / 12];

  {
    ClutterActor *stage;
    gint          i;

    stage = clutter_stage_get_default ();

    for (i = 0; i < sizeof (ABOVE) / sizeof (ABOVE[0]); i++)
    {
      if (prev[i] != next[i])
      {
        middle[i] = clutter_clone_new (prev[i]);

        align (middle[i], above + i, below + i, 0);

        clutter_actor_set_depth     (middle[i], 1E-3);
        clutter_container_add_actor (CLUTTER_CONTAINER (group), middle[i]);
      }

      clutter_clone_set_source (CLUTTER_CLONE (upper[i]), next[i]);
    }
  }
}



static void
update (ClutterTimeline *timeline,
        gint             time,
        gpointer         data)
{
  gfloat progress = clutter_timeline_get_progress (timeline);
  gfloat rotate   = 0;
  gint   i;

  if (progress < 0.5)
    rotate = 4 * progress * progress;
  else if (progress < 1.0)
    rotate = 2 * (progress - 0.75) * (progress - 0.75) + 7.0 / 8.0;

  for (i = 0; i < sizeof (middle) / sizeof (middle[0]); i++)
  {
    if (middle[i] != NULL)
    {
      if (rotate < 0.5)
      {
        clutter_clone_set_source (CLUTTER_CLONE (middle[i]), prev[i]);
        align (middle[i], above + i, below + i, rotate);
      }
      else
      {
        clutter_clone_set_source (CLUTTER_CLONE (middle[i]), next[i]);
        align (middle[i], below + i, above + i, rotate - 1);
      }
    }
  }
}



static void
restart (ClutterTimeline *timeline,
         gpointer         data)
{
  time_t t_t = time (NULL);

  before = *localtime (&t_t);
  before.tm_year += 1900;

  set_time ();

  if (timeline != NULL)
    g_object_unref (timeline);

  timeline = clutter_timeline_new ((gint) (1000 * FLIP_DURATION));

  g_signal_connect (timeline, "started",   G_CALLBACK (start),   NULL);
  g_signal_connect (timeline, "new-frame", G_CALLBACK (update),  NULL);
  g_signal_connect (timeline, "completed", G_CALLBACK (restart), NULL);

  clutter_timeline_set_delay (timeline, get_delay ());
  clutter_timeline_start     (timeline);

  {
    gint i;

    for (i = 0; i < sizeof (ABOVE) / sizeof (ABOVE[0]); i++)
    {
      if (middle[i] != NULL)
      {
        clutter_actor_destroy (middle[i]);

        middle[i] = NULL;
      }
    }
  }
}



static void
start_fade (ClutterTimeline *timeline,
            gpointer         data)
{
  ClutterColor  black = { 0, 0, 0, 255 };
  ClutterActor *stage = clutter_stage_get_default ();

  glass = clutter_rectangle_new_with_color (&black);

  clutter_actor_set_position  (glass, -width, -height);
  clutter_actor_set_size      (glass, 3 * width, 3 * height);
  clutter_actor_set_depth     (glass, FADE_DEPTH);

  clutter_container_add_actor (CLUTTER_CONTAINER (stage), glass);
}



static void
update_fade (ClutterTimeline *timeline,
             gint             time,
             gpointer         data)
{
  gfloat       progress = clutter_timeline_get_progress (timeline);
  gfloat       alpha    = 1 - progress;
  ClutterColor black    = { 0, 0, 0 };

  black.alpha = 255 * alpha;

  clutter_rectangle_set_color (CLUTTER_RECTANGLE (glass), &black);
}



static void
finish_fade (ClutterTimeline *timeline,
             gpointer         data)
{
  clutter_actor_destroy (glass);

  g_object_unref (timeline);
}



int
main (int   argc,
      char *argv[])
{
  ClutterColor     bg = { 0, 0, 0, 255 };
  ClutterActor    *stage;
  ClutterTimeline *fade;

  clutter_init      (&argc, &argv);
  clutterrific_init (&argc, &argv);

  cogl_set_depth_test_enabled (TRUE);

  digit   = tile  +  0;
  ampm    = digit + 10;
  month   = ampm  +  2;
  weekday = month + 12;

  stage = clutter_stage_get_default ();

  clutter_stage_set_color (CLUTTER_STAGE (stage), &bg);
  clutter_actor_show_all  (stage);
  clutter_actor_get_size  (stage, &width, &height);

  if (!import ())
  {
    g_error ("No template SVG");

    return 1;
  }

  set_time ();

  restart (NULL, NULL);

  fade = clutter_timeline_new ((gint) (1000 * FADE_DURATION));

  g_signal_connect (fade, "started",   G_CALLBACK (start_fade),  NULL);
  g_signal_connect (fade, "new-frame", G_CALLBACK (update_fade), NULL);
  g_signal_connect (fade, "completed", G_CALLBACK (finish_fade), NULL);

  clutter_timeline_start (fade);

  clutter_main ();

  return 0;
}
