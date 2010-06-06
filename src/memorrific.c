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



#define SCHEMES                    9

#define FADE_TIME               5000

#define MIN_THEME_TIME         20000

#define MAX_THEME_TIME         30000

#define STYLES                     1

#define SWIPE_STYLE_SWATCHES      16

#define SWIPE_STYLE_MEMORIES       4

#define SWIPE_STYLE_MIN_TIME   20000

#define SWIPE_STYLE_MAX_TIME   30000

#define SWIPE_STYLE_MIN_WAIT    1000

#define SWIPE_STYLE_MAX_WAIT   20000

#define SWIPE_STYLE_MIN_LIFE    3000

#define SWIPE_STYLE_MAX_LIFE    7000

#define SWIPE_STYLE_MIN_SIZE       0.2

#define SWIPE_STYLE_MAX_SIZE       0.4

#define SWIPE_STYLE_MIN_RANGE      0.6

#define SWIPE_STYLE_MAX_RANGE      1.2

#define SWIPE_STYLE_MIN_FULL       0.5

#define SWIPE_STYLE_MAX_FULL       1.0

#define SWIPE_STYLE_MIN_OFFSET     0.0

#define SWIPE_STYLE_MAX_OFFSET     0.0

#define SWIPE_STYLE_MIN_CHANGE     0.2

#define SWIPE_STYLE_MAX_CHANGE     0.2



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



/*
 * Each style is responsible for restarting the timeline on completion.
 */

typedef struct
{
  GCallback start;

  GCallback update;

  GCallback finish;

  gfloat    min_time;

  gfloat    max_time;
}
Style;



typedef struct
{
  gfloat x[2];

  gfloat y[2];
}
Rectangle;



typedef struct
{
  Rectangle r[4];

  gfloat    z[2];

  gfloat    t[2];
}
Frame;



typedef struct
{
  Frame            frame;

  ClutterActor    *actor;

  ClutterTimeline *owner;
}
Swatch;



typedef struct
{
  Swatch    swatch;

  Rectangle start;

  Rectangle finish;
}
Memory;



static gfloat       width;

static gfloat       height;

static ColourScheme SCHEME[SCHEMES];

static FineColour   stage_colour;

static FineColour   target_colour;

static ColourScheme scheme;

static FineColour   scheme_colour;

static gfloat       scheme_parameter;

static Style        STYLE[STYLES];



static FineColour   get_random_achromatic_colour    (const FineColour *base,
                                                     const FineColour *target,
                                                     gfloat            parameter);

static FineColour   get_random_monochromatic_colour (const FineColour *base,
                                                     const FineColour *target,
                                                     gfloat            parameter);

static FineColour   get_random_analogous_colour     (const FineColour *base,
                                                     const FineColour *target,
                                                     gfloat            parameter);

static FineColour   get_random_complementary_colour (const FineColour *base,
                                                     const FineColour *target,
                                                     gfloat            parameter);

static FineColour   get_random_triadic_colour       (const FineColour *base,
                                                     const FineColour *target,
                                                     gfloat            parameter);

static FineColour   get_random_tetradic_colour      (const FineColour *base,
                                                     const FineColour *target,
                                                     gfloat            parameter);

static FineColour   get_random_neutral_colour       (const FineColour *base,
                                                     const FineColour *target,
                                                     gfloat            parameter);

static FineColour   get_random_warm_colour          (const FineColour *base,
                                                     const FineColour *target,
                                                     gfloat            parameter);

static FineColour   get_random_cool_colour          (const FineColour *base,
                                                     const FineColour *target,
                                                     gfloat            parameter);

static void         swipe_style_start               (ClutterTimeline  *timeline,
                                                     gpointer          data);

static void         swipe_style_finish              (ClutterTimeline  *timeline,
                                                     gpointer          data);

static void         swipe_style_create_swatch       (gint              wait,
                                                     gint              life,
                                                     ClutterTimeline  *timeline);

static void         swipe_style_create_memory       (gint              wait,
                                                     gint              life,
                                                     ClutterTimeline  *timeline);

static void         swipe_style_start_swatch        (ClutterTimeline  *timeline,
                                                     gpointer          swatch);

static void         swipe_style_start_memory        (ClutterTimeline  *timeline,
                                                     gpointer          memory);

static void         swipe_style_repeat_swatch       (ClutterTimeline  *timeline,
                                                     gpointer          swatch);

static void         swipe_style_repeat_memory       (ClutterTimeline  *timeline,
                                                     gpointer          memory);

static Frame        swipe_style_random_frame        (void);

static void         swap                            (gfloat           *x,
                                                     gfloat           *y);

static gdouble      get_delta                       (GTimeVal         *time);

static gint         get_remaining_time              (ClutterTimeline  *timeline);

static ClutterColor get_colour                      (const FineColour *data);

static gfloat       get_width                       (const Rectangle  *r);

static gfloat       get_height                      (const Rectangle  *r);

static Rectangle    interpolate                     (const Rectangle  *r0,
                                                     const Rectangle  *r1,
                                                     gfloat            t);

static Rectangle    get_frame                       (const Frame      *f,
                                                     gfloat            t);

static Rectangle    get_frame_extent                (const Frame      *f);

static gfloat       get_depth                       (const Frame      *f,
                                                     gfloat            t);

static void         select_theme                    (ClutterTimeline  *timeline,
                                                     gpointer          data);

static void         update_theme                    (ClutterTimeline  *timeline,
                                                     gint              time,
                                                     gpointer          data);

static void         start_theme                     (ClutterTimeline  *timeline,
                                                     gpointer          data);

static void         start_style                     (ClutterTimeline   *timeline,
                                                     const Style       *style);

static void         update_swatch                   (ClutterTimeline  *timeline,
                                                     gint              time,
                                                     gpointer          swatch);

static void         update_memory                   (ClutterTimeline  *timeline,
                                                     gint              time,
                                                     gpointer          memory);



static FineColour
get_random_achromatic_colour (const FineColour *base,
                              const FineColour *target,
                              gfloat            parameter)
{
  FineColour colour;

  colour.h = 0;
  colour.s = 0;
  colour.a = 1;

  if (base == NULL || target == NULL)
    colour.l = (gint) (parameter + 0.5);
  else if (parameter < 0.5)
    colour.l = g_random_double_range (0.5, 1.0);
  else
    colour.l = g_random_double_range (0.0, 0.5);

  return colour;
}



static FineColour
get_random_monochromatic_colour (const FineColour *base,
                                 const FineColour *target,
                                 gfloat            parameter)
{
  FineColour colour;

  colour.h = base == NULL ? g_random_double () : base->h;
  colour.s = 1;
  colour.a = 1;

  if (base == NULL)
    colour.l = (gint) (3.0 * parameter + 0.5) / 3.0;
  else if (base != NULL && target == NULL)
    colour.l = base->l;
  else if (base->l < 0.5)
    colour.l = g_random_double_range (0.5, 1.0);
  else
    colour.l = g_random_double_range (0.2, 0.5);

  return colour;
}



static FineColour
get_random_analogous_colour (const FineColour *base,
                             const FineColour *target,
                             gfloat            parameter)
{
  FineColour colour;

  colour.h = g_random_double ();
  colour.s = 1;
  colour.l = g_random_double ();
  colour.a = 1;

  return colour;
}



static FineColour
get_random_complementary_colour (const FineColour *base,
                                 const FineColour *target,
                                 gfloat            parameter)
{
  FineColour colour;

  colour.h = g_random_double ();
  colour.s = 1;
  colour.l = g_random_double ();
  colour.a = 1;

  return colour;
}



static FineColour
get_random_triadic_colour (const FineColour *base,
                           const FineColour *target,
                           gfloat            parameter)
{
  FineColour colour;

  colour.h = g_random_double ();
  colour.s = 1;
  colour.l = g_random_double ();
  colour.a = 1;

  return colour;
}



static FineColour
get_random_tetradic_colour (const FineColour *base,
                            const FineColour *target,
                            gfloat            parameter)
{
  FineColour colour;

  colour.h = g_random_double ();
  colour.s = 1;
  colour.l = g_random_double ();
  colour.a = 1;

  return colour;
}



static FineColour
get_random_neutral_colour (const FineColour *base,
                           const FineColour *target,
                           gfloat            parameter)
{
  FineColour colour;

  colour.h = g_random_double ();
  colour.s = 1;
  colour.l = g_random_double ();
  colour.a = 1;

  return colour;
}



static FineColour
get_random_warm_colour (const FineColour *base,
                        const FineColour *target,
                        gfloat            parameter)
{
  FineColour colour;

  colour.h = g_random_double ();
  colour.s = 1;
  colour.l = g_random_double ();
  colour.a = 1;

  return colour;
}



static FineColour
get_random_cool_colour (const FineColour *base,
                        const FineColour *target,
                        gfloat            parameter)
{
  FineColour colour;

  colour.h = g_random_double ();
  colour.s = 1;
  colour.l = g_random_double ();
  colour.a = 1;

  return colour;
}



static void
swipe_style_start (ClutterTimeline *timeline,
                   gpointer         data)
{
  gint wait;
  gint life;

  gint i;

  for (i = 0; i < SWIPE_STYLE_SWATCHES; i++)
  {
    wait = g_random_int_range (SWIPE_STYLE_MIN_WAIT, SWIPE_STYLE_MAX_WAIT + 1);
    life = g_random_int_range (SWIPE_STYLE_MIN_LIFE, SWIPE_STYLE_MAX_LIFE + 1);

    swipe_style_create_swatch (wait, life, timeline);
  }

  for (i = 0; i < SWIPE_STYLE_MEMORIES; i++)
  {
    wait = g_random_int_range (SWIPE_STYLE_MIN_WAIT, SWIPE_STYLE_MAX_WAIT + 1);
    life = g_random_int_range (SWIPE_STYLE_MIN_LIFE, SWIPE_STYLE_MAX_LIFE + 1);

    swipe_style_create_memory (wait, life, timeline);
  }
}



static void
swipe_style_finish (ClutterTimeline *timeline,
                    gpointer         data)
{
  start_style (timeline, STYLE + g_random_int_range (0, STYLES));
}



static void
swipe_style_create_swatch (gint             wait,
                           gint             life,
                           ClutterTimeline *timeline)
{
  gint time = get_remaining_time (timeline);

  if (wait + life < time)
  {
    ClutterTimeline *thread = clutter_timeline_new (life);
    Swatch          *swatch = g_new (Swatch, 1);

    swatch->owner = timeline;

    clutter_timeline_set_delay (thread, wait);

    g_signal_connect       (thread, "started",   G_CALLBACK (swipe_style_start_swatch),  swatch);
    g_signal_connect       (thread, "new-frame", G_CALLBACK (update_swatch),             swatch);
    g_signal_connect_after (thread, "completed", G_CALLBACK (swipe_style_repeat_swatch), swatch);

    clutter_timeline_start (thread);
  }
}



static void
swipe_style_create_memory (gint             wait,
                           gint             life,
                           ClutterTimeline *timeline)
{
  gint time = get_remaining_time (timeline);

  return; /* XXX */

  if (wait + life < time)
  {
    ClutterTimeline *thread = clutter_timeline_new (life);
    Memory          *memory = g_new (Memory, 1);

    memory->swatch.owner = timeline;

    clutter_timeline_set_delay (thread, wait);

    g_signal_connect       (thread, "started",   G_CALLBACK (swipe_style_start_memory),  memory);
    g_signal_connect       (thread, "new-frame", G_CALLBACK (update_memory),             memory);
    g_signal_connect_after (thread, "completed", G_CALLBACK (swipe_style_repeat_memory), memory);

    clutter_timeline_start (thread);
  }
}



static void
swipe_style_start_swatch (ClutterTimeline *timeline,
                          gpointer         swatch)
{
  FineColour    colour = scheme (&scheme_colour, &target_colour, scheme_parameter);
  ClutterColor  color  = get_colour (&colour);
  ClutterActor *stage  = clutter_stage_get_default ();
  Swatch       *object = swatch;

  object->frame = swipe_style_random_frame ();
  object->actor = clutter_rectangle_new_with_color (&color);

  clutter_actor_hide          (object->actor);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), object->actor);
}



static void
swipe_style_start_memory (ClutterTimeline *timeline,
                          gpointer         memory)
{
  ClutterActor *stage  = clutter_stage_get_default ();
  Memory       *object = memory;
  Rectangle     extent;

  object->swatch.frame = swipe_style_random_frame ();
  extent               = get_frame_extent (&object->swatch.frame);
  /* XXX: Initialize the actor and start/finish rectangles */

  clutter_actor_hide          (object->swatch.actor);
  clutter_container_add_actor (CLUTTER_CONTAINER (stage), object->swatch.actor);
}



static void
swipe_style_repeat_swatch (ClutterTimeline *timeline,
                           gpointer         swatch)
{
  Swatch          *object = swatch;
  ClutterTimeline *owner  = object->owner;

  clutter_actor_destroy (object->actor);
  g_free                (object);
  g_object_unref        (timeline);

  {
    gint wait = g_random_int_range (SWIPE_STYLE_MIN_WAIT, SWIPE_STYLE_MAX_WAIT + 1);
    gint life = g_random_int_range (SWIPE_STYLE_MIN_LIFE, SWIPE_STYLE_MAX_LIFE + 1);

    swipe_style_create_swatch (wait, life, owner);
  }
}



static void
swipe_style_repeat_memory (ClutterTimeline *timeline,
                           gpointer         memory)
{
  Memory          *object = memory;
  ClutterTimeline *owner  = object->swatch.owner;

  clutter_actor_destroy (object->swatch.actor);
  g_free                (object);
  g_object_unref        (timeline);

  {
    gint wait = g_random_int_range (SWIPE_STYLE_MIN_WAIT, SWIPE_STYLE_MAX_WAIT + 1);
    gint life = g_random_int_range (SWIPE_STYLE_MIN_LIFE, SWIPE_STYLE_MAX_LIFE + 1);

    swipe_style_create_memory (wait, life, owner);
  }
}



static Frame
swipe_style_random_frame (void)
{
  Frame frame;

  if (g_random_boolean ())
  {
    frame.r[0].y[0] = height * g_random_double_range (-SWIPE_STYLE_MIN_SIZE, 1);
    frame.r[1].y[0] = frame.r[0].y[0];
    frame.r[2].y[0] = frame.r[0].y[0];
    frame.r[3].y[0] = frame.r[0].y[0];

    frame.r[0].y[1] = frame.r[0].y[0] + height * g_random_double_range (SWIPE_STYLE_MIN_SIZE, SWIPE_STYLE_MAX_SIZE);
    frame.r[1].y[1] = frame.r[0].y[1];
    frame.r[2].y[1] = frame.r[0].y[1];
    frame.r[3].y[1] = frame.r[0].y[1];

    {
      gfloat x[6];

      x[0] = g_random_double_range (0, width);
      x[1] = height * g_random_double_range (SWIPE_STYLE_MIN_RANGE,
                                             SWIPE_STYLE_MAX_RANGE);
      x[1] = x[0] + (g_random_boolean () ? x[1] : -x[1]);

      if (g_random_boolean ())
        swap (x + 0, x + 1);

      x[2] = g_random_double_range (SWIPE_STYLE_MIN_OFFSET,
                                    SWIPE_STYLE_MAX_OFFSET);
      x[3] = g_random_double_range (SWIPE_STYLE_MIN_OFFSET,
                                    SWIPE_STYLE_MAX_OFFSET);
      x[2] = x[0] + x[2] * (x[1] - x[0]);
      x[3] = x[1] + x[3] * (x[0] - x[1]);

      x[4] = g_random_double_range (SWIPE_STYLE_MIN_FULL,
                                    SWIPE_STYLE_MAX_FULL);
      x[5] = g_random_double_range (SWIPE_STYLE_MIN_FULL,
                                    SWIPE_STYLE_MAX_FULL);
      x[4] = x[2] + x[4] * (x[3] - x[2]);
      x[5] = x[3] + x[5] * (x[2] - x[3]);

      frame.r[0].x[0] = x[0];
      frame.r[0].x[1] = x[0];
      frame.r[1].x[0] = x[2];
      frame.r[1].x[1] = x[4];
      frame.r[2].x[0] = x[5];
      frame.r[2].x[1] = x[3];
      frame.r[3].x[0] = x[1];
      frame.r[3].x[1] = x[1];
    }

    frame.t[0] =     g_random_double_range (SWIPE_STYLE_MIN_CHANGE,
                                            SWIPE_STYLE_MAX_CHANGE);
    frame.t[1] = 1 - g_random_double_range (SWIPE_STYLE_MIN_CHANGE,
                                            SWIPE_STYLE_MAX_CHANGE);
  }
  else
  {
    frame.r[0].x[0] = g_random_double_range (-SWIPE_STYLE_MIN_SIZE * height, width);
    frame.r[1].x[0] = frame.r[0].x[0];
    frame.r[2].x[0] = frame.r[0].x[0];
    frame.r[3].x[0] = frame.r[0].x[0];

    frame.r[0].x[1] = frame.r[0].x[0] + g_random_double_range (SWIPE_STYLE_MIN_SIZE * height, SWIPE_STYLE_MAX_SIZE * height);
    frame.r[1].x[1] = frame.r[0].x[1];
    frame.r[2].x[1] = frame.r[0].x[1];
    frame.r[3].x[1] = frame.r[0].x[1];

    {
      gfloat y[6];

      y[0] = g_random_double_range (0, height);
      y[1] = height * g_random_double_range (SWIPE_STYLE_MIN_RANGE,
                                             SWIPE_STYLE_MAX_RANGE);
      y[1] = y[0] + (g_random_boolean () ? y[1] : -y[1]);

      if (g_random_boolean ())
        swap (y + 0, y + 1);

      y[2] = g_random_double_range (0, SWIPE_STYLE_MAX_OFFSET);
      y[3] = g_random_double_range (0, SWIPE_STYLE_MAX_OFFSET);
      y[2] = y[0] + y[2] * (y[1] - y[0]);
      y[3] = y[1] + y[3] * (y[0] - y[1]);

      y[4] = g_random_double_range (SWIPE_STYLE_MIN_FULL,
                                    SWIPE_STYLE_MAX_FULL);
      y[5] = g_random_double_range (SWIPE_STYLE_MIN_FULL,
                                    SWIPE_STYLE_MAX_FULL);
      y[4] = y[2] + y[4] * (y[3] - y[2]);
      y[5] = y[3] + y[5] * (y[2] - y[3]);

      frame.r[0].y[0] = y[0];
      frame.r[0].y[1] = y[0];
      frame.r[1].y[0] = y[2];
      frame.r[1].y[1] = y[4];
      frame.r[2].y[0] = y[5];
      frame.r[2].y[1] = y[3];
      frame.r[3].y[0] = y[1];
      frame.r[3].y[1] = y[1];
    }

    frame.t[0] =     g_random_double_range (SWIPE_STYLE_MIN_CHANGE,
                                            SWIPE_STYLE_MAX_CHANGE);
    frame.t[1] = 1 - g_random_double_range (SWIPE_STYLE_MIN_CHANGE,
                                            SWIPE_STYLE_MAX_CHANGE);
  }

  return frame;
}



static void
swap (gfloat *x,
      gfloat *y)
{
  gfloat t = *x;

  *x = *y;
  *y = t;
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



static gint
get_remaining_time (ClutterTimeline *timeline)
{
  if (!CLUTTER_IS_TIMELINE (timeline))
    return 0;

  return clutter_timeline_get_duration     (timeline)
       - clutter_timeline_get_elapsed_time (timeline);
}



static ClutterColor
get_colour (const FineColour *data)
{
  ClutterColor colour;

  clutter_color_from_hls (&colour, 360 * data->h, data->l, data->s);

  colour.alpha = CLAMP (256 * data->a, 0, 255);

  return colour;
}



static gfloat
get_width (const Rectangle *r)
{
  return r->x[1] - r->x[0];
}



static gfloat
get_height (const Rectangle *r)
{
  return r->y[1] - r->y[0];
}



static Rectangle
interpolate (const Rectangle *r0,
             const Rectangle *r1,
             gfloat           t)
{
  Rectangle r;

  r.x[0] = r0->x[0] + t * (r1->x[0] - r0->x[0]);
  r.y[0] = r0->y[0] + t * (r1->y[0] - r0->y[0]);
  r.x[1] = r0->x[1] + t * (r1->x[1] - r0->x[1]);
  r.y[1] = r0->y[1] + t * (r1->y[1] - r0->y[1]);

  return r;
}



static Rectangle
get_frame (const Frame *f,
           gfloat       t)
{
  const Rectangle *r0;
  const Rectangle *r1;

  if (t < f->t[0])
  {
    r0 = f->r + 0;
    r1 = f->r + 1;

    t = (t - 0) / (f->t[0] - 0);
  }
  else if (t < f->t[1])
  {
    r0 = f->r + 1;
    r1 = f->r + 2;

    t = (t - f->t[0]) / (f->t[1] - f->t[0]);
  }
  else
  {
    r0 = f->r + 2;
    r1 = f->r + 3;

    t = (t - f->t[1]) / (1 - f->t[1]);
  }

  return interpolate (r0, r1, t);
}



static Rectangle
get_frame_extent (const Frame *f)
{
  Rectangle r;

  r.x[0] = MIN (f->r[0].x[0], f->r[0].x[1]);
  r.x[0] = MIN (r.x[0],       f->r[1].x[0]);
  r.x[0] = MIN (r.x[0],       f->r[1].x[1]);
  r.x[0] = MIN (r.x[0],       f->r[2].x[0]);
  r.x[0] = MIN (r.x[0],       f->r[2].x[1]);
  r.x[0] = MIN (r.x[0],       f->r[3].x[0]);
  r.x[0] = MIN (r.x[0],       f->r[3].x[1]);

  r.x[1] = MAX (f->r[0].x[0], f->r[0].x[1]);
  r.x[1] = MAX (r.x[1],       f->r[1].x[0]);
  r.x[1] = MAX (r.x[1],       f->r[1].x[1]);
  r.x[1] = MAX (r.x[1],       f->r[2].x[0]);
  r.x[1] = MAX (r.x[1],       f->r[2].x[1]);
  r.x[1] = MAX (r.x[1],       f->r[3].x[0]);
  r.x[1] = MAX (r.x[1],       f->r[3].x[1]);

  r.y[0] = MIN (f->r[0].y[0], f->r[0].y[1]);
  r.y[0] = MIN (r.y[0],       f->r[1].y[0]);
  r.y[0] = MIN (r.y[0],       f->r[1].y[1]);
  r.y[0] = MIN (r.y[0],       f->r[2].y[0]);
  r.y[0] = MIN (r.y[0],       f->r[2].y[1]);
  r.y[0] = MIN (r.y[0],       f->r[3].y[0]);
  r.y[0] = MIN (r.y[0],       f->r[3].y[1]);

  r.y[1] = MAX (f->r[0].y[0], f->r[0].y[1]);
  r.y[1] = MAX (r.y[1],       f->r[1].y[0]);
  r.y[1] = MAX (r.y[1],       f->r[1].y[1]);
  r.y[1] = MAX (r.y[1],       f->r[2].y[0]);
  r.y[1] = MAX (r.y[1],       f->r[2].y[1]);
  r.y[1] = MAX (r.y[1],       f->r[3].y[0]);
  r.y[1] = MAX (r.y[1],       f->r[3].y[1]);

  return r;
}



static gfloat
get_depth (const Frame *f,
           gfloat       t)
{
  return f->z[0] + t * (f->z[1] - f->z[0]);
}



static void
select_theme (ClutterTimeline *timeline,
              gpointer         data)
{
  scheme           = SCHEME[g_random_int_range (0, /* XXX: SCHEMES */ 2)];
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
start_theme (ClutterTimeline *timeline,
             gpointer         data)
{
  if (timeline != NULL)
    g_object_unref (timeline);

  timeline = clutter_timeline_new (g_random_int_range (MIN_THEME_TIME, MAX_THEME_TIME + 1));

  g_signal_connect       (timeline, "started",   G_CALLBACK (select_theme), NULL);
  g_signal_connect       (timeline, "new-frame", G_CALLBACK (update_theme), NULL);
  g_signal_connect_after (timeline, "completed", G_CALLBACK (start_theme),  NULL);

  clutter_timeline_start (timeline);
}



static void
start_style (ClutterTimeline *timeline,
             const Style     *style)
{
  if (timeline != NULL)
    g_object_unref (timeline);

  timeline = clutter_timeline_new (g_random_int_range (style->min_time, style->max_time + 1));

  if (style->start != NULL)
    g_signal_connect (timeline, "started", style->start, NULL);

  if (style->update != NULL)
    g_signal_connect (timeline, "new-frame", style->update, NULL);

  if (style->finish != NULL)
    g_signal_connect_after (timeline, "completed", style->finish, NULL);

  clutter_timeline_start (timeline);
}



static void
update_swatch (ClutterTimeline *timeline,
               gint             time,
               gpointer         swatch)
{
  Swatch    *object = swatch;
  gfloat     ratio  = clutter_timeline_get_progress (timeline);
  Rectangle  frame  = get_frame (&object->frame, ratio);

  if (frame.x[0] > frame.x[1])
    swap (frame.x + 0, frame.x + 1);
  if (frame.y[0] > frame.y[1])
    swap (frame.y + 0, frame.y + 1);

  if (object->actor != NULL)
  {
    clutter_actor_set_position (object->actor,
                                frame.x[0],
                                frame.y[0]);
    clutter_actor_set_size     (object->actor,
                                get_width  (&frame),
                                get_height (&frame));
    clutter_actor_show         (object->actor);
  }
}



static void
update_memory (ClutterTimeline *timeline,
               gint             time,
               gpointer         memory)
{
  Memory    *object = memory;
  gfloat     ratio  = clutter_timeline_get_progress (timeline);
  Rectangle  canvas = interpolate (&object->start, &object->finish, ratio);
  Rectangle  frame  = get_frame (&object->swatch.frame, ratio);

  if (object->swatch.actor != NULL)
  {
    clutter_actor_set_position (object->swatch.actor,
                                canvas.x[0],
                                canvas.y[0]);
    clutter_actor_set_size     (object->swatch.actor,
                                get_width  (&canvas),
                                get_height (&canvas));
    clutter_actor_set_clip     (object->swatch.actor,
                                frame.x[0] - canvas.x[0],
                                frame.y[0] - canvas.y[0],
                                get_width  (&frame),
                                get_height (&frame));
    clutter_actor_show         (object->swatch.actor);
  }
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

  STYLE[0].start    = G_CALLBACK (swipe_style_start);
  STYLE[0].update   = NULL;
  STYLE[0].finish   = G_CALLBACK (swipe_style_finish);
  STYLE[0].min_time = SWIPE_STYLE_MIN_TIME;
  STYLE[0].max_time = SWIPE_STYLE_MAX_TIME;

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

  start_theme (NULL, NULL);
  start_style (NULL, STYLE);

  clutter_main ();

  return 0;
}
