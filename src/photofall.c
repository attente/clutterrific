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



#define PHOTOS    10

#define ROPE      10

#define CURVE      4

#define THICK      1.5

#define FADE       5

#define POLL       2

#define ERP        6E-1

#define CFM        1E-9

#define FRICTION   0

#define BOUNCE     0

#define STICK      9E+9

#define MASS       1E+1

#define DAMP       4E-2

#define STEP       1E-2

#define G          4E-4

#define PPM        5E+3

#define PAN        2E-2

#define SPACE      6E-1

#define EDGE       5

#define EXTS      "jpg|png"



#include <math.h>

#include <ode/ode.h>

#include <clutter/clutter.h>

#include "clutterrific.h"



#define DEG(x) (180 / M_PI * (x))

#define RAD(x) (M_PI / 180 * (x))

#define W clutterrific_width  ()

#define H clutterrific_height ()

#define U MIN (W, H)

#define V MAX (W, H)



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

  dGeomID       geom;

  ClutterActor *actor;

  Rope          rope[2];

  gfloat        start;

  gfloat        slack;
}
Photo;



static GPtrArray     *file;

static gint           next;

static gfloat         shift;

static dWorldID       world;

static dSpaceID       space;

static dJointGroupID  joints;

static ClutterActor  *stage;

static ClutterActor  *glass;

static Photo          photo[PHOTOS];

static gfloat         average;

static gint           samples;



static void     create_photo  (Photo           *photo);

static void     destroy_photo (Photo           *photo);

static void     destroy_rope  (Rope            *rope);

static void     update_photo  (Photo           *photo);

static void     paint_rope    (Rope            *rope);

static void     paint_ropes   (void);

static void     check_geoms   (void            *data,
                               dGeomID          geom0,
                               dGeomID          geom1);

static gboolean update_world  (gpointer         data);

static gboolean poll_photo    (gpointer         data);

static void     update_fade   (ClutterTimeline *timeline,
                               gint             time,
                               gpointer         data);

static void     finish_fade   (ClutterTimeline *timeline,
                               gpointer         data);



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
      ClutterColor  c = { 240, 224, 192, 255 };
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
      clutter_actor_set_depth     (image, 1);
      clutter_container_add_actor (CLUTTER_CONTAINER (group), paper);
      clutter_container_add_actor (CLUTTER_CONTAINER (group), image);
      clutter_container_add_actor (CLUTTER_CONTAINER (photo->actor), group);
      clutter_container_add_actor (CLUTTER_CONTAINER (stage), photo->actor);
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

    gfloat l = average * g_random_double_range (-y0, H - h - 4 * EDGE - y0);
    gfloat l0 = l + 0.1 * U * g_random_double_range (-1, 1);
    gfloat l1 = 2 * l - l0;

    gfloat dx0 = 0.1 * U * g_random_double_range (-1, 1);
    gfloat dy0 = 0.1 * U * g_random_double_range (-1, 0);
    gfloat dz0 = 0.1 * U * g_random_double_range (-1, 1);

    gfloat dx1 = 0.1 * U * g_random_double_range (-1, 1);
    gfloat dy1 = 0.1 * U * g_random_double_range (-1, 0);
    gfloat dz1 = 0.1 * U * g_random_double_range (-1, 1);

    dMass mass;

    photo->body = dBodyCreate (world);
    dBodySetPosition (photo->body, x / PPM, y / PPM, z / PPM);
    dMassSetBoxTotal (&mass, MASS, w / PPM, h / PPM, 1E-1);
    dBodySetMass (photo->body, &mass);
    dBodySetLinearDamping (photo->body, DAMP);
    dBodySetLinearDampingThreshold (photo->body, 0);
    dBodySetAngularDamping (photo->body, DAMP);
    dBodySetAngularDampingThreshold (photo->body, 0);

    photo->geom = dCreateBox (space, (w + 4 * EDGE) / PPM, (h + 4 * EDGE) / PPM, 1E-3);
    dGeomSetPosition (photo->geom, x / PPM, y / PPM, z / PPM);
    dGeomSetBody (photo->geom, photo->body);

    photo->start = y;
    photo->slack = l;

    {
      Rope *rope = photo->rope;
      gint  i;

      dMassSetSphereTotal (&mass, 1, 1E-1);

      for (i = 0; i < ROPE / 2; i++)
      {
        gfloat s = 1.0 * i / (ROPE - 1);
        gfloat t = 2.0 * i / (ROPE - 1);

        gfloat u0 = (x0 + (1 - s) * dx0) / PPM;
        gfloat v0 = (y0 + (1 - s) * dy0) / PPM;
        gfloat w0 = (z0 + dz0 + t * (l0 / 2 - dz0)) / PPM;
        gfloat u1 = (x1 + (1 - s) * dx1) / PPM;
        gfloat v1 = (y1 + (1 - s) * dy1) / PPM;
        gfloat w1 = (z1 + dz1 + t * (l1 / 2 - dz1)) / PPM;

        rope[0].body[i] = dBodyCreate (world);
        dBodySetPosition (rope[0].body[i], u0, v0, w0);
        dBodySetMass (rope[0].body[i], &mass);
        dBodySetLinearDamping (rope[0].body[i], DAMP);
        dBodySetLinearDampingThreshold (rope[0].body[i], 0);
        dBodySetAngularDamping (rope[0].body[i], DAMP);
        dBodySetAngularDampingThreshold (rope[0].body[i], 0);

        rope[1].body[i] = dBodyCreate (world);
        dBodySetPosition (rope[1].body[i], u1, v1, w1);
        dBodySetMass (rope[1].body[i], &mass);
        dBodySetLinearDamping (rope[1].body[i], DAMP);
        dBodySetLinearDampingThreshold (rope[1].body[i], 0);
        dBodySetAngularDamping (rope[1].body[i], DAMP);
        dBodySetAngularDampingThreshold (rope[1].body[i], 0);
      }

      for (i = ROPE / 2; i < ROPE; i++)
      {
        gfloat s = 1.0 * i / (ROPE - 1);
        gfloat t = (i - ROPE / 2.0 + 0.5) / (ROPE / 2.0 - 0.5);

        gfloat u0 = (x0 + (1 - s) * dx0) / PPM;
        gfloat v0 = (y0 + (1 - s) * dy0) / PPM;
        gfloat w0 = (z0 + (1 - t) * l0 / 2) / PPM;
        gfloat u1 = (x1 + (1 - s) * dx1) / PPM;
        gfloat v1 = (y1 + (1 - s) * dy1) / PPM;
        gfloat w1 = (z1 + (1 - t) * l1 / 2) / PPM;

        rope[0].body[i] = dBodyCreate (world);
        dBodySetPosition (rope[0].body[i], u0, v0, w0);
        dBodySetMass (rope[0].body[i], &mass);
        dBodySetLinearDamping (rope[0].body[i], DAMP);
        dBodySetLinearDampingThreshold (rope[0].body[i], 0);
        dBodySetAngularDamping (rope[0].body[i], DAMP);
        dBodySetAngularDampingThreshold (rope[0].body[i], 0);

        rope[1].body[i] = dBodyCreate (world);
        dBodySetPosition (rope[1].body[i], u1, v1, w1);
        dBodySetMass (rope[1].body[i], &mass);
        dBodySetLinearDamping (rope[1].body[i], DAMP);
        dBodySetLinearDampingThreshold (rope[1].body[i], 0);
        dBodySetAngularDamping (rope[1].body[i], DAMP);
        dBodySetAngularDampingThreshold (rope[1].body[i], 0);
      }

      rope[0].nail = dJointCreateBall (world, NULL);
      rope[1].nail = dJointCreateBall (world, NULL);
      rope[0].glue[ROPE - 1] = dJointCreateBall (world, NULL);
      rope[1].glue[ROPE - 1] = dJointCreateBall (world, NULL);
      dJointAttach (rope[0].nail, 0, rope[0].body[0]);
      dJointAttach (rope[1].nail, 0, rope[1].body[0]);
      dJointAttach (rope[0].glue[ROPE - 1], rope[0].body[ROPE - 1], photo->body);
      dJointAttach (rope[1].glue[ROPE - 1], rope[1].body[ROPE - 1], photo->body);
      dJointSetBallAnchor (rope[0].nail, (x0 + dx0) / PPM, (y0 + dy0) / PPM, (z0 + dz0) / PPM);
      dJointSetBallAnchor (rope[1].nail, (x1 + dx1) / PPM, (y1 + dy1) / PPM, (z1 + dz1) / PPM);
      dJointSetBallAnchor (rope[0].glue[ROPE - 1], x0 / PPM, y0 / PPM, z0 / PPM);
      dJointSetBallAnchor (rope[1].glue[ROPE - 1], x1 / PPM, y1 / PPM, z1 / PPM);

      for (i = 0; i < ROPE / 2; i++)
      {
        gfloat s = (i + 0.5) / (ROPE - 1);
        gfloat t = 2.0 * (i + 0.5) / (ROPE - 1);

        gfloat u0 = (x0 + (1 - s) * dx0) / PPM;
        gfloat v0 = (y0 + (1 - s) * dy0) / PPM;
        gfloat w0 = (z0 + dz0 + t * (l0 / 2 - dz0)) / PPM;
        gfloat u1 = (x1 + (1 - s) * dx1) / PPM;
        gfloat v1 = (y1 + (1 - s) * dy1) / PPM;
        gfloat w1 = (z1 + dz1 + t * (l1 / 2 - dz1)) / PPM;

        rope[0].glue[i] = dJointCreateBall (world, NULL);
        dJointAttach (rope[0].glue[i], rope[0].body[i], rope[0].body[i + 1]);
        dJointSetBallAnchor (rope[0].glue[i], u0, v0, w0);

        rope[1].glue[i] = dJointCreateBall (world, NULL);
        dJointAttach (rope[1].glue[i], rope[1].body[i], rope[1].body[i + 1]);
        dJointSetBallAnchor (rope[1].glue[i], u1, v1, w1);
      }

      for (i = ROPE / 2; i + 1 < ROPE; i++)
      {
        gfloat s = (i + 0.5) / (ROPE - 1);
        gfloat t = (i + 1.0 - ROPE / 2.0) / (ROPE / 2.0 - 0.5);

        gfloat u0 = (x0 + (1 - s) * dx0) / PPM;
        gfloat v0 = (y0 + (1 - s) * dy0) / PPM;
        gfloat w0 = (z0 + (1 - t) * l0 / 2) / PPM;
        gfloat u1 = (x1 + (1 - s) * dx1) / PPM;
        gfloat v1 = (y1 + (1 - s) * dy1) / PPM;
        gfloat w1 = (z1 + (1 - t) * l1 / 2) / PPM;

        rope[0].glue[i] = dJointCreateBall (world, NULL);
        dJointAttach (rope[0].glue[i], rope[0].body[i], rope[0].body[i + 1]);
        dJointSetBallAnchor (rope[0].glue[i], u0, v0, w0);

        rope[1].glue[i] = dJointCreateBall (world, NULL);
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

    dGeomDestroy (photo->geom);
    photo->geom = NULL;

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



static void
update_photo (Photo *photo)
{
  if (photo->body != NULL)
  {
    ClutterActor *group = clutter_group_get_nth_child (CLUTTER_GROUP (photo->actor), 0);

    const dReal *a = dBodyGetPosition (photo->body);
    const dReal *b = dBodyGetPosition (photo->rope[0].body[ROPE - 1]);
    const dReal *c = dBodyGetPosition (photo->rope[1].body[ROPE - 1]);

    gfloat w = clutter_actor_get_width  (photo->actor);
    gfloat h = clutter_actor_get_height (photo->actor);

    gfloat ax = a[0] * PPM;
    gfloat ay = a[1] * PPM;
    gfloat az = a[2] * PPM;

    gfloat bx = b[0] * PPM;
    gfloat by = b[1] * PPM;
    gfloat bz = b[2] * PPM;

    gfloat cx = c[0] * PPM;
    gfloat cy = c[1] * PPM;
    gfloat cz = c[2] * PPM;

    gfloat dx = ax - bx;
    gfloat dy = ay - by;
    gfloat dz = az - bz;

    gfloat ex;
    gfloat ey;
    gfloat ez;

    gfloat rx;
    gfloat ry = asin ((bz - cz) / MAX (bz - cz, w - 2 * EDGE));
    gfloat rz = atan2 (cy - by, cx - bx);

    if (ax + w + h < shift)
    {
      average *= samples;
      average += photo->slack / (ay - photo->start);
      average /= ++samples;

      destroy_photo (photo);

      return;
    }

    if (bx > cx)
      ry = M_PI - ry;

    ex = dx *  cos (rz) + dy * sin (rz);
    ey = dx * -sin (rz) + dy * cos (rz);
    ez = dz;

    dx = ex * cos (ry) + ez * -sin (ry);
    dy = ey;
    dz = ex * sin (ry) + ez *  cos (ry);

    rx = atan2 (dz, dy);

    clutter_actor_set_position (photo->actor, bx - EDGE - shift, by - EDGE);
    clutter_actor_set_depth    (photo->actor, bz);
    clutter_actor_set_rotation (photo->actor, CLUTTER_Y_AXIS, DEG (ry), EDGE, EDGE, 0);
    clutter_actor_set_rotation (photo->actor, CLUTTER_Z_AXIS, DEG (rz), EDGE, EDGE, 0);
    clutter_actor_set_rotation (group, CLUTTER_X_AXIS, DEG (rx), EDGE, EDGE, 0);
  }
}



static void
paint_rope (Rope *rope)
{
  gint   points = 1 + (ROPE - 1) * CURVE;
  gfloat data[3 * 2 * (1 + (ROPE - 1) * CURVE)];
  gint   i, j;

  for (i = 0; i + 1 < ROPE; i++)
  {
    dVector3     a, b, c;
    const dReal *d = dBodyGetPosition (rope->body[i]);
    const dReal *e = dBodyGetPosition (rope->body[i + 1]);

    dJointGetBallAnchor (rope->glue[i], b);

    a[0] = d[0] * PPM - shift;
    a[1] = d[1] * PPM;
    a[2] = d[2] * PPM;
    b[0] = b[0] * PPM - shift;
    b[1] = b[1] * PPM;
    b[2] = b[2] * PPM;
    c[0] = e[0] * PPM - shift;
    c[1] = e[1] * PPM;
    c[2] = e[2] * PPM;

    for (j = 0; j < CURVE + (i + 1 == ROPE - 1); j++)
    {
      gfloat  t = (gfloat) j / CURVE;
      gfloat  s = 1 - t;

      gfloat *u = data + 3 * 2 * (CURVE * i + j);
      gfloat *v = u + 3;
      gfloat  w[3];

      w[0] = s * s * a[0] + 2 * s * t * b[0] + t * t * c[0];
      w[1] = s * s * a[1] + 2 * s * t * b[1] + t * t * c[1];
      w[2] = s * s * a[2] + 2 * s * t * b[2] + t * t * c[2];

      u[0] = w[0] - THICK;
      u[1] = w[1];
      u[2] = w[2];

      v[0] = w[0] + THICK;
      v[1] = w[1];
      v[2] = w[2];
    }
  }

  {
    CoglHandle        buffer = cogl_vertex_buffer_new (2 * points);
    CoglVerticesMode  mode   = COGL_VERTICES_MODE_TRIANGLE_STRIP;
    CoglAttributeType type   = COGL_ATTRIBUTE_TYPE_FLOAT;

    cogl_vertex_buffer_add  (buffer, "gl_Vertex", 3, type, TRUE, 12, data);
    cogl_vertex_buffer_draw (buffer, mode, 0, 2 * points);

    cogl_handle_unref (buffer);
  }
}



static void
paint_ropes (void)
{
  gint i;

  cogl_set_source_color4f (0.2, 0.2, 0.4, 1.0);

  for (i = 0; i < PHOTOS; i++)
  {
    if (photo[i].body != NULL)
    {
      paint_rope (photo[i].rope + 0);
      paint_rope (photo[i].rope + 1);
    }
  }
}



static void
check_geoms (void    *data,
             dGeomID  geom0,
             dGeomID  geom1)
{
  dBodyID  body0 = dGeomGetBody (geom0);
  dBodyID  body1 = dGeomGetBody (geom1);
  dContact contact;
  dJointID correct;

  dCollide (geom0, geom1, 1, &contact.geom, sizeof (dContact));

  contact.surface.mode = dContactBounce;
  contact.surface.mu = FRICTION;
  contact.surface.bounce = BOUNCE;
  contact.surface.bounce_vel = STICK;

  correct = dJointCreateContact (world, joints, &contact);
  dJointAttach (correct, body0, body1);
}



static gboolean
update_world (gpointer data)
{
  gdouble dt = clutterrific_delta ();
  gint    i;

  shift += dt * PAN * PPM;

  dSpaceCollide (space, NULL, check_geoms);
  dWorldQuickStep (world, 1);
  dJointGroupEmpty (joints);

  for (i = 0; i < PHOTOS; i++)
    if (photo[i].body != NULL)
      update_photo (photo + i);

  clutter_actor_queue_redraw (stage);

  return TRUE;
}



static gboolean
poll_photo (gpointer data)
{
  gint i;

  for (i = 0; i < PHOTOS; i++)
  {
    if (photo[i].body == NULL)
    {
      create_photo (photo + i);

      break;
    }
  }

  return TRUE;
}



static void
update_fade (ClutterTimeline *timeline,
             gint             time,
             gpointer         data)
{
  gfloat       alpha = clutter_timeline_get_progress (timeline);
  ClutterColor black = { 0, 0, 0 };

  black.alpha = 255 * (1 - alpha);

  clutter_rectangle_set_color (CLUTTER_RECTANGLE (glass), &black);
}



static void
finish_fade (ClutterTimeline *timeline,
             gpointer         data)
{
  clutter_actor_destroy (glass);

  g_object_unref (timeline);

  g_timeout_add ((gint) 1000 * POLL, poll_photo, NULL);
}



int
main (int   argc,
      char *argv[])
{
  shift   = 0;
  average = 1;
  samples = 0;

  dInitODE ();
  world = dWorldCreate ();
  dWorldSetCFM (world, CFM);
  dWorldSetERP (world, ERP);
  dWorldSetGravity (world, 0, G, 0);
  space = dSimpleSpaceCreate (NULL);
  joints = dJointGroupCreate (0);

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
  g_signal_connect_after (stage, "paint", paint_ropes, NULL);

  {
    ClutterTimeline *fade  = clutter_timeline_new ((gint) (1000 * FADE));
    ClutterColor     black = { 0, 0, 0, 255 };

    glass = clutter_rectangle_new_with_color (&black);
    clutter_actor_set_position  (glass, -W, -H);
    clutter_actor_set_depth     (glass, 400);
    clutter_actor_set_size      (glass, 3 * W, 3 * H);
    clutter_container_add_actor (CLUTTER_CONTAINER (stage), glass);

    g_signal_connect (fade, "new-frame", G_CALLBACK (update_fade), NULL);
    g_signal_connect (fade, "completed", G_CALLBACK (finish_fade), NULL);

    clutter_timeline_start (fade);
  }

  clutter_main ();

  dJointGroupDestroy (joints);
  dSpaceDestroy (space);
  dWorldDestroy (world);
  dCloseODE ();

  return 0;
}
