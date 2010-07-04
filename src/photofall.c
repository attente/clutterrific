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



/* extensions to look for */
#define EXTENSIONS      "jpg|png"

/* pixels per metre */
#define PPM             4911.3

/* depth of overlay */
#define FADE_DEPTH       400.0

/* seconds for fade in */
#define FADE_DURATION      5.0

/* in units of screen width */
#define PAN_SPEED          0.1

/* for the physics engine */
#define ERP                0.1

#define ITERATIONS        20

/* g */
#define GRAVITY            2E-3
/* XXX: #define GRAVITY            9.8 */

#define PAPER_DENSITY      1E+3

#define ROPE_DENSITY       1E+3

#define PAPER_THICKNESS    1E-4

#define ROPE_THICKNESS     1E-2

/* number of segments per rope */
#define SEGMENTS          20

/* number of photos on stack */
#define PHOTOS             8

/* how often to add new photos */
#define POLL               3.0

/* relative photo size */
#define COVER              0.4

/* chance without drop */
#define STATIC             0.2

/* photo border in pixels */
#define BORDER            10

/* jitter of rope anchors */
#define OFFSET            20

#define W clutterrific_width  ()

#define H clutterrific_height ()



#include <ode/ode.h>

#include <clutter/clutter.h>

#include "clutterrific.h"



typedef struct
{
  dBodyID       point[SEGMENTS];

  dJointID      anchor;

  dJointGroupID joints;

  dJointID      glue;
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

gfloat        offset;

dWorldID      world;

ClutterActor *stage;

ClutterActor *glass;

Photo         photo[PHOTOS];

gint          next;



static void     create_photo  (Photo           *photo);

static void     destroy_photo (Photo           *photo);

static void     destroy_rope  (Rope            *rope);

static gboolean poll_photo    (gpointer         data);

static void     update_world  (ClutterActor    *actor,
                               gpointer         data);

static void     update_photo  (Photo           *photo);

static void     update_fade   (ClutterTimeline *timeline,
                               gint             time,
                               gpointer         data);

static void     finish_fade   (ClutterTimeline *timeline,
                               gpointer         data);



static void
create_photo (Photo *photo)
{
  gfloat w, h;

  if (photo->actor != NULL)
    destroy_photo (photo);

  if (path != NULL)
  {
    ClutterActor *image = NULL;
    ClutterActor *paper;
    gint          i;

    for (i = 0; i < path->len && image == NULL; i++)
    {
      image = clutter_texture_new_from_file (path->pdata[file++], NULL);

      if (file >= path->len)
        file = 0;
    }

    if (image != NULL)
    {
      ClutterColor c = { 255, 255, 255, 255 };
      gint         a, b;

      paper = clutter_rectangle_new_with_color (&c);
      photo->actor = clutter_group_new ();

      clutter_texture_get_base_size (CLUTTER_TEXTURE (image), &a, &b);

      w = a;
      h = b;

      clutterrific_pack (&w, &h, COVER * W, COVER * H);

      clutter_actor_set_size      (paper, w + 2 * BORDER, h + 2 * BORDER);
      clutter_actor_set_position  (image, BORDER, BORDER);
      clutter_actor_set_size      (image, w, h);
      clutter_container_add       (CLUTTER_CONTAINER (photo->actor), paper, image, NULL);
      /* XXX: clutter_container_add_actor (CLUTTER_CONTAINER (stage), photo->actor); */
    }
  }

  if (photo->actor != NULL)
  {
    gfloat x  = offset + (g_random_double () < STATIC ? W + w : g_random_double_range (w, W - w));
    gfloat y  = -h;
    gfloat z  = 0;

    gfloat x0 = x - w / 2 - BORDER / 2.0;
    gfloat y0 = y - h / 2 - BORDER / 2.0;
    gfloat z0 = z;

    gfloat x1 = x + w / 2 + BORDER / 2.0;
    gfloat y1 = y0;
    gfloat z1 = z;

    gfloat h0 = g_random_double_range (-OFFSET, OFFSET);
    gfloat h1 = g_random_double_range (-OFFSET, OFFSET);
    gfloat d0 = g_random_double_range (-OFFSET, OFFSET);
    gfloat d1 = g_random_double_range (-OFFSET, OFFSET);

    gfloat l  = g_random_double_range (2 * h, H);
    gfloat l0 = l + g_random_double_range (-OFFSET, OFFSET);
    gfloat l1 = l + g_random_double_range (-OFFSET, OFFSET);

    gfloat v0 = sqrt (l0 * l0 - h0 * h0 - d0 * d0) / (SEGMENTS - 1) / 2 * 8;
    gfloat v1 = sqrt (l1 * l1 - h1 * h1 - d1 * d1) / (SEGMENTS - 1) / 2;

    dMass mass;

    photo->body = dBodyCreate (world);

    dBodySetPosition (photo->body, x / PPM, -h / PPM, 0);
    dMassSetBox      (&mass, PAPER_DENSITY, w / PPM, h / PPM, PAPER_THICKNESS);
    dBodySetMass     (photo->body, &mass);

    {
      gint i;

      for (i = 0; i < SEGMENTS; i++)
      {
        gfloat t = 1 - (gfloat) i / (SEGMENTS - 1);

        photo->rope[0].point[i] = dBodyCreate (world);
        photo->rope[1].point[i] = dBodyCreate (world);

        dBodySetPosition (photo->rope[0].point[i], (x0 + t * h0) / PPM, y0 / PPM, (z0 + t * d0) / PPM);
        dBodySetPosition (photo->rope[1].point[i], (x1 + t * h1) / PPM, y1 / PPM, (z1 + t * d1) / PPM);

        dMassSetCylinder (&mass, ROPE_DENSITY, 2, ROPE_THICKNESS / 2.0, v0 * 2);
        dBodySetMass     (photo->rope[0].point[i], &mass);
        dMassSetCylinder (&mass, ROPE_DENSITY, 2, ROPE_THICKNESS / 2.0, v1 * 2);
        dBodySetMass     (photo->rope[1].point[i], &mass);
      }

      photo->rope[0].anchor = dJointCreateBall  (world, 0);
      photo->rope[1].anchor = dJointCreateBall  (world, 0);
      photo->rope[0].joints = dJointGroupCreate (0);
      photo->rope[1].joints = dJointGroupCreate (0);
      photo->rope[0].glue   = dJointCreateBall  (world, 0);
      photo->rope[1].glue   = dJointCreateBall  (world, 0);

      dJointAttach (photo->rope[0].anchor, 0, photo->rope[0].point[0]);
      dJointAttach (photo->rope[1].anchor, 0, photo->rope[1].point[0]);
      dJointAttach (photo->rope[0].glue, photo->rope[0].point[SEGMENTS - 1], photo->body);
      dJointAttach (photo->rope[1].glue, photo->rope[1].point[SEGMENTS - 1], photo->body);

      dJointSetBallAnchor (photo->rope[0].anchor, (x0 + h0) / PPM, y0 / PPM, (z0 + d0) / PPM);
      dJointSetBallAnchor (photo->rope[1].anchor, (x1 + h1) / PPM, y1 / PPM, (z1 + d1) / PPM);
      dJointSetBallAnchor (photo->rope[0].glue, x0 / PPM, y0 / PPM, z0 / PPM);
      dJointSetBallAnchor (photo->rope[1].glue, x1 / PPM, y1 / PPM, z1 / PPM);

      for (i = 0; i + 1 < SEGMENTS; i++)
      {
        gfloat t = 1 - (i + 0.5) / (SEGMENTS - 1);

        dJointID joint = dJointCreateBall (world, photo->rope[0].joints);
        dJointAttach (joint, photo->rope[0].point[i], photo->rope[0].point[i + 1]);
        dJointSetBallAnchor (joint, (x0 + t * h0) / PPM, (y0 + (i & 1 ? v0 : -v0)) / PPM, (z0 + t * d0) / PPM);

        joint = dJointCreateBall (world, photo->rope[1].joints);
        dJointAttach (joint, photo->rope[1].point[i], photo->rope[1].point[i + 1]);
        dJointSetBallAnchor (joint, (x1 + t * h1) / PPM, (y1 + (i & 1 ? v1 : -v1)) / PPM, (z1 + t * d1) / PPM);
      }
    }
  }
}



static void
destroy_photo (Photo *photo)
{
  dBodyDestroy (photo->body);

  destroy_rope (photo->rope + 0);
  destroy_rope (photo->rope + 1);

  clutter_actor_destroy (photo->actor);
  photo->actor = NULL;
}



static void
destroy_rope (Rope *rope)
{
  gint i;

  for (i = 0; i < SEGMENTS; i++)
    dBodyDestroy (rope->point[i]);

  dJointDestroy      (rope->anchor);
  dJointGroupDestroy (rope->joints);
  dJointDestroy      (rope->glue);
}



static gboolean
poll_photo (gpointer data)
{
  gint i;

  for (i = 0; i < PHOTOS; i++)
  {
    gfloat x;

    if (photo[i].actor == NULL)
      continue;

    x = dBodyGetPosition (photo[i].body)[0];

    if (x * PPM < offset - COVER * W)
      destroy_photo (photo + i);
  }

  for (i = 0; i < PHOTOS; i++)
  {
    if (photo[i].actor == NULL)
    {
      create_photo (photo + i);

      break;
    }
  }

  return TRUE;
}



/* XXX */
static void
draw_body (dBodyID body)
{
  const dReal *p = dBodyGetPosition (body);
  gfloat x = p[0] * PPM - offset;
  gfloat y = p[1] * PPM;
  gfloat s = 1;

  x = x / s + (W - W / s) / 2;
  y = y / s + (H - H / s) / 2;

  cogl_rectangle (x - 3, y - 3, x + 3, y + 3);

  /* cogl_rectangle (p[0] * PPM - offset - 2, p[1] * PPM - 2, p[0] * PPM - offset + 2, p[1] * PPM + 2); */
}
/* XXX */
static gboolean
asdf (gpointer null)
{
  clutter_actor_queue_redraw (stage);
  return TRUE;
}



static void
update_world (ClutterActor *actor,
              gpointer      data)
{
  gdouble delta = clutterrific_delta ();

  offset += delta * W * PAN_SPEED;

  /* XXX: dWorldQuickStep (world, delta); */
  dWorldQuickStep (world, 1);

  {
    gint i;

    for (i = 0; i < PHOTOS; i++)
      if (photo->actor != NULL)
        update_photo (photo + i);
  }

  {
    gint i;

    cogl_set_source_color4f (0, 0, 0, 1);

    for (i = 0; i < PHOTOS; i++)
      if (photo[i].actor != NULL)
        draw_body (photo[i].body);

    cogl_set_source_color4f (0, 0, 1, 1);

    for (i = 0; i < PHOTOS; i++)
      if (photo[i].actor != NULL)
      {
        gint j;

        for (j = 0; j < SEGMENTS; j++)
        {
          draw_body (photo[i].rope[0].point[j]);
          draw_body (photo[i].rope[1].point[j]);
        }
      }
  }
}



static void
update_photo (Photo *photo)
{
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

  g_timeout_add ((guint) (1000 * POLL), poll_photo, NULL);
  /* XXX */ g_timeout_add (10, asdf, NULL);
  g_signal_connect_after (stage, "paint", G_CALLBACK (update_world), NULL);

  g_object_unref (timeline);
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
  next   = 0;

  dInitODE ();
  world = dWorldCreate ();
  dWorldSetERP (world, ERP);
  dWorldSetCFM (world, 0);
  dWorldSetGravity (world, 0, GRAVITY, 0);
  dWorldSetQuickStepNumIterations (world, ITERATIONS);

  {
    ClutterColor black = {   0,   0,   0, 255 };
    ClutterColor white = { 255, 255, 255, 255 };

    stage = clutter_stage_get_default        ();
    glass = clutter_rectangle_new_with_color (&black);

    clutter_stage_set_color     (CLUTTER_STAGE (stage), &white);
    clutter_actor_set_position  (glass, -W, -H);
    clutter_actor_set_size      (glass, 3 * W, 3 * H);
    clutter_actor_set_depth     (glass, FADE_DEPTH);
    clutter_container_add_actor (CLUTTER_CONTAINER (stage), glass);
    clutter_actor_show_all      (stage);
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
