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
unsigned dhcp4_client_getifindx(Dhcp4Client* client);
enum dhcp4_clientstate dhcp4_client_getstate(Dhcp4Client* client);
struct dhcp4_client_lease* dhcp4_client_getlease(Dhcp4Client* client);
void dhcp4_client_stop(Dhcp4Client* client);

#define DHCP4_CLIENT_SIGNAL_LEASE           "lease"
#define DHCP4_CLIENT_DETAIL_LEASE_OBTAINED  "obtained"
#define DHCP4_CLIENT_DETAIL_LEASE_RENEWED   "renewed"
#define DHCP4_CLIENT_DETAIL_LEASE_EXPIRED   "expired"

#define DHCP4_CLIENT_DETAILEDSIGNAL_LEASE_OBTAINED DHCP4_CLIENT_SIGNAL_LEASE"::"DHCP4_CLIENT_DETAIL_LEASE_OBTAINED
#define DHCP4_CLIENT_DETAILEDSIGNAL_LEASE_RENEWED  DHCP4_CLIENT_SIGNAL_LEASE"::"DHCP4_CLIENT_DETAIL_LEASE_RENEWED
#define DHCP4_CLIENT_DETAILEDSIGNAL_LEASE_EXPIRED  DHCP4_CLIENT_SIGNAL_LEASE"::"DHCP4_CLIENT_DETAIL_LEASE_EXPIRED

typedef void (*dhcp4_client_leasecb)(Dhcp4Client* client,
		struct dhcp4_client_lease* lease, gpointer user_data);

G_END_DECLS

