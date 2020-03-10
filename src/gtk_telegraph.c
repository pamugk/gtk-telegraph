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

struct _GtktelegraphPrivate
{
	GtkStack* drawerStack;
	GtkBox* chatsDrawer;
	GtkBox* settingsDrawer;
	GtkBox* messagesBox;
	GtkRevealer* infoboxRevealer;
	GtkWindow* callsDialog;
	GtkWindow* contactsDialog;
	GtkWindow* settingsDialog;
	GtkAboutDialog* aboutDialog;
};

GtkCssProvider* darkProvider;
GtkCssProvider* lightProvider;
GtktelegraphPrivate *priv;
GdkScreen* screen;

/* Create a new window loading a file */
static void
gtk_telegraph_new_window (GApplication *app,
                           GFile        *file){
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
	screen = gtk_style_context_get_screen(context);
	darkProvider = gtk_css_provider_new ();
	gtk_css_provider_load_from_path (darkProvider, "css/main_dark.css", NULL);
	lightProvider = gtk_css_provider_new ();
	gtk_css_provider_load_from_path (lightProvider, "css/main_light.css", NULL);
	GtkCssProvider* provider = gtk_css_provider_new ();
	gtk_css_provider_load_from_path (provider, "css/main_light.css", NULL);
	gtk_style_context_add_provider_for_screen (screen,
                                    GTK_STYLE_PROVIDER(provider),
                                    GTK_STYLE_PROVIDER_PRIORITY_USER);
	gtk_style_context_add_provider_for_screen (screen,
                                    GTK_STYLE_PROVIDER(lightProvider),
                                    GTK_STYLE_PROVIDER_PRIORITY_USER);
	

	priv->drawerStack = GTK_STACK (gtk_builder_get_object(builder, "drawerStack"));
	priv->chatsDrawer = GTK_BOX (gtk_builder_get_object(builder, "chatsDrawer"));
	priv->settingsDrawer = GTK_BOX (gtk_builder_get_object(builder, "settingsDrawer"));
	priv->messagesBox = GTK_BOX (gtk_builder_get_object(builder, "messagesBox"));
	priv->infoboxRevealer = GTK_REVEALER (gtk_builder_get_object(builder, "infoboxRevealer"));
	priv->callsDialog = GTK_WINDOW(gtk_builder_get_object(builder, "callsDialog"));
	priv->contactsDialog = GTK_WINDOW(gtk_builder_get_object(builder, "contactsDialog"));
	priv->settingsDialog = GTK_WINDOW(gtk_builder_get_object(builder, "settingsDialog"));
	priv->aboutDialog = GTK_ABOUT_DIALOG(gtk_builder_get_object(builder, "aboutDialog"));
	
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
gtk_telegraph_activate (GApplication *application){
	gtk_telegraph_new_window (application, NULL);
}

static void
gtk_telegraph_open (GApplication  *application,
                     GFile        **files,
                     gint           n_files,
                     const gchar   *hint){
	gint i;

	for (i = 0; i < n_files; i++)
		gtk_telegraph_new_window (application, files[i]);
}

static void
gtk_telegraph_init (Gtktelegraph *object){
	object->priv = G_TYPE_INSTANCE_GET_PRIVATE (object, GTK_TELEGRAPH_TYPE_APPLICATION, GtktelegraphPrivate);
}

static void
gtk_telegraph_finalize (GObject *object){
	G_OBJECT_CLASS (gtk_telegraph_parent_class)->finalize (object);
}

static void
gtk_telegraph_class_init (GtktelegraphClass *klass){
	G_APPLICATION_CLASS (klass)->activate = gtk_telegraph_activate;
	G_APPLICATION_CLASS (klass)->open = gtk_telegraph_open;

	g_type_class_add_private (klass, sizeof (GtktelegraphPrivate));

	G_OBJECT_CLASS (klass)->finalize = gtk_telegraph_finalize;
}

add_to_contacts_btn_clicked(GtkButton *button){
	printf("Contact added\n");
}

block_user_btn_clicked (GtkButton *button, gpointer user_data){
	printf("User blocked\n");
}

bookmark_btn_clicked (GtkButton *button){
	gtk_stack_set_visible_child(priv->drawerStack, priv->chatsDrawer);
}

calls_btn_clicked(GtkButton *button){
	gtk_widget_show(priv->callsDialog);
}

calls_close_btn_clicked(GtkButton *button){
	gtk_widget_hide(priv->callsDialog);
}

clear_history_btn_clicked (GtkButton *button, gpointer user_data){
	printf("History cleared\n");
}

contacts_btn_clicked(GtkButton *button){
	gtk_widget_show(priv->contactsDialog);
}

contacts_close_btn_clicked(GtkButton *button){
	gtk_widget_hide(priv->contactsDialog);
}

delete_chat_btn_clicked (GtkButton *button, gpointer user_data){
	printf("Chat deleted\n");
}

drawer_btn_clicked (GtkButton *button){
	gtk_stack_set_visible_child(priv->drawerStack, priv->settingsDrawer);
}

infopanel_btn_clicked (GtkButton *button){
	gboolean childRevealed = gtk_revealer_get_reveal_child (priv->infoboxRevealer);
	gtk_revealer_set_reveal_child (priv->infoboxRevealer, !childRevealed);
}

message_entry_activate(GtkEntry *messageEntry, gpointer user_data){
	const gchar *text = gtk_entry_get_text(messageEntry);
	if (strcmp(text, "") != 0){
		GtkLabel* messageLabel = gtk_label_new(text);
		gtk_box_pack_start(priv->messagesBox, messageLabel, FALSE, FALSE, 0);
		gtk_widget_set_halign(messageLabel, GTK_ALIGN_END);
		gtk_widget_set_margin_end(messageLabel, 20);
		gtk_widget_set_visible (messageLabel, TRUE);
		gtk_entry_set_text (messageEntry, "");
	}	
	return TRUE;
}

nm_switch_activated(GtkSwitch* widget, gboolean state, gpointer user_data){
	if (state) {
		gtk_style_context_remove_provider_for_screen(screen, lightProvider);
		gtk_style_context_add_provider_for_screen (screen,
		                                GTK_STYLE_PROVIDER(darkProvider),
		                                GTK_STYLE_PROVIDER_PRIORITY_USER);
	}
	else {
		gtk_style_context_remove_provider_for_screen(screen, darkProvider);
		gtk_style_context_add_provider_for_screen (screen,
		                                GTK_STYLE_PROVIDER(lightProvider),
		                                GTK_STYLE_PROVIDER_PRIORITY_USER);
	}
	gtk_switch_set_state (widget, state);
	return TRUE;
}

on_modal_window_delete(GtkWidget *widget, GdkEvent  *event, gpointer user_data){
	gtk_widget_hide(widget);
	return TRUE;
}

on_window_destroy(GtkWindow *window){
	cleanup();
}

settings_btn_clicked(GtkButton *button){
	gtk_widget_show(priv->settingsDialog);
}

Gtktelegraph *
gtk_telegraph_new (void)
{
	return g_object_new (gtk_telegraph_get_type (),
	                     "application-id", "org.gnome.gtk_telegraph",
	                     "flags", G_APPLICATION_HANDLES_OPEN,
	                     NULL);
}