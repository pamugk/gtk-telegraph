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
#include "client.h"
#include <config.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "gtk_telegraph.h"

#pragma region App GUI definition
/* For testing purpose, define TEST to use the local (not installed) ui file */
#define TEST
#ifdef TEST
#define UI_FILE "src/gtk_telegraph.ui"
#else
#define UI_FILE PACKAGE_DATA_DIR"/ui/gtk_telegraph.ui"
#endif
#define TOP_WINDOW "window"

typedef struct _Private Private;
struct _Private
{
	GdkScreen* screen;
	GtkCssProvider* darkProvider;
	GtkCssProvider* lightProvider;
	
	GtkStack* mainStack;
	
	GtkLabel* loginStatusLbl;

	GtkStack* drawerStack;
	GtkSpinner* connectionSpinner;
	GtkListBox* chatsBox;
	GtkLabel* chatsBoxPlaceholder;
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

static Private* priv = NULL;
#pragma endregion
#pragma region Chat client GUI actions
void putMessage(const gchar* text, GtkBox* messagesBox, gboolean local) {
	GtkWidget* messageLabel = gtk_label_new(text);
	GtkStyleContext* context = gtk_widget_get_style_context(messageLabel);
	gtk_style_context_add_class(context, "message");
	gtk_box_pack_start(messagesBox, messageLabel, FALSE, FALSE, 0);
	gtk_widget_set_halign(messageLabel, local ? GTK_ALIGN_END : GTK_ALIGN_START);
	gtk_widget_set_margin_end(messageLabel, 20);
	gtk_widget_set_visible (messageLabel, TRUE);
}

void setLastMessageToContactView(GtkGrid* contactView, struct Message* message) {
	GtkLabel* lastMessageLabel = GTK_LABEL(gtk_grid_get_child_at(contactView, 1, 1));
	gtk_label_set_text (lastMessageLabel, message->text);
}

void acceptNewMessage(struct Message* message){
	for (int i = 0; i < contacts->count; i += 1) {
		if (strcmp(contacts->list[i]->id, message->fromId) == 0) {
			setLastMessageToContactView(GTK_GRID(gtk_bin_get_child(
				           gtk_list_box_get_row_at_index (priv->chatsBox, i))),
			                            message);
			break;
		}
	}
	putMessage (message->text, 
		GTK_BOX(gtk_stack_get_child_by_name (priv->messagesStack, message->fromId)),
	    FALSE);
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
		if (messages->count > 0)
			setLastMessageToContactView(GTK_GRID(gtk_bin_get_child(
				           gtk_list_box_get_row_at_index (priv->chatsBox, i))),
			                            messages->list[messages->count - 1]);
	}
}

void onServerShutdown() {
	printf("Received a message about server shutdown\n");
	gtk_stack_set_visible_child_name (priv->mainStack, "disconnectionMessage");
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
#pragma endregion
#pragma region Signal handlers
add_to_contacts_btn_clicked(GtkButton *button){
	printf("Contact added\n");
}

block_user_btn_clicked (GtkButton *button, gpointer user_data){
	printf("User blocked\n");
}

bookmark_btn_clicked (GtkButton *button){
	gtk_stack_set_visible_child_name(priv->drawerStack, "chatsDrawer");
}

calls_btn_clicked(GtkButton *button){
	gtk_widget_show(priv->callsDialog);
}

calls_close_btn_clicked(GtkButton *button){
	gtk_widget_hide(priv->callsDialog);
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

void removeChild(GtkWidget* widget, gpointer container) {
	gtk_container_remove (GTK_CONTAINER(container), widget);
}

clear_history_btn_clicked (GtkButton *button, gpointer user_data){
	GtkListBoxRow* selectedChat = gtk_list_box_get_selected_row (priv->chatsBox);
	char* userId = contacts->list[gtk_list_box_row_get_index(selectedChat)]->id;
	if (selectedChat != NULL) {
		int outcome = clearHistory (userId);
		if (outcome == 0) {
			GtkBox* messages = GTK_BOX(gtk_stack_get_child_by_name (
			                                   priv->messagesStack, userId));
			gtk_container_foreach (messages, removeChild, messages);
		}
	}
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
	gtk_stack_set_visible_child_name(priv->drawerStack, "settingsDrawer");
}

infopanel_btn_clicked (GtkButton *button){
	gboolean childRevealed = gtk_revealer_get_reveal_child (priv->infoboxRevealer);
	gtk_revealer_set_reveal_child (priv->infoboxRevealer, !childRevealed);
}

login_entry_activate(GtkEntry *messageEntry, gpointer user_data) {
	const gchar *details = gtk_entry_get_text(messageEntry);
	if (details == NULL || strcmp("", details) == 0) {
		gtk_label_set_text (priv->loginStatusLbl, "Details can not be empty");
		return 0;
	}
	if (initialize() != 0) {
		gtk_label_set_text (priv->loginStatusLbl, "Server does not respond");
		return 0;
	}
	user = login(details);
	if (user == NULL) {
		gtk_label_set_text (priv->loginStatusLbl, "Login has failed");
		return 0;
	}
	gtk_stack_set_visible_child_name(priv->mainStack, "mainPage");
	gtk_spinner_start (priv->connectionSpinner);
	contacts = getContacts ();
	showCurrentUserDetails();
	showUserContacts();
	collectAllMessages();
	gtk_spinner_stop (priv->connectionSpinner);
}

message_entry_activate(GtkEntry *messageEntry, gpointer user_data){
	gchar *text = gtk_entry_get_text(messageEntry);
	if (text == NULL || strcmp(text, "") == 0)
		return TRUE;
	struct Message* message = 
			(struct Message*)malloc(sizeof(struct Message));
	char* curUserId = (char*)calloc(strlen(userId), sizeof(char));
	strcpy(curUserId, userId);
	message->fromId = curUserId;
	GtkBin* contactRow = gtk_list_box_get_selected_row (priv->chatsBox);
	message->toId = contacts->list[gtk_list_box_row_get_index(contactRow)]->id;
	message->text = text;
	message->id = sendMessage (message);
	if (message -> id == NULL)
		messageDestructor (message);
	else {
			GtkGrid* grid = GTK_GRID(gtk_bin_get_child(contactRow));
			printf("%s\n", message->text);
			setLastMessageToContactView(grid, message);
			putMessage(text, GTK_BOX(
				gtk_stack_get_child_by_name(priv->messagesStack, 
						                       message->toId)), TRUE);
			gtk_entry_set_text (messageEntry, "");
	}	
	return TRUE;
}

nm_switch_activated(GtkSwitch* widget, gboolean state, gpointer user_data){
	if (state) {
		gtk_style_context_remove_provider_for_screen(priv->screen, 
		                                             priv->lightProvider);
		gtk_style_context_add_provider_for_screen (priv->screen,
		                                GTK_STYLE_PROVIDER(priv->darkProvider),
		                                GTK_STYLE_PROVIDER_PRIORITY_USER);
	}
	else {
		gtk_style_context_remove_provider_for_screen(priv->screen, priv->darkProvider);
		gtk_style_context_add_provider_for_screen (priv->screen,
		                                GTK_STYLE_PROVIDER(priv->lightProvider),
		                                GTK_STYLE_PROVIDER_PRIORITY_USER);
	}
	gtk_switch_set_state (widget, state);
	return TRUE;
}

on_message_window_delete(GtkWidget *widget, GdkEvent  *event, gpointer user_data) {
	gtk_main_quit ();
}

on_modal_window_delete(GtkWidget *widget, GdkEvent  *event, gpointer user_data){
	gtk_widget_hide(widget);
	return TRUE;
}

on_window_destroy(GtkWindow *window){
	cleanup();
	gtk_main_quit ();
}

settings_btn_clicked(GtkButton *button){
	gtk_widget_show(priv->settingsDialog);
}
#pragma endregion
#pragma region App initialization
static GtkWidget*
create_window (void)
{
	GtkWidget *window;
	GtkBuilder *builder;
	GError* error = NULL;

	/* Load UI from file */
	builder = gtk_builder_new ();
	if (!gtk_builder_add_from_file (builder, UI_FILE, &error))
	{
		g_critical ("Couldn't load builder file: %s", error->message);
		g_error_free (error);
	}

	/* Auto-connect signal handlers */
	gtk_builder_connect_signals (builder, NULL);

	/* Get the window object from the ui file */
	window = GTK_WIDGET (gtk_builder_get_object (builder, TOP_WINDOW));
        if (!window)
        {
                g_critical ("Widget \"%s\" is missing in file %s.",
				TOP_WINDOW,
				UI_FILE);
        }

	priv = g_malloc (sizeof (struct _Private));
	
	GtkStyleContext* context = gtk_widget_get_style_context (window);
	priv->screen = gtk_style_context_get_screen(context);
	priv->darkProvider = gtk_css_provider_new ();
	gtk_css_provider_load_from_path (priv->darkProvider, "css/main_dark.css", NULL);
	priv->lightProvider = gtk_css_provider_new ();
	gtk_css_provider_load_from_path (priv->lightProvider, "css/main_light.css", NULL);
	GtkCssProvider* provider = gtk_css_provider_new ();
	gtk_css_provider_load_from_path (provider, "css/main.css", NULL);
	gtk_style_context_add_provider_for_screen (priv->screen,
                                    GTK_STYLE_PROVIDER(provider),
                                    GTK_STYLE_PROVIDER_PRIORITY_USER);
	gtk_style_context_add_provider_for_screen (priv->screen,
                                    GTK_STYLE_PROVIDER(priv->lightProvider),
                                    GTK_STYLE_PROVIDER_PRIORITY_USER);
	
	priv->mainStack =
		GTK_STACK (gtk_builder_get_object(builder, "mainStack"));

	priv->loginStatusLbl = 
		GTK_LABEL (gtk_builder_get_object(builder, "loginStatusLbl"));
	
	priv->drawerStack =
		GTK_STACK (gtk_builder_get_object(builder, "drawerStack"));
	priv->connectionSpinner =
		GTK_SPINNER (gtk_builder_get_object(builder, "connectionSpinner"));
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
	gtk_widget_set_visible (priv->chatsBoxPlaceholder, TRUE);
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
	return window;
}

void start_telegraph(int argc, char *argv[]){
 	GtkWidget *window;

#ifdef ENABLE_NLS

	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif
	
	gtk_init (&argc, &argv);

	window = create_window ();
	gtk_widget_show (window);
	gtk_main ();
	g_free (priv);
}
#pragma endregion