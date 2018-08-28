#include <gio/gunixsocketaddress.h>
#include "include/teenynet/unix.h"

GSocketService* unix_createsocketlistener(const gchar* path,
		incomingcallback callback, gpointer user_data) {
	GSocketAddress* socketaddress = g_unix_socket_address_new(path);
	GSocketService* socketservice = g_socket_service_new();

	GError* err = NULL;
	if (!g_socket_listener_add_address((GSocketListener*) socketservice,
			socketaddress, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT,
			NULL, NULL, &err)) {
		g_message("failed to add listener; %s", err->message);
	}
	g_error_free(err);

	g_signal_connect(socketservice, "incoming", G_CALLBACK(callback),
			user_data);

	return socketservice;
}
