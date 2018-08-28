#include <gio/gunixsocketaddress.h>
#include "include/teenynet/unix.h"

GSocketService* unix_socketservice_create(const gchar* path,
		incomingcallback callback, gpointer user_data) {
	GSocketAddress* socketaddress = g_unix_socket_address_new(path);
	GSocketService* socketservice = g_socket_service_new();

	GError* err = NULL;
	if (!g_socket_listener_add_address((GSocketListener*) socketservice,
			socketaddress, G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_DEFAULT,
			NULL, NULL, &err)) {
		g_message("failed to add listener; %s", err->message);
		g_error_free(err);
		goto err_addlistener;
	}

	g_signal_connect(socketservice, "incoming", G_CALLBACK(callback),
			user_data);

	return socketservice;

	err_addlistener: //
	g_object_unref(socketservice);
	return NULL;
}

void unix_socketservice_destroy(GSocketService* service, const gchar* path) {
	g_socket_service_stop(service);
	g_object_unref(service);
	GFile* sockfile = g_file_new_for_path(path);
	g_file_delete(sockfile, NULL, NULL);
	g_object_unref(sockfile);
}
