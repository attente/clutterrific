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

#define THICK      1

#define FADE       5

#define POLL       2

#define ERP        5E-1

#define CFM        1E-9

#define FRICTION   0

#define BOUNCE     0

#define STICK      9E+9

#define SNAP       1E-1

#define MASS       1E+1

#define DAMP       6E-2

#define STEP       1E-2

#define G          4E-4

#define PPM        5E+3

#define PAN        2E-2

#define SPACE      6E-1

#define LIGHT      1E+0

#define WALL       2E-1

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
  dBodyID         body[3];

  dJointID        joint[2];

  dGeomID         geom;

  ClutterActor   *actor;

  Rope            rope[2];

  gfloat          start;

  gfloat          slack;

  gfloat          stress;

  dJointID        snap;

  dJointFeedback  info;
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

static void     check_snap    (Photo           *photo);

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

  if (photo->body[0] != NULL)
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
    gfloat l0 = l + 0.1 * U * g_random_double_range (-0.5, 0.5);
    gfloat l1 = 2 * l - l0;

    gfloat dx0 = 0.1 * U * g_random_double_range (-1, 1);
    gfloat dy0 = 0.1 * U * g_random_double_range (-1, 0);
    gfloat dz0 = 0.1 * U * g_random_double_range (-1, 1);

    gfloat dx1 = 0.1 * U * g_random_double_range (-1, 1);
    gfloat dy1 = 0.1 * U * g_random_double_range (-1, 0);
    gfloat dz1 = 0.1 * U * g_random_double_range (-1, 1);

    dQuaternion quaternion = { 0, 0, 0, 1 };
    dMass       mass;

    photo->body[0] = dBodyCreate (world);
    photo->body[1] = dBodyCreate (world);
    photo->body[2] = dBodyCreate (world);
    dBodySetPosition (photo->body[0], x / PPM, y / PPM, z / PPM);
    dBodySetPosition (photo->body[1], x0 / PPM, y0 / PPM, z0 / PPM);
    dBodySetPosition (photo->body[2], x1 / PPM, y1 / PPM, z1 / PPM);
    dBodySetQuaternion (photo->body[0], quaternion);
    dBodySetQuaternion (photo->body[1], quaternion);
    dBodySetQuaternion (photo->body[2], quaternion);
    dMassSetBoxTotal (&mass, MASS, w / PPM, h / PPM, 1E-1);
    dBodySetMass (photo->body[0], &mass);
    dMassSetSphereTotal (&mass, 1E-1, 1E-1);
    dBodySetMass (photo->body[1], &mass);
    dBodySetMass (photo->body[2], &mass);
    dBodySetLinearDamping (photo->body[0], DAMP);
    dBodySetLinearDamping (photo->body[1], DAMP);
    dBodySetLinearDamping (photo->body[2], DAMP);
    dBodySetLinearDampingThreshold (photo->body[0], 0);
    dBodySetLinearDampingThreshold (photo->body[1], 0);
    dBodySetLinearDampingThreshold (photo->body[2], 0);
    dBodySetAngularDamping (photo->body[0], DAMP);
    dBodySetAngularDamping (photo->body[1], DAMP);
    dBodySetAngularDamping (photo->body[2], DAMP);
    dBodySetAngularDampingThreshold (photo->body[0], 0);
    dBodySetAngularDampingThreshold (photo->body[1], 0);
    dBodySetAngularDampingThreshold (photo->body[2], 0);

    photo->geom = dCreateBox (space, (w + 4 * EDGE) / PPM, (h + 4 * EDGE) / PPM, 1E-3);
    dGeomSetPosition (photo->geom, x / PPM, y / PPM, z / PPM);
    dGeomSetBody (photo->geom, photo->body[0]);

    photo->joint[0] = dJointCreateBall (world, NULL);
    photo->joint[1] = dJointCreateBall (world, NULL);
    dJointAttach (photo->joint[0], photo->body[0], photo->body[1]);
    dJointAttach (photo->joint[1], photo->body[0], photo->body[2]);
    dJointSetBallAnchor (photo->joint[0], x0 / PPM, y0 / PPM, z0 / PPM);
    dJointSetBallAnchor (photo->joint[1], x1 / PPM, y1 / PPM, z1 / PPM);

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
        dBodySetQuaternion (rope[0].body[i], quaternion);
        dBodySetMass (rope[0].body[i], &mass);
        dBodySetLinearDamping (rope[0].body[i], DAMP);
        dBodySetLinearDampingThreshold (rope[0].body[i], 0);
        dBodySetAngularDamping (rope[0].body[i], DAMP);
        dBodySetAngularDampingThreshold (rope[0].body[i], 0);

        rope[1].body[i] = dBodyCreate (world);
        dBodySetPosition (rope[1].body[i], u1, v1, w1);
        dBodySetQuaternion (rope[1].body[i], quaternion);
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
        dBodySetQuaternion (rope[0].body[i], quaternion);
        dBodySetMass (rope[0].body[i], &mass);
        dBodySetLinearDamping (rope[0].body[i], DAMP);
        dBodySetLinearDampingThreshold (rope[0].body[i], 0);
        dBodySetAngularDamping (rope[0].body[i], DAMP);
        dBodySetAngularDampingThreshold (rope[0].body[i], 0);

        rope[1].body[i] = dBodyCreate (world);
        dBodySetPosition (rope[1].body[i], u1, v1, w1);
        dBodySetQuaternion (rope[1].body[i], quaternion);
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
      dJointAttach (rope[0].glue[ROPE - 1], rope[0].body[ROPE - 1], photo->body[1]);
      dJointAttach (rope[1].glue[ROPE - 1], rope[1].body[ROPE - 1], photo->body[2]);
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

    if (g_random_double () < SNAP)
    {
      if (g_random_boolean ())
        photo->snap = photo->rope[l0 > l1].nail;
      else
        photo->snap = photo->rope[l0 > l1].glue[ROPE - 1];

      dJointSetFeedback (photo->snap, &photo->info);
    }

    photo->start  = y;
    photo->slack  = l;
    photo->stress = -1;
  }
}



static void
destroy_photo (Photo *photo)
{
  if (photo->body[0] != NULL)
  {
    dBodyDestroy (photo->body[0]);
    dBodyDestroy (photo->body[1]);
    dBodyDestroy (photo->body[2]);

    dJointDestroy (photo->joint[0]);
    dJointDestroy (photo->joint[1]);

    dGeomDestroy (photo->geom);

    clutter_actor_destroy (photo->actor);

    destroy_rope (photo->rope + 0);
    destroy_rope (photo->rope + 1);

    photo->body[0]  = NULL;
    photo->body[1]  = NULL;
    photo->body[2]  = NULL;
    photo->joint[0] = NULL;
    photo->joint[1] = NULL;
    photo->geom     = NULL;
    photo->actor    = NULL;
    photo->snap     = NULL;
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
  if (photo->body[0] != NULL)
  {
    ClutterActor *group = clutter_group_get_nth_child (CLUTTER_GROUP (photo->actor), 0);

    const dReal *a = dBodyGetPosition (photo->body[0]);
    const dReal *b = dBodyGetPosition (photo->body[1]);
    const dReal *c = dBodyGetPosition (photo->body[2]);

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
    gint              stride = 3 * sizeof (gfloat);

    cogl_vertex_buffer_add  (buffer, "gl_Vertex", 3, type, TRUE, stride, data);
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
    if (photo[i].body[0] != NULL)
    {
      paint_rope (photo[i].rope + 0);
      paint_rope (photo[i].rope + 1);
    }
  }
}



static void
paint_shadow (Photo *photo)
{
  gfloat data[4][3];
  gfloat light[3];
  gfloat abc[3];
  gfloat bc[3];
  gfloat a[3];
  gfloat b[3];
  gfloat c[3];
  gfloat s, t;
  gint   i, j;

  light[0] = shift;
  light[1] = H / 4;
  light[2] = LIGHT * U;

  {
    const dReal *d = dBodyGetPosition (photo->body[0]);
    const dReal *e = dBodyGetPosition (photo->body[1]);
    const dReal *f = dBodyGetPosition (photo->body[2]);

    for (i = 0; i < 3; i++)
    {
      a[i] = d[i] * PPM;
      b[i] = e[i] * PPM;
      c[i] = f[i] * PPM;

      abc[i] = (b[i] + c[i]) / 2 - a[i];
      bc[i] = c[i] - b[i];
    }
  }

  s = EDGE / sqrt (dCalcVectorLengthSquare3 (abc));
  t = EDGE / sqrt (dCalcVectorLengthSquare3 (bc));

  dAddScaledVectors3 (data[0], abc, bc, -s, -t);
  dAddScaledVectors3 (data[1], abc, bc, -s,  t);

  for (i = 0; i < 3; i++)
  {
    data[0][i] += b[i] - a[i];
    data[1][i] += c[i] - a[i];
  }

  dCopyNegatedVector3 (data[2], data[1]);
  dCopyNegatedVector3 (data[3], data[0]);

  for (i = 0; i < 4; i++)
  {
    for (j = 0; j < 3; j++)
      data[i][j] += a[j];

    s = (-WALL * U - light[2]) / (data[i][2] - light[2]);

    for (j = 0; j < 3; j++)
      data[i][j] = light[j] + s * (data[i][j] - light[j]);

    data[i][0] -= shift;
  }

  {
    CoglHandle        buffer = cogl_vertex_buffer_new (4);
    CoglAttributeType type   = COGL_ATTRIBUTE_TYPE_FLOAT;
    gint              stride = 3 * sizeof (gfloat);

    cogl_vertex_buffer_add  (buffer, "gl_Vertex", 3, type, TRUE, stride, data);
    cogl_vertex_buffer_draw (buffer, COGL_VERTICES_MODE_TRIANGLE_STRIP, 0, 4);

    cogl_handle_unref (buffer);
  }
}



static void
paint_shadows (void)
{
  gint i;

  cogl_set_source_color4ub (240, 240, 248, 255);

  for (i = 0; i < PHOTOS; i++)
    if (photo[i].body[0] != NULL)
      paint_shadow (photo + i);
}



static void
check_snap (Photo *photo)
{
  if (photo->snap != NULL)
  {
    gfloat f[2];

    f[0] = dCalcVectorLengthSquare3 (photo->info.f1);
    f[1] = dCalcVectorLengthSquare3 (photo->info.f2);

    if (f[0] + f[1] < 0.99 * photo->stress)
    {
      dJointAttach (photo->snap, 0, 0);

      photo->snap = 0;
    }

    photo->stress = f[0] + f[1];
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
  {
    if (photo[i].body[0] != NULL)
    {
      update_photo (photo + i);

      if (photo[i].snap != NULL)
        check_snap (photo + i);
    }
  }

  clutter_actor_queue_redraw (stage);

  return TRUE;
}



static gboolean
poll_photo (gpointer data)
{
  gint i;

  for (i = 0; i < PHOTOS; i++)
  {
    if (photo[i].body[0] == NULL)
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
  dGeomID wall;

  shift   = 0;
  average = 1;
  samples = 0;

  dInitODE ();
  world = dWorldCreate ();
  dWorldSetCFM (world, CFM);
  dWorldSetERP (world, ERP);
  dWorldSetGravity (world, 0, G, 0);
  dWorldSetQuickStepNumIterations (world, 100);

  space = dSimpleSpaceCreate (NULL);
  joints = dJointGroupCreate (0);

  clutter_init (&argc, &argv);
  clutterrific_init (&argc, &argv);
  cogl_set_depth_test_enabled (TRUE);

  wall = dCreatePlane (space, 0, 0, 1, -WALL * U / PPM);

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
  g_signal_connect_after (stage, "paint", paint_shadows, NULL);

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

  {
    gint i;

    for (i = 0; i < PHOTOS; i++)
      if (photo[i].body[0] != NULL)
        destroy_photo (photo + i);
  }

  dGeomDestroy (wall);
  dJointGroupDestroy (joints);
  dSpaceDestroy (space);
  dWorldDestroy (world);
  dCloseODE ();

  return 0;
}
