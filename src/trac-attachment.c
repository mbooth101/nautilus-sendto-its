/*
 * Copyright (C) 2010 Mat Booth
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more av.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http://www.gnu.org/licenses/>
 *
 * Author:  Mat Booth <mbooth@fedoraproject.org>
 *
 */

#include <string.h>
#include <stdlib.h>

#include <nautilus-sendto-plugin.h>
#include <libsoup/soup.h>
#include <gnome-keyring.h>

static const GnomeKeyringPasswordSchema http_auth_password_schema =
{
	GNOME_KEYRING_ITEM_GENERIC_SECRET,
	{
		{ "host",  GNOME_KEYRING_ATTRIBUTE_TYPE_STRING },
		{ "realm", GNOME_KEYRING_ATTRIBUTE_TYPE_STRING },
		{ "user",  GNOME_KEYRING_ATTRIBUTE_TYPE_STRING },
		{ NULL, 0 }
	}
};
const GnomeKeyringPasswordSchema *HTTP_AUTH_SCHEMA = &http_auth_password_schema;

static SoupSession *session = NULL;

static GtkWidget *url_field = NULL;
static GtkWidget *ticket_field = NULL;

static void handle_error(GError *error, const char* message)
{
	if (error)
	{
		g_print ("[trac-attachment] %s - %d %s\n", message, error->code, error->message);
		g_error_free (error);
	}
	else
	{
		g_print ("[trac-attachment] %s\n", message);
	}
}

static gchar* send_xmlrpc (const gchar *uri, const gchar *method, GValue *result, ...)
{
	// Create an XMLRPC request from the arguments
	va_list args;
	va_start (args, result);
	GValueArray *params = soup_value_array_from_args (args);
	va_end (args);
	char *body = soup_xmlrpc_build_method_call (method, params->values, params->n_values);
	g_value_array_free (params);

	if (!body)
		return g_strdup("Could not create XMLRPC method call");

	// Create and send the actual request message
	SoupMessage *msg = soup_message_new ("POST", uri);
	soup_message_set_request (msg, "text/xml", SOUP_MEMORY_TAKE, body, strlen(body));
	soup_session_send_message (session, msg);

	if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
	{
		gchar *retval = g_strdup_printf("Request failed: %s", msg->reason_phrase);
		g_object_unref (msg);
		return retval;
	}

	// Parse the XMLRPC response
	GError *err = NULL;
	if (!soup_xmlrpc_parse_method_response (msg->response_body->data, msg->response_body->length, result, &err))
	{
		gchar *retval = NULL;
		if (err)
		{
			retval = g_strdup_printf("Error: %s", err->message);
			g_error_free (err);
		}
		else
		{
			retval = g_strdup("Could not parse XMLRPC response");
		}
		g_object_unref (msg);
		return retval;
	}
	g_object_unref (msg);

	return NULL;
}

static GtkWidget* get_credentials_dialog (const gchar* default_username, GtkWidget **user_field, GtkWidget **pass_field)
{
	GtkWidget *dialog = gtk_dialog_new_with_buttons("Authentication Required", NULL, GTK_DIALOG_MODAL,
			GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
			NULL);
	gtk_window_set_icon_name(GTK_WINDOW(dialog), "document-send");

	GtkWidget *credentials_widget = gtk_table_new(2, 2, FALSE);
	GtkWidget *user_label = gtk_label_new("Username:");
	*user_field = gtk_entry_new();
	GtkWidget *pass_label = gtk_label_new("Password:");
	*pass_field = gtk_entry_new();

	gtk_table_attach_defaults (GTK_TABLE (credentials_widget), user_label, 0, 1, 0, 1);
	gtk_widget_show (user_label);

	gtk_table_attach_defaults (GTK_TABLE (credentials_widget), *user_field, 1, 2, 0, 1);
	gtk_widget_show (*user_field);
	gtk_entry_set_text(GTK_ENTRY(*user_field), default_username);

	gtk_table_attach_defaults (GTK_TABLE (credentials_widget), pass_label, 0, 1, 1, 2);
	gtk_widget_show (pass_label);

	gtk_table_attach_defaults (GTK_TABLE (credentials_widget), *pass_field, 1, 2, 1, 2);
	gtk_widget_show (*pass_field);
	gtk_entry_set_visibility(GTK_ENTRY(*pass_field), FALSE);
	gtk_widget_grab_focus(*pass_field);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), credentials_widget, TRUE, TRUE, 0);
	gtk_widget_show (credentials_widget);

	return dialog;
}

static void session_authenticate (SoupSession *session, SoupMessage *msg, SoupAuth *auth, gboolean retrying, gpointer user_data)
{
	GnomeKeyringResult result;
	gchar *user = NULL;
	gchar *password = NULL;

	// Get the previous user name that was used for this uri
	// TODO Gconf stuff

	// There wasn't one, so let's use the logged in user's name
	if (!user)
		user = g_strdup(getenv("USER"));

	if (retrying)
	{
		// Prompt the user if there were no valid credentials in the keyring
		GtkWidget *user_field = NULL;
		GtkWidget *pass_field = NULL;
		GtkWidget *dialog = get_credentials_dialog(user, &user_field, &pass_field);
		gint response = gtk_dialog_run(GTK_DIALOG (dialog));
		if (response == GTK_RESPONSE_REJECT)
		{
			gtk_widget_destroy(dialog);
			return;
		}
		g_free(user);
		user = g_strdup(gtk_entry_get_text(GTK_ENTRY(user_field)));
		password = g_strdup(gtk_entry_get_text(GTK_ENTRY(pass_field)));
		gtk_widget_destroy(dialog);

		// Store password in the keyring
		gchar * description = g_strdup_printf("Nautilus-Sendto-Trac:%s:%s", soup_auth_get_host(auth), soup_auth_get_realm(auth));
		result = gnome_keyring_store_password_sync(HTTP_AUTH_SCHEMA, NULL, description, password,
				"host", soup_auth_get_host(auth),
				"realm", soup_auth_get_realm(auth),
				"user", user,
				NULL);
		g_free(description);
		g_free(password);
	}

	// Try and get a password for the user from the keyring
	result = gnome_keyring_find_password_sync(HTTP_AUTH_SCHEMA, &password,
			"host", soup_auth_get_host(auth),
			"realm", soup_auth_get_realm(auth),
			"user", user,
			NULL);
	if (result != GNOME_KEYRING_RESULT_OK)
		password = "";

	// Do the authentication
	soup_auth_authenticate(auth, user, password);
	g_free(user);
	if (result == GNOME_KEYRING_RESULT_OK)
		gnome_keyring_free_password(password);
	return;
}

static void ticket_insert_text (GtkEditable *editable, gchar *new_text, gint new_text_length, gint *position, gpointer user_data)
{
	int i, count = 0;
	gchar *result = g_new (gchar, new_text_length);

	// Allow digits only
	for (i = 0; i < new_text_length; i++)
	{
		if (!g_ascii_isdigit (new_text[i]))
			continue;
		result[count++] =  new_text[i];
	}

	// Insert the text without re-emitting the signal
	if (count > 0)
	{
		g_signal_handlers_block_by_func (G_OBJECT (editable), G_CALLBACK (ticket_insert_text), user_data);
		gtk_editable_insert_text (editable, result, count, position);
		g_signal_handlers_unblock_by_func (G_OBJECT (editable), G_CALLBACK (ticket_insert_text), user_data);
	}

	// Don't let the default signal handler run
	g_signal_stop_emission_by_name (G_OBJECT (editable), "insert-text");

	g_free(result);
}

static gboolean init (NstPlugin *plugin)
{
	g_print ("Init trac-attachment plugin\n");

	session = soup_session_sync_new ();
	g_signal_connect (G_OBJECT (session), "authenticate", G_CALLBACK (session_authenticate), NULL);

	url_field = gtk_entry_new ();
	ticket_field = gtk_entry_new ();

	return TRUE;
}

static GtkWidget* get_contacts_widget (NstPlugin *plugin)
{
	GtkWidget *contact_widget = gtk_table_new (2, 2, FALSE);
	GtkWidget *url_label = gtk_label_new ("Trac URI:");
	GtkWidget *ticket_label = gtk_label_new ("Ticket #:");

	gtk_table_attach_defaults (GTK_TABLE (contact_widget), url_label, 0, 1, 0, 1);
	gtk_widget_show (url_label);

	gtk_table_attach_defaults (GTK_TABLE (contact_widget), url_field, 1, 2, 0, 1);
	gtk_widget_show (url_field);
	gtk_entry_set_text(GTK_ENTRY(url_field), "http://sources/trac/xmlrpc");
	// TODO get last uri from Gconf

	gtk_table_attach_defaults (GTK_TABLE (contact_widget), ticket_label, 0, 1, 1, 2);
	gtk_widget_show (ticket_label);

	gtk_table_attach_defaults (GTK_TABLE (contact_widget), ticket_field, 1, 2, 1, 2);
	gtk_widget_show (ticket_field);
	g_signal_connect (G_OBJECT (ticket_field), "insert-text", G_CALLBACK (ticket_insert_text), NULL);

	return contact_widget;
}

static gboolean validate_destination (NstPlugin *plugin, GtkWidget *contact_widget, char **error)
{
	// Validate ticket number the user has entered
	if (gtk_entry_get_text_length (GTK_ENTRY(ticket_field)) <= 0)
	{
		*error = g_strdup("No ticket number specified");
		return FALSE;
	}

	// Validate the URI the user has entered
	SoupURI *soup_uri = soup_uri_new(gtk_entry_get_text(GTK_ENTRY(url_field)));
	if (!soup_uri)
	{
		*error = g_strdup("URI is invalid");
		return FALSE;
	}
	gchar *uri = soup_uri_to_string (soup_uri, FALSE);
	soup_uri_free(soup_uri);

	// Try an XMLRPC request to see if the URI exists, we can authenticate, the ticket exists, etc
	// We don't really care about the result, as long as it succeeds
	GValue result;
	gchar *err = send_xmlrpc(uri, "ticket.get", &result,
			G_TYPE_INT, strtol(gtk_entry_get_text(GTK_ENTRY(ticket_field)), NULL, 10),
			G_TYPE_INVALID);

	g_free(uri);

	if (err)
	{
		*error = err;
		return FALSE;
	}

	return TRUE;
}

static gboolean send_files (NstPlugin *plugin, GtkWidget *contact_widget, GList *file_list)
{
	GList *l;
	for (l = file_list; l != NULL; l = l->next)
	{
		// Open file
		GError *error = NULL;
		GFile *source = g_file_new_for_commandline_arg (l->data);
		GFileInputStream *in = g_file_read (source, NULL, &error);
		if (!in)
		{
			handle_error(error, "could not get input stream for file");
			g_object_unref (source);
			return FALSE;
		}

		// Read file contents
		GByteArray *data = g_byte_array_new ();
		guint8 buffer[1024];
		gssize len = g_input_stream_read (G_INPUT_STREAM(in), buffer, 1024, NULL, &error);
		while (len > 0)
		{
			g_byte_array_append (data, buffer, len);
			len = g_input_stream_read (G_INPUT_STREAM(in), buffer, 1024, NULL, &error);
		}
		if (len < 0)
		{
			handle_error(error, "could not read file");
			g_byte_array_free (data, TRUE);
			g_object_unref (in);
			g_object_unref (source);
			return FALSE;
		}

		// XMLRPC request to attach the file to the ticket
		GValue result;
		gchar *err = send_xmlrpc(gtk_entry_get_text(GTK_ENTRY(url_field)), "ticket.putAttachment", &result,
				G_TYPE_INT, strtol(gtk_entry_get_text(GTK_ENTRY(ticket_field)), NULL, 10),
				G_TYPE_STRING, g_file_get_basename(source),
				G_TYPE_STRING, "",
				SOUP_TYPE_BYTE_ARRAY, data,
				G_TYPE_BOOLEAN, TRUE,
				G_TYPE_INVALID);

		g_byte_array_free (data, TRUE);
		g_object_unref (in);

		if (err)
		{
			handle_error(NULL, err);
			g_free(err);
			g_object_unref (source);
			return FALSE;
		}
		g_object_unref (source);
	}

	return TRUE;
}

static gboolean destroy (NstPlugin *plugin)
{
	gtk_widget_destroy (url_field);
	gtk_widget_destroy (ticket_field);

	soup_session_abort (session);
	g_object_unref (session);
	return TRUE;
}

static NstPluginInfo plugin_info =
{
	"stock_internet",
	"trac-attachment",
	"Trac Attachment",
	NULL,
	FALSE,
	NAUTILUS_CAPS_NONE,
	init,
	get_contacts_widget,
	validate_destination,
	send_files,
	destroy
};

NST_INIT_PLUGIN (plugin_info)
