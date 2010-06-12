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



#define EXTENSIONS "jpg|png"

/* cube upscale */
#define SCALE         3

/* intertile space */
#define SPACE         0

/* number of turns */
#define MIN_TURNS    18
#define MAX_TURNS    22

/* non-turning moves */
#define PRE_WAIT      1
#define POST_WAIT     5

/* milliseconds per turn */
#define PERIOD     1000



#include <clutter/clutter.h>

#include "clutterrific.h"



static gint         *move;
static gint          moves;
static gint          pivot[6];
static gint          index[54];

static ClutterActor *cube;
static ClutterActor *tile[54][3];

static GPtrArray    *file;



static gint          pack      (gint             i,
                                gint             j,
                                gint             k);

static void          twist     (gint             move);

static void          untwist   (gint             move);

static void          shuffle   ();

static gint          get_delta (gint             index,
                                gint             where,
                                gint             pivot);

static void          layout    (gint             move,
                                gfloat           angle);

static void          spin      (ClutterTimeline *timeline,
                                gint             frame,
                                gpointer         data);

static void          turn      (ClutterTimeline *timeline,
                                gint             frame,
                                gpointer         data);



static gint
pack (gint i,
      gint j,
      gint k)
{
  return i * 9 + j * 3 + k;
}



static void
swap (gint turns,
      gint a,
      gint b,
      gint c,
      gint d)
{
  gint w = index[a];
  gint x = index[b];
  gint y = index[c];
  gint z = index[d];

  switch ((turns % 4 + 4) % 4)
  {
    case 1:
      index[a] = z;
      index[b] = w;
      index[c] = x;
      index[d] = y;

      break;

    case 2:
      index[a] = y;
      index[b] = z;
      index[c] = w;
      index[d] = x;

      break;

    case 3:
      index[a] = x;
      index[b] = y;
      index[c] = z;
      index[d] = w;

      break;
  }
}



static void
twist (gint move)
{
  gint face  = move / 3;
  gint turns = move % 3 + 1;

  if (face >= 0)
  {
    pivot[face] += face < 3 ? turns : 4 - turns;
    pivot[face] %= 4;
  }

  switch (face)
  {
    case 0:
      swap (turns,  0,  2,  8,  6);
      swap (turns,  1,  5,  7,  3);
      swap (turns,  9, 45, 36, 18);
      swap (turns, 12, 48, 39, 21);
      swap (turns, 15, 51, 42, 24);

      break;

    case 1:
      swap (turns,  9, 11, 17, 15);
      swap (turns, 10, 14, 16, 12);
      swap (turns,  0, 26, 27, 45);
      swap (turns,  1, 25, 28, 46);
      swap (turns,  2, 24, 29, 47);

      break;

    case 2:
      swap (turns, 18, 20, 26, 24);
      swap (turns, 19, 23, 25, 21);
      swap (turns,  0, 42, 35, 11);
      swap (turns,  3, 43, 32, 10);
      swap (turns,  6, 44, 29,  9);

      break;

    case 3:
      swap (turns, 27, 33, 35, 29);
      swap (turns, 28, 30, 34, 32);
      swap (turns, 11, 47, 38, 20);
      swap (turns, 14, 50, 41, 23);
      swap (turns, 17, 53, 44, 26);

      break;

    case 4:
      swap (turns, 36, 42, 44, 38);
      swap (turns, 37, 39, 43, 41);
      swap (turns,  6, 20, 33, 51);
      swap (turns,  7, 19, 34, 52);
      swap (turns,  8, 18, 35, 53);

      break;

    case 5:
      swap (turns, 45, 51, 53, 47);
      swap (turns, 46, 48, 52, 50);
      swap (turns,  2, 36, 33, 17);
      swap (turns,  5, 37, 30, 16);
      swap (turns,  8, 38, 27, 15);

      break;
  }
}



static void
untwist (gint move)
{
  twist (move < 0 ? -3 : move / 3 * 3 + 2 - move % 3);
}



static void
shuffle (void)
{
  if (moves)
    g_free (move);

  {
    gint i, j, k;

    for (i = 0; i < 6; i++)
    for (j = 0; j < 3; j++)
    for (k = 0; k < 3; k++)
    {
      pivot[i]              = 0;
      index[pack (i, j, k)] = i * 9 + j * 3 + k;
    }
  }

  moves = POST_WAIT
        + g_random_int_range (MIN_TURNS, MAX_TURNS + 1)
        + PRE_WAIT;

  if (moves < POST_WAIT + 1 + PRE_WAIT)
    moves = POST_WAIT + 1 + PRE_WAIT;

  move = g_new (gint, moves);

  {
    gint i;

    for (i = 0; i < POST_WAIT; i++)
      move[i] = -3;

    for (i = moves - PRE_WAIT; i < moves; i++)
      move[i] = -3;

    move[POST_WAIT] = g_random_int_range (0, 18);

    twist (move[POST_WAIT]);

    for (i = POST_WAIT + 1; i < moves - PRE_WAIT; i++)
    {
      if (i == POST_WAIT + 1 || move[i - 2] / 3 % 3 != move[i - 1] / 3 % 3)
      {
        move[i] = g_random_int_range (0, 15);

        if (move[i] / 3 >= move[i - 1] / 3)
          move[i] += 3;
      }
      else
      {
        move[i] = g_random_int_range (0, 12);

        if (move[i] / 3 >= move[i - 1] / 3 % 3)
          move[i] += 3;

        if (move[i] / 3 >= move[i - 1] / 3 % 3 + 3)
          move[i] += 3;
      }

      twist (move[i]);
    }
  }
}



static gint
get_delta (gint index,
           gint where,
           gint pivot)
{
  gint x1 = index % 3 - 1;
  gint y1 = index / 3 % 3 - 1;
  gint x2 = where % 3 - 1;
  gint y2 = where / 3 % 3 - 1;

  if (!x1 && !y1 && !x2 && !y2)
    return 90 * pivot;

  if (x2 == x1 && y2 == y1)
    return 0;
  else if (x2 == -y1 && y2 == x1)
    return 90;
  else if (x2 == -x1 && y2 == -y1)
    return 180;
  else
    return 270;
}



static void
rotate (gint              index,
        gint              level,
        ClutterRotateAxis axis,
        gfloat            angle)
{
  clutter_actor_set_rotation (tile[index][level], axis, angle,
                              45 - SPACE * (index % 3 - 1),
                              45 - SPACE * (index / 3 % 3 - 1),
                              -45 - 2 * SPACE);
}



static void
layout (gint   face,
        gfloat angle)
{
  gint i, j, k;

  for (i = 0; i < 3; i++)
  for (j = 0; j < 3; j++)
  {
    gint l[6];

    for (k = 0; k < 6; k++)
      l[k] = index[pack (k, i, j)];

    rotate (l[0], 0, CLUTTER_X_AXIS,   0);
    rotate (l[0], 0, CLUTTER_Y_AXIS, 270);
    rotate (l[0], 0, CLUTTER_Z_AXIS,   0);
    rotate (l[1], 0, CLUTTER_X_AXIS,  90);
    rotate (l[1], 0, CLUTTER_Y_AXIS,   0);
    rotate (l[1], 0, CLUTTER_Z_AXIS,   0);
    rotate (l[2], 0, CLUTTER_X_AXIS, 180);
    rotate (l[2], 0, CLUTTER_Y_AXIS,   0);
    rotate (l[2], 0, CLUTTER_Z_AXIS,   0);
    rotate (l[3], 0, CLUTTER_X_AXIS,   0);
    rotate (l[3], 0, CLUTTER_Y_AXIS,  90);
    rotate (l[3], 0, CLUTTER_Z_AXIS,   0);
    rotate (l[4], 0, CLUTTER_X_AXIS, 270);
    rotate (l[4], 0, CLUTTER_Y_AXIS,   0);
    rotate (l[4], 0, CLUTTER_Z_AXIS,   0);
    rotate (l[5], 0, CLUTTER_X_AXIS,   0);
    rotate (l[5], 0, CLUTTER_Y_AXIS,   0);
    rotate (l[5], 0, CLUTTER_Z_AXIS,   0);

    for (k = 0; k < 6; k++)
      rotate (l[k], 2,
              CLUTTER_Z_AXIS,
              get_delta (l[k], 3 * i + j, pivot[k]));
  }

  for (i = 0; i < 54; i++)
  {
    rotate (i, 1, CLUTTER_X_AXIS, 0);
    rotate (i, 1, CLUTTER_Y_AXIS, 0);
    rotate (i, 1, CLUTTER_Z_AXIS, 0);
  }

  if (face >= 0)
  {
    for (i = 0; i < 9; i++)
      rotate (index[9 * face + i], 1,
              CLUTTER_Z_AXIS,
              face < 3 ? -angle : angle);
  }

  switch (face)
  {
    case 0:
      rotate (index[ 9], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[12], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[15], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[18], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[21], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[24], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[36], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[39], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[42], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[45], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[48], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[51], 1, CLUTTER_X_AXIS,  angle);

      break;

    case 1:
      rotate (index[ 0], 1, CLUTTER_Y_AXIS,  angle);
      rotate (index[ 1], 1, CLUTTER_Y_AXIS,  angle);
      rotate (index[ 2], 1, CLUTTER_Y_AXIS,  angle);
      rotate (index[24], 1, CLUTTER_Y_AXIS, -angle);
      rotate (index[25], 1, CLUTTER_Y_AXIS, -angle);
      rotate (index[26], 1, CLUTTER_Y_AXIS, -angle);
      rotate (index[27], 1, CLUTTER_Y_AXIS,  angle);
      rotate (index[28], 1, CLUTTER_Y_AXIS,  angle);
      rotate (index[29], 1, CLUTTER_Y_AXIS,  angle);
      rotate (index[45], 1, CLUTTER_Y_AXIS,  angle);
      rotate (index[46], 1, CLUTTER_Y_AXIS,  angle);
      rotate (index[47], 1, CLUTTER_Y_AXIS,  angle);

      break;

    case 2:
      rotate (index[ 0], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[ 3], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[ 6], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[ 9], 1, CLUTTER_Y_AXIS,  angle);
      rotate (index[10], 1, CLUTTER_Y_AXIS,  angle);
      rotate (index[11], 1, CLUTTER_Y_AXIS,  angle);
      rotate (index[29], 1, CLUTTER_X_AXIS, -angle);
      rotate (index[32], 1, CLUTTER_X_AXIS, -angle);
      rotate (index[35], 1, CLUTTER_X_AXIS, -angle);
      rotate (index[42], 1, CLUTTER_Y_AXIS, -angle);
      rotate (index[43], 1, CLUTTER_Y_AXIS, -angle);
      rotate (index[44], 1, CLUTTER_Y_AXIS, -angle);

      break;

    case 3:
      rotate (index[11], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[14], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[17], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[20], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[23], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[26], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[38], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[41], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[44], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[47], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[50], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[53], 1, CLUTTER_X_AXIS,  angle);

      break;

    case 4:
      rotate (index[ 6], 1, CLUTTER_Y_AXIS,  angle);
      rotate (index[ 7], 1, CLUTTER_Y_AXIS,  angle);
      rotate (index[ 8], 1, CLUTTER_Y_AXIS,  angle);
      rotate (index[18], 1, CLUTTER_Y_AXIS, -angle);
      rotate (index[19], 1, CLUTTER_Y_AXIS, -angle);
      rotate (index[20], 1, CLUTTER_Y_AXIS, -angle);
      rotate (index[33], 1, CLUTTER_Y_AXIS,  angle);
      rotate (index[34], 1, CLUTTER_Y_AXIS,  angle);
      rotate (index[35], 1, CLUTTER_Y_AXIS,  angle);
      rotate (index[51], 1, CLUTTER_Y_AXIS,  angle);
      rotate (index[52], 1, CLUTTER_Y_AXIS,  angle);
      rotate (index[53], 1, CLUTTER_Y_AXIS,  angle);

      break;

    case 5:
      rotate (index[ 2], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[ 5], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[ 8], 1, CLUTTER_X_AXIS,  angle);
      rotate (index[15], 1, CLUTTER_Y_AXIS,  angle);
      rotate (index[16], 1, CLUTTER_Y_AXIS,  angle);
      rotate (index[17], 1, CLUTTER_Y_AXIS,  angle);
      rotate (index[27], 1, CLUTTER_X_AXIS, -angle);
      rotate (index[30], 1, CLUTTER_X_AXIS, -angle);
      rotate (index[33], 1, CLUTTER_X_AXIS, -angle);
      rotate (index[36], 1, CLUTTER_Y_AXIS, -angle);
      rotate (index[37], 1, CLUTTER_Y_AXIS, -angle);
      rotate (index[38], 1, CLUTTER_Y_AXIS, -angle);

      break;
  }
}



static void
spin (ClutterTimeline *timeline,
      gint             frame,
      gpointer         data)
{
  static gfloat angle = 0;

  gint size = 45 + 2 * SPACE;

  angle += 0.1;

  clutter_actor_set_rotation (cube, CLUTTER_X_AXIS,
                              3 * angle + 30,
                              size, size, size);
  clutter_actor_set_rotation (cube, CLUTTER_Y_AXIS,
                              2 * angle + 60,
                              size, size, size);
  clutter_actor_set_rotation (cube, CLUTTER_Z_AXIS,
                              1 * angle + 90,
                              size, size, size);
}



static void
turn (ClutterTimeline *timeline,
      gint             frame,
      gpointer         data)
{
  static gfloat previous = 2;

  gfloat progress = clutter_timeline_get_progress (timeline);
  gfloat angle    = 0;
  gfloat time;

  if (progress < previous)
  {
    if (moves)
      untwist (move[--moves]);

    if (!moves)
    {
      g_free  (move);
      shuffle ();
    }
  }

  if (progress < 0.5)
    time = 2 * progress * progress;
  else
    time = 1 - 2 * (1 - progress) * (1 - progress);

  switch (move[moves - 1] % 3)
  {
    case 0:
      angle =  90 * time;
      break;

    case 1:
      angle = 180 * time;
      break;

    case 2:
      angle = -90 * time;
      break;
  }

  layout (move[moves - 1] / 3, angle);

  previous = progress;
}



int
main (int   argc,
      char *argv[])
{
  gfloat           width;
  gfloat           height;
  gfloat           period = PERIOD;

  ClutterActor    *stage;
  ClutterScore    *score;
  ClutterTimeline *timeline_spin;
  ClutterTimeline *timeline_turn;

  clutter_init      (&argc, &argv);
  clutterrific_init (&argc, &argv);

  {
    const gchar *dir = g_get_user_special_dir (G_USER_DIRECTORY_PICTURES);

    file = clutterrific_list (dir, "(?i)\\.(" EXTENSIONS ")$");
  }

  cogl_set_depth_test_enabled (TRUE);

  /* create black stage actor */
  {
    ClutterColor bg = { 0, 0, 0, 255 };
    stage = clutter_stage_get_default ();

    clutter_stage_set_color (CLUTTER_STAGE (stage), &bg);
    clutter_actor_show_all  (stage);
    clutter_actor_get_size  (stage, &width, &height);
  }

  /* create animation score */
  score         = clutter_score_new    ();
  timeline_spin = clutter_timeline_new (period);
  timeline_turn = clutter_timeline_new (period);

  g_signal_connect (timeline_spin, "new-frame", G_CALLBACK (spin), NULL);
  g_signal_connect (timeline_turn, "new-frame", G_CALLBACK (turn), NULL);

  clutter_score_append   (score, NULL, timeline_spin);
  clutter_score_append   (score, NULL, timeline_turn);
  clutter_score_set_loop (score, TRUE);
  clutter_score_start    (score);

  /* create cube tiles */
  cube = clutter_group_new ();

  {
    gint i, j, k, l;

    for (i = 0; i < 6; i++)
    for (j = 0; j < 3; j++)
    for (k = 0; k < 3; k++)
    {
      l = pack (i, j, k);

      tile[l][0] = clutter_group_new ();
      tile[l][1] = clutter_group_new ();
      tile[l][2] = NULL;

      if (j || k)
        tile[l][2] = clutter_clone_new (tile[i * 9][2]);
      else if (file != NULL)
      {
        gint try;

        for (try = 0; tile[l][2] == NULL && try < file->len; try++)
        {
          const gchar *path = file->pdata[g_random_int_range (0, file->len)];

          tile[l][2] = clutter_texture_new_from_file (path, NULL);
        }
      }

      if (tile[l][2] == NULL)
      {
        ClutterColor colour;

        clutter_color_from_hls (&colour, (i * 60 + 0) % 360, 0.8, 1);

        tile[l][2] = clutter_rectangle_new_with_color (&colour);
      }

      clutter_actor_set_position  (tile[l][0], (k + 1) * SPACE,
                                               (j + 1) * SPACE);
      clutter_actor_set_depth     (tile[l][0], 90 + 4 * SPACE);
      clutter_actor_set_size      (tile[l][2], 90, 90);
      clutter_actor_set_clip      (tile[l][2], k * 30, j * 30, 30, 30);
      clutter_container_add_actor (CLUTTER_CONTAINER (tile[l][1]),
                                                      tile[l][2]);
      clutter_container_add_actor (CLUTTER_CONTAINER (tile[l][0]),
                                                      tile[l][1]);
      clutter_container_add_actor (CLUTTER_CONTAINER (cube), tile[l][0]);
    }
  }

  {
    gint scale = SCALE * height / 480;
    gint size  = scale * (90 + 4 * SPACE);

    clutter_actor_set_position  (cube, (width  - size) / 2,
                                       (height - size) / 2);
    clutter_actor_set_depth     (cube, -size);
    clutter_actor_set_scale     (cube, scale, scale);
    clutter_container_add_actor (CLUTTER_CONTAINER (stage), cube);
    clutter_actor_show_all      (stage);
  }

  clutter_main ();

  g_object_unref (timeline_turn);
  g_object_unref (timeline_spin);
  g_object_unref (score);

  if (moves)
    g_free (move);

  g_ptr_array_unref (file);

  return 0;
}
