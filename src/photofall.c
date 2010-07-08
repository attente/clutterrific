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



#define EXTS   "jpg|png"

#define ROPE   20

#define STEP    1E-2

#define G       1E-4

#define PPM     5E+3

#define SPACE   6E-1

#define EDGE    5

#define W      clutterrific_width  ()

#define H      clutterrific_height ()

#define U      MIN (W, H)

#define V      MAX (W, H)



#include <ode/ode.h>

#include <clutter/clutter.h>

#include "clutterrific.h"



typedef struct
{
  dBodyID  body[ROPE];

  dJointID nail;

  dJointID glue[ROPE];
}
Rope;



typedef struct
{
  dBodyID       body;

  ClutterActor *actor;

  Rope          rope[2];
}
Photo;



static GPtrArray    *file;

static gint          next;

static gfloat        shift;

static dWorldID      world;

static ClutterActor *stage;

static Photo photo; /* XXX */



static void     create_photo  (Photo    *photo);

static void     destroy_photo (Photo    *photo);

static void     destroy_rope  (Rope     *rope);

static gboolean update_world  (gpointer  data);



static void
create_photo (Photo *photo)
{
  gfloat w;
  gfloat h;

  if (photo->body != NULL)
    destroy_photo (photo);

  if (file == NULL)
    return;

  {
    ClutterActor *image = NULL;
    gint          i;

    for (i = 0; image == NULL && i < file->len; i++)
    {
      image = clutter_texture_new_from_file (file->pdata[next++], NULL);

      if (next >= file->len)
        next = 0;
    }

    if (image == NULL)
      return;

    {
      ClutterColor  c = { 224, 224, 224, 255 };
      ClutterActor *group;
      ClutterActor *paper;
      gint          a;
      gint          b;

      photo->actor = clutter_group_new ();
      group        = clutter_group_new ();
      paper        = clutter_rectangle_new_with_color (&c);

      clutter_texture_get_base_size (CLUTTER_TEXTURE (image), &a, &b);

      w = a;
      h = b;

      clutterrific_pack (&w, &h, SPACE * MIN (W, H), SPACE * MIN (W, H));

      clutter_actor_set_size      (paper, w + 4 * EDGE, h + 4 * EDGE);
      clutter_actor_set_position  (image, 2 * EDGE, 2 * EDGE);
      clutter_actor_set_size      (image, w, h);
      clutter_container_add_actor (CLUTTER_CONTAINER (group), paper);
      clutter_container_add_actor (CLUTTER_CONTAINER (group), image);
      clutter_container_add_actor (CLUTTER_CONTAINER (photo->actor), group);
    }
  }

  {
    gfloat x = shift + 0.1 * U + 2 * EDGE + w / 2 + g_random_double_range (0, W);
    gfloat y = -0.1 * U - 2 * EDGE - h / 2;
    gfloat z = 0.1 * U * g_random_double_range (-1, 1);

    gfloat x0 = x - w / 2 - EDGE;
    gfloat y0 = y - h / 2 - EDGE;
    gfloat z0 = z;

    gfloat x1 = x + w / 2 + EDGE;
    gfloat y1 = y0;
    gfloat z1 = z;

    gfloat l = g_random_double_range (2 * EDGE + h / 2, H - 2 * EDGE - h / 2) - y0;
    gfloat l0 = l + 0.1 * U * g_random_double_range (0, 1);
    gfloat l1 = l + 0.1 * U * g_random_double_range (0, 1);

    gfloat dx0 = 0.1 * U * g_random_double_range (-1, 1);
    gfloat dy0 = 0.1 * U * g_random_double_range (-1, 0);
    gfloat dz0 = 0.1 * U * g_random_double_range (-1, 1);

    gfloat dx1 = 0.1 * U * g_random_double_range (-1, 1);
    gfloat dy1 = 0.1 * U * g_random_double_range (-1, 0);
    gfloat dz1 = 0.1 * U * g_random_double_range (-1, 1);

    dMass mass;

    photo->body = dBodyCreate (world);
    dBodySetPosition (photo->body, x / PPM, y / PPM, z / PPM);
    dMassSetBox (&mass, 1E+4, w / PPM, h / PPM, 1E-2);
    dBodySetMass (photo->body, &mass);

    {
      Rope *rope = photo->rope;
      gint  i;

      for (i = 0; i < ROPE / 2; i++)
      {
        rope[0].body[i] = dBodyCreate (world);
        rope[1].body[i] = dBodyCreate (world);
      }

      for (i = ROPE / 2; i < ROPE; i++)
      {
        rope[0].body[i] = dBodyCreate (world);
        rope[1].body[i] = dBodyCreate (world);
      }

      for (i = 0; i < ROPE; i++)
      {
        gfloat t  = (gfloat) i / (ROPE - 1);
        gfloat u0 = (x0 + (1 - t) * dx0) / PPM;
        gfloat v0 = (y1 + (1 - t) * dy0) / PPM;
        gfloat w0 = (z + (1 - t) * dz0) / PPM;
        gfloat u1 = (x1 + (1 - t) * dx1) / PPM;
        gfloat v1 = (y1 + (1 - t) * dy1) / PPM;
        gfloat w1 = (z + (1 - t) * dz1) / PPM;

        rope[0].body[i] = dBodyCreate (world);
        dBodySetPosition (rope[0].body[i], u0, v0, w0);
        dMassSetSphere (&mass, 1E+3, 1E-1);
        dBodySetMass (rope[0].body[i], &mass);

        rope[1].body[i] = dBodyCreate (world);
        dBodySetPosition (rope[1].body[i], u1, v1, w1);
        dMassSetSphere (&mass, 1E+3, 1E-1);
        dBodySetMass (rope[1].body[i], &mass);
      }

      rope[0].nail = dJointCreateBall (world, 0);
      rope[1].nail = dJointCreateBall (world, 0);
      rope[0].glue[ROPE - 1] = dJointCreateBall (world, 0);
      rope[1].glue[ROPE - 1] = dJointCreateBall (world, 0);
      dJointAttach (rope[0].nail, 0, rope[0].body[0]);
      dJointAttach (rope[1].nail, 0, rope[1].body[0]);
      dJointAttach (rope[0].glue[ROPE - 1], rope[0].body[ROPE - 1], photo->body);
      dJointAttach (rope[1].glue[ROPE - 1], rope[1].body[ROPE - 1], photo->body);
      dJointSetBallAnchor (rope[0].nail, (x0 + dx0) / PPM, (y1 + dy0) / PPM, (z + dz0) / PPM);
      dJointSetBallAnchor (rope[1].nail, (x1 + dx1) / PPM, (y1 + dy1) / PPM, (z + dz1) / PPM);
      dJointSetBallAnchor (rope[0].glue[ROPE - 1], x0 / PPM, y1 / PPM, z / PPM);
      dJointSetBallAnchor (rope[1].glue[ROPE - 1], x1 / PPM, y1 / PPM, z / PPM);

      for (i = 0; i + 1 < ROPE; i++)
      {
        gfloat t  = (i + 0.5) / (ROPE - 1);
        gfloat u0 = (x0 + (1 - t) * dx0) / PPM;
        gfloat v0 = (y1 + (1 - t) * dy0) / PPM;
        gfloat w0 = (z + (1 - t) * dz0) / PPM;
        gfloat u1 = (x1 + (1 - t) * dx1) / PPM;
        gfloat v1 = (y1 + (1 - t) * dy1) / PPM;
        gfloat w1 = (z + (1 - t) * dz1) / PPM;

        rope[0].glue[i] = dJointCreateBall (world, 0);
        dJointAttach (rope[0].glue[i], rope[0].body[i], rope[0].body[i + 1]);
        dJointSetBallAnchor (rope[0].glue[i], u0, v0, w0);

        rope[1].glue[i] = dJointCreateBall (world, 0);
        dJointAttach (rope[1].glue[i], rope[1].body[i], rope[1].body[i + 1]);
        dJointSetBallAnchor (rope[1].glue[i], u1, v1, w1);
      }
    }
  }
}



static void
destroy_photo (Photo *photo)
{
  if (photo->body != NULL)
  {
    dBodyDestroy (photo->body);
    photo->body = NULL;

    clutter_actor_destroy (photo->actor);
    photo->actor = NULL;

    destroy_rope (photo->rope + 0);
    destroy_rope (photo->rope + 1);
  }
}



static void
destroy_rope (Rope *rope)
{
  if (rope->nail != NULL)
  {
    gint i;

    dJointDestroy (rope->nail);
    rope->nail = NULL;

    for (i = 0; i < ROPE; i++)
    {
      dBodyDestroy (rope->body[i]);
      rope->body[i] = NULL;

      dJointDestroy (rope->glue[i]);
      rope->glue[i] = NULL;
    }
  }
}



static gboolean
update_world (gpointer data)
{
  clutter_actor_queue_redraw (stage); /* XXX */

  dWorldQuickStep (world, 1);

  return TRUE;
}



/* XXX */
static void
paint_rect (gfloat x,
            gfloat y,
            gfloat s,
            gfloat r,
            gfloat g,
            gfloat b)
{
  gfloat c = 0.2; 
  x = (1 - c) * W / 2 + c * x;
  y = (1 - c) * H / 2 + c * y;

  cogl_set_source_color4f (r, g, b, 1);

  cogl_rectangle (x - s, y - s, x + s, y + s);
}



static void
paint_body (dBodyID body)
{
  const dReal *p = dBodyGetPosition (body);

  gfloat x = p[0] * PPM;
  gfloat y = p[1] * PPM;

  paint_rect (x, y, 3, 0, 0, 1);
}



static void
paint_joint (dJointID joint)
{
  dVector3 p;

  gfloat x;
  gfloat y;

  dJointGetBallAnchor (joint, p);

  x = p[0] * PPM;
  y = p[1] * PPM;

  paint_rect (x, y, 2, 1, 0, 0);
}



static void
paint_world (void)
{
  gint i;

  paint_body (photo.body);
  paint_joint (photo.rope[0].nail);
  paint_joint (photo.rope[1].nail);

  for (i = 0; i < ROPE; i++)
  {
    paint_body (photo.rope[0].body[i]);
    paint_body (photo.rope[1].body[i]);
    paint_joint (photo.rope[0].glue[i]);
    paint_joint (photo.rope[1].glue[i]);
  }
}
/* XXX */



int
main (int   argc,
      char *argv[])
{
  shift = 0;

  dInitODE ();
  world = dWorldCreate ();
  dWorldSetGravity (world, 0, G, 0);

  clutter_init (&argc, &argv);
  clutterrific_init (&argc, &argv);
  cogl_set_depth_test_enabled (TRUE);

  {
    const gchar *dir = g_get_user_special_dir (G_USER_DIRECTORY_PICTURES);

    file = clutterrific_list (dir, "(?i)\\.(" EXTS ")$");
    clutterrific_shuffle (file);
    next = 0;
  }

  {
    ClutterColor bg = { 255, 255, 255, 255 };

    stage = clutter_stage_get_default ();
    clutter_stage_set_color (CLUTTER_STAGE (stage), &bg);
    clutter_actor_show_all (stage);
  }

  g_timeout_add ((guint) (1000 * STEP), update_world, NULL);
  g_signal_connect_after (stage, "paint", paint_world, NULL);

  create_photo (&photo);

  clutter_main ();

  dWorldDestroy (world);
  dCloseODE ();

  return 0;
}
