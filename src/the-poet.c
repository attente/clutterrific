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



#define CAP          7

#define STEPS       64

#define QUOTE(x)    #x

#define EXPAND(x)   QUOTE(x)

#define FONT        "the-poet.font"

#define FONT_DATA_0 FONT

#define FONT_DATA_1 "data/" FONT

#define FONT_DATA_2 "../" FONT_DATA_1

#define FONT_DATA_3 EXPAND(DATA_DIR) "/" FONT



#include <math.h>

#include <clutter/clutter.h>

#include "clutterrific.h"



#ifndef M_PI
  #define M_PI 3.141592653589793238
#endif



typedef struct
{
  gfloat x;

  gfloat y;

  gfloat z;
}
Point;



typedef struct
{
  gint    n;

  Point  *p;

  gfloat  cap[2];
}
Curve;



typedef struct
{
  gint    n;

  Point   end;

  gfloat  life;

  GSList *time;

  GSList *curve;
}
Glyph;



static gfloat        shift;

static GHashTable   *font;

static ClutterActor *stage;



static void
destroy (gpointer data,
         gpointer user)
{
  g_free (data);
}



static void
curve_free (Curve *curve)
{
  g_free (curve->p);
  g_free (curve);
}



static void
curve_destroy (gpointer data,
               gpointer user)
{
  curve_free (data);
}



static void
glyph_free (Glyph *glyph)
{
  g_slist_foreach (glyph->time, destroy, NULL);
  g_slist_free    (glyph->time);

  g_slist_foreach (glyph->curve, curve_destroy, NULL);
  g_slist_free    (glyph->curve);

  g_free (glyph);
}



static Curve *
curve_read (GVariant *data)
{
  const GVariantType *type = (const GVariantType *) "(dda(ddd))";

  Curve        *curve;
  GVariant     *array;
  GVariantIter  iter;
  gdouble       cap[2];
  gint          points;
  gdouble       p[3];
  gint          i;

  if (data == NULL || !g_variant_is_of_type (data, type))
    return NULL;

  g_variant_get (data, "(dd@a(ddd))", cap + 0, cap + 1, &array);
  points = g_variant_n_children (array);

  curve         = g_new (Curve, 1);
  curve->n      = (points - 1) / 3;
  curve->p      = g_new (Point, points);
  curve->cap[0] = cap[0];
  curve->cap[1] = cap[1];

  g_variant_iter_init (&iter, array);

  for (i = 0; g_variant_iter_loop (&iter, "(ddd)", p + 0, p + 1, p + 2); i++)
  {
    curve->p[i].x = p[0];
    curve->p[i].y = p[1];
    curve->p[i].z = p[2];
  }

  return curve;
}



static Glyph *
glyph_read (GVariant *data)
{
  const GVariantType *type = (const GVariantType *) "(a(dd(dda(ddd)))(dd))";

  Glyph        *glyph;
  GVariant     *curve;
  GVariantIter  iter;
  gdouble       x, y;
  gdouble       t[2];

  if (data == NULL || !g_variant_is_of_type (data, type))
    return NULL;

  g_variant_get_child (data, 1, "(dd)", &x, &y);

  glyph        = g_new (Glyph, 1);
  glyph->n     = 0;
  glyph->end.x = x;
  glyph->end.y = y;
  glyph->life  = 0;
  glyph->time  = NULL;
  glyph->curve = NULL;

  data = g_variant_get_child_value (data, 0);

  g_variant_iter_init (&iter, data);

  while (g_variant_iter_loop (&iter, "(dd@(dda(ddd)))", t + 0, t + 1, &curve))
  {
    gfloat *f[2];

    glyph->life += t[0];
    glyph->life += t[1];

    f[0] = g_new (gfloat, 1);
    f[1] = g_new (gfloat, 1);
    *f[0] = t[0];
    *f[1] = t[1];
    glyph->time = g_slist_prepend (glyph->time, f[0]);
    glyph->time = g_slist_prepend (glyph->time, f[1]);

    glyph->curve = g_slist_prepend (glyph->curve, curve_read (curve));

    glyph->n++;
  }

  glyph->time  = g_slist_reverse (glyph->time);
  glyph->curve = g_slist_reverse (glyph->curve);

  return glyph;
}



static GHashTable *
font_read (GVariant *data)
{
  const GVariantType *type = (const GVariantType *) "a{s(a(dd(dda(ddd)))(dd))}";

  GHashTable   *font;
  GVariantIter  iter;
  gchar        *key;
  GVariant     *val;

  if (data == NULL || !g_variant_is_of_type (data, type))
    return NULL;

  font = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) glyph_free);

  g_variant_iter_init (&iter, data);

  while (g_variant_iter_loop (&iter, "{s@(a(dd(dda(ddd)))(dd))}", &key, &val))
    g_hash_table_insert (font, g_strdup (key), glyph_read (val));

  g_variant_unref (data);

  return font;
}



static GHashTable *
font_parse (const gchar *text)
{
  const GVariantType *type = (const GVariantType *) "a{s(a(dd(dda(ddd)))(dd))}";
  GVariant           *data = g_variant_parse (type, text, NULL, NULL, NULL);
  GHashTable         *font = data == NULL ? NULL : font_read (data);

  g_variant_unref (data);

  return font;
}



static GHashTable *
font_load (const gchar *file)
{
  GHashTable *font = NULL;
  gchar      *text = NULL;

  if (g_file_get_contents (file, &text, NULL, NULL))
    font = font_parse (text);

  g_free (text);

  return font;
}



static void
stroke_paint (const Point *p,
              gfloat       c0,
              gfloat       c1,
              gfloat       t)
{
  Point   q, dq, pdq;
  gfloat *data;
  gfloat  l;
  gint    n;
  gint    i;

  if (t < 0)
    return;
  else if (t > 1)
    t = 1;

  n = CAP + 1 + STEPS * t + CAP;

  data = g_new0 (gfloat, 3 * (1 + 2 * n + 1));

  dq.x = p[1].x - p[0].x;
  dq.y = p[1].y - p[0].y;

  l = sqrt (dq.x * dq.x + dq.y * dq.y);

  pdq.x = -dq.y * p[0].z / l;
  pdq.y =  dq.x * p[0].z / l;

  dq.x *= c0 * p[0].z / l;
  dq.y *= c0 * p[0].z / l;

  data[0] = p[0].x - dq.x;
  data[1] = p[0].y - dq.y;

  for (i = 0; i <= CAP; i++)
  {
    gfloat a = M_PI / 2 * (i + 1) / (CAP + 1);
    gfloat x =  sin (a);
    gfloat y = -cos (a);

    data[3 * (1 + 2 * i + 0) + 0] = p[0].x + y * dq.x - x * pdq.x;
    data[3 * (1 + 2 * i + 0) + 1] = p[0].y + y * dq.y - x * pdq.y;
    data[3 * (1 + 2 * i + 1) + 0] = p[0].x + y * dq.x + x * pdq.x;
    data[3 * (1 + 2 * i + 1) + 1] = p[0].y + y * dq.y + x * pdq.y;
  }

  for (i = 1; i <= STEPS && i - 1 <= STEPS * t; i++)
  {
    gfloat s = MIN (1.0 * i / STEPS, t);
    gfloat r = 1 - s;

    gfloat r2 = r * r;
    gfloat s2 = s * s;
    gfloat r3 = r * r2;
    gfloat s3 = s * s2;
    gfloat r2s = r2 * s;
    gfloat rs2 = r * s2;

    q.x = r3      * p[0].x
        + 3 * r2s * p[1].x
        + 3 * rs2 * p[2].x
        + s3      * p[3].x;
    q.y = r3      * p[0].y
        + 3 * r2s * p[1].y
        + 3 * rs2 * p[2].y
        + s3      * p[3].y;
    q.z = r3      * p[0].z
        + 3 * r2s * p[1].z
        + 3 * rs2 * p[2].z
        + s3      * p[3].z;

    dq.x = -r2             * p[0].x
         + r * (1 - 3 * s) * p[1].x
         + s * (2 - 3 * s) * p[2].x
         + s2              * p[3].x;
    dq.y = -r2             * p[0].y
         + r * (1 - 3 * s) * p[1].y
         + s * (2 - 3 * s) * p[2].y
         + s2              * p[3].y;

    l = sqrt (dq.x * dq.x + dq.y * dq.y);

    pdq.x = -dq.y * q.z / l;
    pdq.y =  dq.x * q.z / l;

    data[3 * (1 + 2 * (CAP + i) + 0) + 0] = q.x - pdq.x;
    data[3 * (1 + 2 * (CAP + i) + 0) + 1] = q.y - pdq.y;
    data[3 * (1 + 2 * (CAP + i) + 1) + 0] = q.x + pdq.x;
    data[3 * (1 + 2 * (CAP + i) + 1) + 1] = q.y + pdq.y;
  }

  dq.x *= c1 * q.z / l;
  dq.y *= c1 * q.z / l;

  for (i = 0; i < CAP; i++)
  {
    gfloat a = M_PI / 2 * (i + 1) / (CAP + 1);
    gfloat x = cos (a);
    gfloat y = sin (a);

    data[3 * (1 + 2 * (n - CAP + i) + 0) + 0] = q.x + y * dq.x - x * pdq.x;
    data[3 * (1 + 2 * (n - CAP + i) + 0) + 1] = q.y + y * dq.y - x * pdq.y;
    data[3 * (1 + 2 * (n - CAP + i) + 1) + 0] = q.x + y * dq.x + x * pdq.x;
    data[3 * (1 + 2 * (n - CAP + i) + 1) + 1] = q.y + y * dq.y + x * pdq.y;
  }

  data[3 * (1 + 2 * n) + 0] = q.x + dq.x;
  data[3 * (1 + 2 * n) + 1] = q.y + dq.y;

  {
    CoglHandle        buffer = cogl_vertex_buffer_new (1 + 2 * n + 1);
    CoglVerticesMode  mode   = COGL_VERTICES_MODE_TRIANGLE_STRIP;
    CoglAttributeType type   = COGL_ATTRIBUTE_TYPE_FLOAT;
    gint              stride = 3 * sizeof (gfloat);

    cogl_vertex_buffer_add (buffer, "gl_Vertex", 3, type, TRUE, stride, data);
    cogl_vertex_buffer_draw (buffer, mode, 0, 1 + 2 * n + 1);
    cogl_handle_unref (buffer);
  }

  g_free (data);
}



static void
curve_paint (const Curve *c,
             gfloat       t)
{
  gint i;

  if (c == NULL)
    return;

  t = c->n * CLAMP (t, 0, 1);

  for (i = 0; i < t; i++)
  {
    gfloat c0 = i ?            1 : c->cap[0];
    gfloat c1 = i + 1 < c->n ? 1 : c->cap[1];

    stroke_paint (c->p + 3 * i, c0, c1, t - i);
  }
}



static void
glyph_paint (const Glyph *g,
             gfloat       t)
{
  if (g != NULL)
  {
    GSList *time  = g->time;
    GSList *curve = g->curve;

    while (t > 0 && time != NULL && curve != NULL)
    {
      gfloat life = * (gfloat *) time->data;

      curve_paint (curve->data, t / life);
      t -= life;

      time = time->next;
      curve = curve->next;

      if (time != NULL)
      {
        t -= * (gfloat *) time->data;

        time = time->next;
      }
    }
  }
}



static const Glyph *
glyph_lookup (GHashTable  *font,
              const gchar *name,
              gboolean     head,
              gboolean     tail)
{
  const Glyph *glyph = NULL;
  gchar       *key   = NULL;

  if (head && tail)
  {
    key   = g_strdup_printf ("_%s_", name);
    glyph = g_hash_table_lookup (font, key);

    g_free (key);
  }

  if (glyph == NULL && head)
  {
    key   = g_strdup_printf ("_%s", name);
    glyph = g_hash_table_lookup (font, key);

    g_free (key);
  }

  if (glyph == NULL && tail)
  {
    key   = g_strdup_printf ("%s_", name);
    glyph = g_hash_table_lookup (font, key);

    g_free (key);
  }

  return glyph == NULL ? g_hash_table_lookup (font, name) : glyph;
}



static void
text_paint (GHashTable  *font,
            const gchar *text,
            gfloat       x,
            gfloat       y,
            gfloat       t)
{
  gchar name[2];
  gint  i;

  name[1] = 0;

  for (i = 0; t > 0 && text[i]; i++)
  {
    const Glyph *glyph;

    gboolean head = !i           || g_ascii_isspace (text[i - 1]);
    gboolean tail = !text[i + 1] || g_ascii_isspace (text[i + 1]);

    /* XXX */
    if (text[i] == ' ')
    {
      cogl_translate (20, 0, 0);
      x += 20;
      continue;
    }
    if (text[i] == '\n')
    {
      cogl_translate (50-x, 60, 0);
      x = 50;
      continue;
    }

    name[0] = text[i];
    glyph   = glyph_lookup (font, name, head, tail);

    if (glyph != NULL)
    {
      glyph_paint (glyph, t);

      cogl_translate (glyph->end.x, glyph->end.y, 0);

      x += glyph->end.x;
      y += glyph->end.y;
      t -= glyph->life;
    }
  }
}



static gfloat
text_size (GHashTable  *font,
           const gchar *text)
{
  gchar  name[2];
  gfloat x = 0;
  gint   i;

  name[1] = 0;

  for (i = 0; text[i]; i++)
  {
    gboolean head = !i           || g_ascii_isspace (text[i - 1]);
    gboolean tail = !text[i + 1] || g_ascii_isspace (text[i + 1]);

    const Glyph *glyph;

    name[0] = text[i];
    glyph   = glyph_lookup (font, name, head, tail);

    if (glyph != NULL)
      x += glyph->end.x;
  }

  return x;
}



static void
paint_XXX (void)
{
  static gfloat  t = -20.000;
  static gfloat dt =  0.03125;

  cogl_translate (50, 100, 0);
  cogl_scale (1, 1, 1);

  cogl_set_source_color4f (0.0, 0.0, 0.0, 1.0);

  /* text_paint (font, "dear gnome friends\n\ni love you guys\n\nsincerely\nwilliam hua", 50, 0, t); */
  /* text_paint (font, "the quick brown fox\njumped over the lazy dog", 50, 0, t); */
  text_paint (font, "hi everybody\n\nim still travelling at the\nmoment and if ive learned\nanything from this experience\nits that between travelling\ndistractions and a lack of decent", 50, 0, t);

  t += dt;
}

static gboolean
redraw_XXX (gpointer data)
{
  clutter_actor_queue_redraw (stage);

  return TRUE;
}



int
main (int   argc,
      char *argv[])
{
  shift = 0;

  if ((font = font_load (FONT_DATA_0)) == NULL)
  if ((font = font_load (FONT_DATA_1)) == NULL)
  if ((font = font_load (FONT_DATA_2)) == NULL)
  if ((font = font_load (FONT_DATA_3)) == NULL)
  {
    g_error ("No template font");

    return 1;
  }

  clutter_init      (&argc, &argv);
  clutterrific_init (&argc, &argv);

  {
    ClutterColor bg = { 255, 255, 255, 255 };

    stage = clutter_stage_get_default ();
    clutter_stage_set_color (CLUTTER_STAGE (stage), &bg);
    clutter_actor_show_all (stage);
  }

  g_timeout_add          (10, redraw_XXX, NULL);
  g_signal_connect_after (stage, "paint", paint_XXX, NULL);

  clutter_main ();

  return 0;
}
