/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * gtk-telegraph.h
 * Copyright (C) 2020 PAM <pamugk@gmail.com>
 * 
 * gtk-telegraph is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * gtk-telegraph is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GTK_TELEGRAPH_
#define _GTK_TELEGRAPH_

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_TELEGRAPH_TYPE_APPLICATION             (gtk_telegraph_get_type ())
#define GTK_TELEGRAPH_APPLICATION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TELEGRAPH_TYPE_APPLICATION, Gtktelegraph))
#define GTK_TELEGRAPH_APPLICATION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TELEGRAPH_TYPE_APPLICATION, GtktelegraphClass))
#define GTK_TELEGRAPH_IS_APPLICATION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TELEGRAPH_TYPE_APPLICATION))
#define GTK_TELEGRAPH_IS_APPLICATION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TELEGRAPH_TYPE_APPLICATION))
#define GTK_TELEGRAPH_APPLICATION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TELEGRAPH_TYPE_APPLICATION, GtktelegraphClass))

typedef struct _GtktelegraphClass GtktelegraphClass;
typedef struct _Gtktelegraph Gtktelegraph;
typedef struct _GtktelegraphPrivate GtktelegraphPrivate;

struct _GtktelegraphClass
{
	GtkApplicationClass parent_class;
};

struct _Gtktelegraph
{
	GtkApplication parent_instance;

	GtktelegraphPrivate *priv;

};

GType gtk_telegraph_get_type (void) G_GNUC_CONST;
Gtktelegraph *gtk_telegraph_new (void);

/* Callbacks */

G_END_DECLS

#endif /* _APPLICATION_H_ */

