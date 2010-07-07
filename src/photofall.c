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



#define STEP 1E-2

#define G    1E-3

#define W    clutterrific_width  ()

#define H    clutterrific_height ()



#include <ode/ode.h>

#include <clutter/clutter.h>

#include "clutterrific.h"



typedef struct
{
}
Rope;



typedef struct
{
  dBodyID       body;

  ClutterActor *actor;

  Rope          rope[2];
}
Photo;



static dWorldID      world;

static ClutterActor *stage;

/* XXX */
static dBodyID  body [5];
static dJointGroupID group;
static dJointID joint[5];
/* XXX */



static gboolean
update_world (gpointer data)
{
  clutter_actor_queue_redraw (stage);

  dWorldQuickStep (world, 1);

  return TRUE;
}

/* XXX */
static void draw_body (dBodyID body) {
  const dReal *p = dBodyGetPosition (body);
  gfloat x = p[0] * 200 + 200;
  gfloat y = p[1] * 200;

  cogl_rectangle (x - 2, y - 2, x + 2, y + 2);
}

static void draw_joint (dJointID joint) {
  dVector3 p;
  gfloat x;
  gfloat y;
  dJointGetBallAnchor (joint, p);
  x = p[0] * 200 + 200;
  y = p[1] * 200;

  cogl_rectangle (x - 2, y - 2, x + 2, y + 2);
}

static void
paint_world (void)
{
  cogl_set_source_color4f (0, 0, 1, 1);
  draw_body (body[0]);
  draw_body (body[1]);
  draw_body (body[2]);
  draw_body (body[3]);
  draw_body (body[4]);
  cogl_set_source_color4f (1, 0, 0, 1);
  draw_joint (joint[0]);
  draw_joint (joint[1]);
  draw_joint (joint[2]);
  draw_joint (joint[3]);
  draw_joint (joint[4]);
}
/* XXX */



int
main (int   argc,
      char *argv[])
{
  dInitODE ();
  world = dWorldCreate ();
  dWorldSetGravity (world, 0, G, 0);

  clutter_init (&argc, &argv);
  clutterrific_init (&argc, &argv);
  cogl_set_depth_test_enabled (TRUE);
  stage = clutter_stage_get_default ();
  clutter_actor_show_all (stage);

  {
    ClutterColor bg = { 255, 255, 255, 255 };

    clutter_stage_set_color (CLUTTER_STAGE (stage), &bg);
  }

  /* XXX */
  {
    dMass mass;
    body[0] = dBodyCreate (world);
    body[1] = dBodyCreate (world);
    body[2] = dBodyCreate (world);
    body[3] = dBodyCreate (world);
    body[4] = dBodyCreate (world);
    dBodySetPosition (body[0], 0.1, 0.5, 0);
    dBodySetPosition (body[1], 0.2, 0.5, 0);
    dBodySetPosition (body[2], 0.3, 0.5, 0);
    dBodySetPosition (body[3], 0.4, 0.5, 0);
    dBodySetPosition (body[4], 0.8, 0.1, 0);
    dMassSetSphere (&mass, 1, 0.02);
    dBodySetMass (body[0], &mass);
    dBodySetMass (body[1], &mass);
    dBodySetMass (body[2], &mass);
    dBodySetMass (body[3], &mass);
    dMassSetSphere (&mass, 5, 0.02);
    dBodySetMass (body[4], &mass);
    group = dJointGroupCreate (0);
    joint[0] = dJointCreateBall (world, group);
    joint[1] = dJointCreateBall (world, group);
    joint[2] = dJointCreateBall (world, group);
    joint[3] = dJointCreateBall (world, group);
    joint[4] = dJointCreateBall (world, group);
    dJointAttach (joint[0], 0, body[0]);
    dJointAttach (joint[1], body[0], body[1]);
    dJointAttach (joint[2], body[1], body[2]);
    dJointAttach (joint[3], body[2], body[3]);
    dJointAttach (joint[4], body[3], body[4]);
    dJointSetBallAnchor (joint[0], 0.05, 0.5, 0);
    dJointSetBallAnchor (joint[1], 0.15, 0.5, 0);
    dJointSetBallAnchor (joint[2], 0.25, 0.5, 0);
    dJointSetBallAnchor (joint[3], 0.35, 0.5, 0);
    dJointSetBallAnchor (joint[4], 0.45, 0.5, 0);
  }
  /* XXX */

  g_timeout_add ((guint) (1000 * STEP), update_world, NULL);
  g_signal_connect_after (stage, "paint", paint_world, NULL);

  clutter_main ();

  dWorldDestroy (world);
  dCloseODE ();

  return 0;
}
