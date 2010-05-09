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



void
clutterrific_init (int    *argc,
                   char ***argv)
{
  const gchar *id = g_getenv ("XSCREENSAVER_WINDOW");
  gchar       *end;

  if (id != NULL)
  {
    Window window = (Window) g_ascii_strtoull (id, &end, 0);

    if (window && end != NULL && (!*end || *end == ' ') && (window < G_MAXULONG && errno != ERANGE))
      clutter_x11_set_stage_foreign (CLUTTER_STAGE (clutter_stage_get_default ()), window);
  }
}
