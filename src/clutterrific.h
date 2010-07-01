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



#ifndef __CLUTTERRIFIC_H__
#define __CLUTTERRIFIC_H__



#include <glib.h>



void        clutterrific_init    (int           *argc,
                                  char        ***argv);

gfloat      clutterrific_x       (gfloat         x);
gfloat      clutterrific_x_min   (gfloat         x);
gfloat      clutterrific_x_max   (gfloat         x);

gfloat      clutterrific_y       (gfloat         y);
gfloat      clutterrific_y_min   (gfloat         y);
gfloat      clutterrific_y_max   (gfloat         y);

gdouble     clutterrific_delta   (void);

GPtrArray * clutterrific_list    (const gchar   *path,
                                  const gchar   *pattern);

void        clutterrific_shuffle (GPtrArray     *array);



#endif /* __CLUTTERRIFIC_H__ */
