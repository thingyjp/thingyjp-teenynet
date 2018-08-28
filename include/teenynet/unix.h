#ifndef INCLUDE_TEENYNET_UNIX_H_
#define INCLUDE_TEENYNET_UNIX_H_

#include <gio/gio.h>

typedef gboolean (*incomingcallback)(GSocketService *service,
		GSocketConnection *connection, GObject *source_object,
		gpointer user_data);

GSocketService* unix_createsocketlistener(const gchar* path,
		incomingcallback callback, gpointer user_data);

#endif /* INCLUDE_TEENYNET_UNIX_H_ */
