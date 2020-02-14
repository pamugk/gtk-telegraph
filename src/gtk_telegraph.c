/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * gtk-telegraph.c
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
#include "gtk_telegraph.h"
#include "controller.h"

#include <glib/gi18n.h>



/* For testing propose use the local (not installed) ui file */
/* #define UI_FILE PACKAGE_DATA_DIR"/ui/gtk_telegraph.ui" */
#define UI_FILE "src/gtk_telegraph.ui"
#define TOP_WINDOW "window"


G_DEFINE_TYPE (Gtktelegraph, gtk_telegraph, GTK_TYPE_APPLICATION);

/* ANJUTA: Macro GTK_TELEGRAPH_APPLICATION gets Gtktelegraph - DO NOT REMOVE */
struct _GtktelegraphPrivate
{
	/* ANJUTA: Widgets declaration for gtk_telegraph.ui - DO NOT REMOVE */
	GtkStack* drawerStack;
	GtkWidget* chatsDrawer;
	GtkWidget* settingsDrawer;
	GtkRevealer* infoboxRevealer;
};

GtktelegraphPrivate *priv;

/* Create a new window loading a file */
static void
gtk_telegraph_new_window (GApplication *app,
                           GFile        *file)
{
	GtkWidget *window;

	GtkBuilder *builder;
	GError* error = NULL;

	priv = GTK_TELEGRAPH_APPLICATION(app)->priv;

	/* Load UI from file */
	builder = gtk_builder_new ();
	if (!gtk_builder_add_from_file (builder, UI_FILE, &error))
	{
		g_critical ("Couldn't load builder file: %s", error->message);
		g_error_free (error);
	}

	/* Auto-connect signal handlers */
	gtk_builder_connect_signals (builder, app);

	/* Get the window object from the ui file */
	window = GTK_WIDGET (gtk_builder_get_object (builder, TOP_WINDOW));
        if (!window)
        {
		g_critical ("Widget \"%s\" is missing in file %s.",
				TOP_WINDOW,
				UI_FILE);
        }
	GtkStyleContext* context = gtk_widget_get_style_context (window);
	GtkCssProvider *provider = gtk_css_provider_new ();
	gtk_css_provider_load_from_path (provider, "css/main.css", NULL);
	gtk_style_context_add_provider (context,
                                    GTK_STYLE_PROVIDER(provider),
                                    GTK_STYLE_PROVIDER_PRIORITY_THEME);
	
	/* ANJUTA: Widgets initialization for gtk_telegraph.ui - DO NOT REMOVE */
	priv->drawerStack = GTK_STACK (gtk_builder_get_object(builder, "drawerStack"));
	priv->chatsDrawer = GTK_WIDGET (gtk_builder_get_object(builder, "chatsDrawer"));
	priv->settingsDrawer = GTK_WIDGET (gtk_builder_get_object(builder, "settingsDrawer"));
	priv->infoboxRevealer = GTK_REVEALER (gtk_builder_get_object(builder, "infoboxRevealer"));
	
	g_object_unref (builder);
	
	
	gtk_window_set_application (GTK_WINDOW (window), GTK_APPLICATION (app));
	if (file != NULL)
	{
		/* TODO: Add code here to open the file in the new window */
	}

	gtk_widget_show_all (GTK_WIDGET (window));
}


/* GApplication implementation */
static void
gtk_telegraph_activate (GApplication *application)
{
	gtk_telegraph_new_window (application, NULL);
}

static void
gtk_telegraph_open (GApplication  *application,
                     GFile        **files,
                     gint           n_files,
                     const gchar   *hint)
{
	gint i;

	for (i = 0; i < n_files; i++)
		gtk_telegraph_new_window (application, files[i]);
}

static void
gtk_telegraph_init (Gtktelegraph *object)
{
	object->priv = G_TYPE_INSTANCE_GET_PRIVATE (object, GTK_TELEGRAPH_TYPE_APPLICATION, GtktelegraphPrivate);
}

static void
gtk_telegraph_finalize (GObject *object)
{
	G_OBJECT_CLASS (gtk_telegraph_parent_class)->finalize (object);
}

static void
gtk_telegraph_class_init (GtktelegraphClass *klass)
{
	G_APPLICATION_CLASS (klass)->activate = gtk_telegraph_activate;
	G_APPLICATION_CLASS (klass)->open = gtk_telegraph_open;

	g_type_class_add_private (klass, sizeof (GtktelegraphPrivate));

	G_OBJECT_CLASS (klass)->finalize = gtk_telegraph_finalize;
}

on_window_destroy(GtkWindow *window)
{
	cleanup();
}

drawer_btn_clicked (GtkButton *button)
{
	gtk_stack_set_visible_child(priv->drawerStack, priv->settingsDrawer);
}

bookmark_btn_clicked (GtkButton *button)
{
	gtk_stack_set_visible_child(priv->drawerStack, priv->chatsDrawer);
}

infopanel_btn_clicked (GtkButton *button)
{
	gboolean childRevealed = gtk_revealer_get_reveal_child (priv->infoboxRevealer);
	gtk_revealer_set_reveal_child (priv->infoboxRevealer, !childRevealed);
}

message_entry_editing_done(GtkCellEditable *cell_editable, gpointer user_data)
{
	printf ("debugged");
}

Gtktelegraph *
gtk_telegraph_new (void)
{
	return g_object_new (gtk_telegraph_get_type (),
	                     "application-id", "org.gnome.gtk_telegraph",
	                     "flags", G_APPLICATION_HANDLES_OPEN,
	                     NULL);
}

