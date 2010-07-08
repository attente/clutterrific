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



#include <errno.h>

#include <clutter/clutter.h>
#include <clutter/x11/clutter-x11.h>



static gfloat width;

static gfloat height;



static void list (GPtrArray    *array,
                  const gchar  *path,
                  const GRegex *regex);



void
clutterrific_init (int    *argc,
                   char ***argv)
{
  const gchar  *id = g_getenv ("XSCREENSAVER_WINDOW");
  ClutterActor *stage;
  gchar        *end;

  stage = clutter_stage_get_default ();

  if (id != NULL)
  {
    Window window = (Window) g_ascii_strtoull (id, &end, 0);

    if (window &&
        end != NULL &&
        (!*end || *end == ' ') &&
        (window < G_MAXULONG && errno != ERANGE))
      clutter_x11_set_stage_foreign (CLUTTER_STAGE (stage), window);
  }

  clutter_actor_get_size (stage, &width, &height);
}



gfloat
clutterrific_width (void)
{
  return width;
}



gfloat
clutterrific_height (void)
{
  return height;
}



void
clutterrific_pack (gfloat *w,
                   gfloat *h,
                   gfloat  w0,
                   gfloat  h0)
{
  gfloat s = MIN (w0 / *w, h0 / *h);

  *w *= s;
  *h *= s;
}



void
clutterrific_wrap (gfloat *w,
                   gfloat *h,
                   gfloat  w0,
                   gfloat  h0)
{
  gfloat s = MAX (w0 / *w, h0 / *h);

  *w *= s;
  *h *= s;
}



gdouble
clutterrific_delta (void)
{
  static GTimeVal then = { 0 };

  gfloat   dt = 0;
  GTimeVal now;

  g_get_current_time (&now);

  if (then.tv_sec)
    dt = now.tv_sec - then.tv_sec + 1E-6 * (now.tv_usec - then.tv_usec);

  then = now;

  return dt;
}



GPtrArray *
clutterrific_list (const gchar *path,
                   const gchar *pattern)
{
  GPtrArray *array = g_ptr_array_new_with_free_func (g_free);
  GRegex    *regex = g_regex_new (pattern, G_REGEX_OPTIMIZE, 0, NULL);

  list (array, path, regex);

  g_regex_unref (regex);

  return array;
}



void
clutterrific_shuffle (GPtrArray *array)
{
  if (array != NULL)
  {
    gint i, j;

    for (i = 0; i < array->len; i++)
    {
      j = g_random_int_range (i, array->len);

      if (i != j)
      {
        gpointer      p = array->pdata[i];
        array->pdata[i] = array->pdata[j];
        array->pdata[j] = p;
      }
    }
  }
}



static void
list (GPtrArray    *array,
      const gchar  *path,
      const GRegex *regex)
{
  GDir *dir = g_dir_open (path, 0, NULL);

  if (dir != NULL)
  {
    const gchar *name;

    while ((name = g_dir_read_name (dir)) != NULL)
    {
      if (name[0] != '.')
      {
        gchar *file = g_build_filename (path, name, NULL);

        if (g_file_test (file, G_FILE_TEST_IS_DIR))
        {
          list (array, file, regex);

          g_free (file);
        }
        else if (g_regex_match (regex, file, 0, NULL))
          g_ptr_array_add (array, file);
        else
          g_free (file);
      }
    }

    g_dir_close (dir);
  }
}
