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

#include <glib/gi18n.h>



/* For testing propose use the local (not installed) ui file */
/* #define UI_FILE PACKAGE_DATA_DIR"/ui/gtk_telegraph.ui" */
#define UI_FILE "src/gtk_telegraph.ui"
#define TOP_WINDOW "window"


G_DEFINE_TYPE (Gtktelegraph, gtk_telegraph, GTK_TYPE_APPLICATION);

struct _GtktelegraphPrivate
{
	GtkStack* mainStack;
	GtkBox* mainBox;
	GtkStack* drawerStack;
	GtkBox* chatsDrawer;
	GtkSpinner* connectionSpinner;
	GtkListBox* chatsBox;
	GtkLabel* chatsBoxPlaceholder;
	GtkBox* settingsDrawer;
	GtkLabel* curUserCellphone;
	GtkLabel* curUserFullname;
	GtkRevealer* topboxRevealer;
	GtkRevealer* messageRevealer;
	GtkStack* messagesStack;
	GtkLabel* messagesPlaceholder;
	GtkRevealer* infoboxRevealer;
	GtkLabel* contactFullname;
	GtkStack* contactMobileStack;
	GtkLabel* contactMobileValueLabel;
	GtkRevealer* contactUsernameRevealer;
	GtkLabel* contactUsername;
	GtkRevealer* contactBiographyRevealer;
	GtkLabel* contactBioLabel;
	GtkWindow* callsDialog;
	GtkWindow* contactsDialog;
	GtkWindow* settingsDialog;
	GtkLabel* curUserFullname1;
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
	gtk_css_provider_load_from_path (provider, "css/main.css", NULL);
	gtk_style_context_add_provider_for_screen (screen,
                                    GTK_STYLE_PROVIDER(provider),
                                    GTK_STYLE_PROVIDER_PRIORITY_USER);
	gtk_style_context_add_provider_for_screen (screen,
                                    GTK_STYLE_PROVIDER(lightProvider),
                                    GTK_STYLE_PROVIDER_PRIORITY_USER);
	
	priv->mainStack =
		GTK_STACK (gtk_builder_get_object(builder, "mainStack"));
	priv->mainBox =
		GTK_BOX (gtk_builder_get_object(builder, "mainBox"));
	priv->drawerStack =
		GTK_STACK (gtk_builder_get_object(builder, "drawerStack"));
	priv->chatsDrawer =
		GTK_BOX (gtk_builder_get_object(builder, "chatsDrawer"));
	priv->connectionSpinner =
		GTK_SPINNER (gtk_builder_get_object(builder, "connectionSpinner"));
	gtk_widget_set_visible (priv->chatsBoxPlaceholder, TRUE);
	priv->settingsDrawer =
		GTK_BOX (gtk_builder_get_object(builder, "settingsDrawer"));
	priv->curUserCellphone =
		GTK_LABEL (gtk_builder_get_object(builder, "curUserCellphone"));
	priv->curUserFullname =
		GTK_LABEL (gtk_builder_get_object(builder, "curUserFullname"));
	priv->topboxRevealer = 
		GTK_REVEALER (gtk_builder_get_object(builder, "topboxRevealer"));
	priv->messageRevealer =
		GTK_REVEALER (gtk_builder_get_object(builder, "messageRevealer"));
	priv->messagesStack =
		GTK_STACK (gtk_builder_get_object(builder, "messagesStack"));
	priv->messagesPlaceholder =
		GTK_LABEL (gtk_builder_get_object(builder, "messagesPlaceholder"));
	priv->chatsBox = GTK_LIST_BOX (gtk_builder_get_object(builder, 
		"chatsBox"));
	priv->chatsBoxPlaceholder = GTK_LABEL(gtk_label_new("No contacts found"));
	gtk_list_box_set_placeholder (priv->chatsBox, priv->chatsBoxPlaceholder);
	priv->infoboxRevealer =
		GTK_REVEALER (gtk_builder_get_object(builder, "infoboxRevealer"));
	priv->contactFullname =
		GTK_LABEL(gtk_builder_get_object(builder, "contactFullname"));
	priv->contactMobileStack =
		GTK_STACK (gtk_builder_get_object(builder, "contactMobileStack"));
	priv->contactMobileValueLabel =
		GTK_LABEL (gtk_builder_get_object(builder, "contactMobileValueLabel"));
	priv->contactUsernameRevealer =
		GTK_REVEALER (gtk_builder_get_object(builder, "contactUsernameRevealer"));
	priv->contactUsername =
		GTK_LABEL (gtk_builder_get_object(builder, "contactUsername"));
	priv->contactBiographyRevealer =
		GTK_REVEALER (gtk_builder_get_object(builder, "contactBiographyRevealer"));
	priv->contactBioLabel =
		GTK_LABEL (gtk_builder_get_object(builder, "contactBioLabel"));
	priv->callsDialog =
		GTK_WINDOW(gtk_builder_get_object(builder, "callsDialog"));
	priv->contactsDialog =
		GTK_WINDOW(gtk_builder_get_object(builder, "contactsDialog"));
	priv->settingsDialog =
		GTK_WINDOW(gtk_builder_get_object(builder, "settingsDialog"));
	priv->curUserFullname1 =
		GTK_LABEL (gtk_builder_get_object(builder, "curUserFullname1"));
	priv->aboutDialog =
		GTK_ABOUT_DIALOG(gtk_builder_get_object(builder, "aboutDialog"));
	
	g_object_unref (builder);
	
	gtk_window_set_application (GTK_WINDOW (window), GTK_APPLICATION (app));
	if (file != NULL)
	{
		/* TODO: Add code here to open the file in the new window */
	}
	gtk_widget_show_all (GTK_WIDGET (window));
}

/*Chat client interactions*/
void putMessage(const gchar* text, GtkBox* messagesBox, gboolean local) {
	GtkWidget* messageLabel = gtk_label_new(text);
	GtkStyleContext* context = 
		gtk_widget_get_style_context(messageLabel);
	gtk_style_context_add_class(context, "message");
	gtk_box_pack_start(messagesBox, messageLabel, FALSE, FALSE, 0);
	gtk_widget_set_halign(messageLabel, local ? GTK_ALIGN_END : GTK_ALIGN_START);
	gtk_widget_set_margin_end(messageLabel, 20);
	gtk_widget_set_visible (messageLabel, TRUE);
}

void collectAllMessages() {
	for (int i = 0; i < contacts->count; i += 1) {
		struct User* contact = contacts->list[i];
		GtkBox* messageBox = GTK_BOX(gtk_stack_get_child_by_name 
		                     	(priv->messagesStack, contact->id));
		struct MessageList* messages = getMessages (contact->id);
		for (int i = 0; i < messages->count; i += 1)
			putMessage(messages->list[i]->text, 
				messageBox, strcmp(userId, messages->list[i]->fromId) == 0);
	}
}

void acceptNewMessage(struct Message* message){
	putMessage (message->text, 
		GTK_BOX(gtk_stack_get_child_by_name (priv->messagesStack, message->fromId)),
	    FALSE);
}

void showCurrentUserDetails() {
	gtk_label_set_text(priv->curUserCellphone, user->phone);
	gtk_label_set_text(priv->curUserFullname, user->fullname);
	gtk_label_set_text(priv->curUserFullname1, user->fullname);
}

void showUserDetails(struct User* user) {
	gtk_revealer_set_reveal_child(priv->topboxRevealer, TRUE);
	gtk_revealer_set_reveal_child(priv->messageRevealer, TRUE);
	gtk_revealer_set_reveal_child(priv->infoboxRevealer, TRUE);
	gtk_label_set_text(priv->contactFullname, user->fullname);
	gtk_label_set_text(priv->contactMobileValueLabel, user->phone);
	gtk_label_set_text(priv->contactUsername, user->username);
	if (user->biography == NULL || strcmp(user->biography, "") == 0)
		gtk_revealer_set_reveal_child(priv->contactBiographyRevealer, FALSE);
	else {
		gtk_label_set_text (priv->contactBioLabel, user->biography);
		gtk_revealer_set_reveal_child(priv->contactBiographyRevealer, TRUE);
	}
}

GtkWidget* makeContact(struct User* user) {
	GtkGrid* contactInfo = GTK_GRID(gtk_grid_new());
	GtkStyleContext* context = 
		gtk_widget_get_style_context(GTK_WIDGET(contactInfo));
	gtk_style_context_add_class(context, "contact");
	gtk_widget_set_halign (GTK_WIDGET(contactInfo), GTK_ALIGN_FILL);

	GtkWidget* contactAvatar = gtk_image_new();
	gtk_widget_set_visible (contactAvatar, TRUE);
	gtk_widget_set_halign (contactAvatar, GTK_ALIGN_START);
	
	GtkWidget* contactName = gtk_label_new(user->fullname);
	context = gtk_widget_get_style_context (contactName);
	gtk_style_context_add_class(context, "username");
	gtk_widget_set_visible (contactName, TRUE);
	gtk_widget_set_halign (contactName, GTK_ALIGN_CENTER);

	GtkWidget* contactLastMessage = gtk_label_new("");
	context = gtk_widget_get_style_context (contactLastMessage);
	gtk_style_context_add_class(context, "secondary");
	gtk_widget_set_visible (contactLastMessage, TRUE);
	gtk_widget_set_halign (contactLastMessage, GTK_ALIGN_CENTER);

	GtkWidget* contactLastSeen = gtk_label_new("Recently");
	context = gtk_widget_get_style_context (contactLastSeen);
	gtk_style_context_add_class(context, "secondary");
	gtk_widget_set_visible (contactLastSeen, TRUE);
	gtk_widget_set_halign (contactLastSeen, GTK_ALIGN_END);
	
	gtk_grid_attach(contactInfo, contactAvatar, 0, 0, 1, 2);
	gtk_grid_attach(contactInfo, contactName, 1, 0, 1, 1);
	gtk_grid_attach(contactInfo, contactLastMessage, 1, 1, 1, 1);
	gtk_grid_attach(contactInfo, contactLastSeen, 2, 1, 1, 1);
	
	return GTK_WIDGET(contactInfo);
}

void showUserContacts() {
	for (int i = 0; i < contacts->count; i += 1) {
		GtkWidget* contact = makeContact (contacts->list[i]);
		GtkWidget* contactMessages = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
		gtk_widget_set_visible (contactMessages, TRUE);
		gtk_stack_add_named (priv->messagesStack, contactMessages, 
		                     contacts->list[i]->id);
		gtk_list_box_insert(priv->chatsBox, contact, -1);
		gtk_widget_set_visible(contact, TRUE);
	}
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

on_login_btn_clicked(GtkButton *button) {
	initialize();
	gtk_stack_set_visible_child(priv->mainStack, priv->mainBox);
	user = login(userId);
	gtk_spinner_start (priv->connectionSpinner);
	contacts = getContacts (userId);
	showCurrentUserDetails();
	showUserContacts();
	collectAllMessages();
	gtk_spinner_stop (priv->connectionSpinner);
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
		struct Message* message = 
			(struct Message*)malloc(sizeof(struct Message));
		char* curUserId = (char*)calloc(strlen(userId), sizeof(char));
		strcpy(curUserId, userId);
		message->fromId = curUserId;
		message->toId = contacts->list[gtk_list_box_row_get_index(
			gtk_list_box_get_selected_row (priv->chatsBox))]->id;
		message->text = text;
		message->id = sendMessage (message);
		if (message -> id == NULL)
			messageDestructor (message);
		else
			putMessage(text, GTK_BOX(
				gtk_stack_get_child_by_name(priv->messagesStack, 
				                            message->toId)), TRUE);
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

chatRowSelected(GtkListBox* box, GtkListBoxRow *row, gpointer user_data) {
	if (row == NULL) {
		gtk_stack_set_visible_child (priv->messagesStack, 
		                             priv->messagesPlaceholder);
		gtk_revealer_set_reveal_child (priv->topboxRevealer, FALSE);
		gtk_revealer_set_reveal_child (priv->infoboxRevealer, FALSE);
		gtk_revealer_set_reveal_child (priv->messageRevealer, FALSE);
	}
	else {
		gint i = gtk_list_box_row_get_index(row);
		showUserDetails (contacts->list[i]);
		gtk_stack_set_visible_child_name(priv->messagesStack, 
		                                 contacts->list[i]->id);
	}
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