#pragma once

#include <net/ethernet.h>
#include <glib.h>
#include <gio/gio.h>
#include <gobject/gobject.h>
#include "dhcp4.h"

G_BEGIN_DECLS

#define DHCP4_TYPE_CLIENT dhcp4_client_get_type ()
G_DECLARE_FINAL_TYPE(Dhcp4Client, dhcp4_client, DHCP4, CLIENT, GObject)

#define DHCP4_CLIENT_MAXNAMESERVERS	4

enum dhcp4_clientstate {
	DHCP4CS_IDLE, DHCP4CS_DISCOVERING, DHCP4CS_REQUESTING, DHCP4CS_CONFIGURED
};

struct dhcp4_client_lease {
	guint8 serverip[DHCP4_ADDRESS_LEN];
	guint8 leasedip[DHCP4_ADDRESS_LEN];
	guint8 subnetmask[DHCP4_ADDRESS_LEN];
	guint8 defaultgw[DHCP4_ADDRESS_LEN];
	guint8 nameservers[DHCP4_CLIENT_MAXNAMESERVERS][DHCP4_ADDRESS_LEN];
	guint8 numnameservers;
	guint32 leasetime;
};

Dhcp4Client* dhcp4_client_new(unsigned ifidx, const guint8* mac);
void dhcp4_client_start(Dhcp4Client* client);
void dhcp4_client_pause(Dhcp4Client* client);
void dhcp4_client_resume(Dhcp4Client* client);
void dhcp4_client_stop(Dhcp4Client* client);

#define DHCP4_CLIENT_SIGNAL_INTERFACE           "interface"
#define DHCP4_CLIENT_DETAIL_INTERFACE_CLEAR     "clear"
#define DHCP4_CLIENT_DETAIL_INTERFACE_CONFIGURE "configure"

#define DHCP4_CLIENT_DETAILEDSIGNAL_INTERFACE_CLEAR     DHCP4_CLIENT_SIGNAL_INTERFACE"::"DHCP4_CLIENT_DETAIL_INTERFACE_CLEAR
#define DHCP4_CLIENT_DETAILEDSIGNAL_INTERFACE_CONFIGURE DHCP4_CLIENT_SIGNAL_INTERFACE"::"DHCP4_CLIENT_DETAIL_INTERFACE_CONFIGURE

G_END_DECLS

