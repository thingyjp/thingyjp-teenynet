#include "include/teenynet/dhcp4_client.h"
#include "include/teenynet/packetsocket.h"
#include "include/teenynet/ip4.h"
#include "dhcp4_model.h"

//TODO add proper lease renewal
//TODO add lease release

struct _Dhcp4Client {
	GObject parent_instance;
	enum dhcp4_clientstate state;
	unsigned ifidx;
	guint8 mac[ETHER_ADDR_LEN];

	GRand* rand;
	int rawsocket;
	guint rawsocketeventsource;

	guint32 xid;
	struct dhcp4_client_lease* pendinglease;
	struct dhcp4_client_lease* currentlease;

	gboolean paused;
};

G_DEFINE_TYPE(Dhcp4Client, dhcp4_client, G_TYPE_OBJECT)

static guint signal_lease;
static GQuark detail_lease_obtained;
static GQuark detail_lease_renewed;
static GQuark detail_lease_expired;

static void dhcp4_client_class_init(Dhcp4ClientClass *klass) {
	GSignalFlags flags = G_SIGNAL_RUN_LAST | G_SIGNAL_NO_HOOKS
			| G_SIGNAL_NO_RECURSE | G_SIGNAL_DETAILED;
	GType params[] = { G_TYPE_POINTER };
	signal_lease = g_signal_newv(DHCP4_CLIENT_SIGNAL_LEASE,
	DHCP4_TYPE_CLIENT, flags, NULL,
	NULL, NULL, NULL, G_TYPE_NONE, G_N_ELEMENTS(params), params);
	detail_lease_obtained = g_quark_from_string(
	DHCP4_CLIENT_DETAIL_LEASE_OBTAINED);
	detail_lease_renewed = g_quark_from_string(
	DHCP4_CLIENT_DETAIL_LEASE_RENEWED);
	detail_lease_expired = g_quark_from_string(
	DHCP4_CLIENT_DETAIL_LEASE_EXPIRED);
}

static void dhcp4_client_init(Dhcp4Client *self) {
	self->rand = g_rand_new();
	self->paused = TRUE;
}

Dhcp4Client* dhcp4_client_new(unsigned ifidx, const guint8* mac) {
	Dhcp4Client* client = g_object_new(DHCP4_TYPE_CLIENT, NULL);
	//these should be properties
	client->ifidx = ifidx;
	memcpy(client->mac, mac, sizeof(client->mac));
	return client;
}

static void dhcp4_client_changestate(Dhcp4Client* client,
		enum dhcp4_clientstate newstate);

static void dhcp4_client_send_discover(Dhcp4Client* client) {
	client->xid = g_rand_int(client->rand);
	struct dhcp4_pktcntx* pkt = dhcp4_model_pkt_new();
	dhcp4_model_fillheader(FALSE, pkt->header, client->xid, NULL, NULL,
			client->mac);
	dhcp4_model_pkt_set_dhcpmessagetype(pkt, DHCP4_DHCPMESSAGETYPE_DISCOVER);
	gsize pktsz;
	guint8* pktbytes = dhcp4_model_pkt_freetobytes(pkt, &pktsz);
	packetsocket_send_udp(client->rawsocket, client->ifidx, 0,
	DHCP4_PORT_CLIENT,
	DHCP4_PORT_SERVER, pktbytes, pktsz);
	g_free(pktbytes);
}

static gboolean dhcp4_client_discoverytimeout(gpointer data) {
	Dhcp4Client* client = data;

	if (client->paused)
		return TRUE;

	if (client->state != DHCP4CS_DISCOVERING)
		return FALSE;

	dhcp4_client_send_discover(client);
	return TRUE;
}

static void dhcp4_client_send_request(Dhcp4Client* client) {
	struct dhcp4_pktcntx* pkt = dhcp4_model_pkt_new();
	dhcp4_model_fillheader(FALSE, pkt->header, client->xid, NULL, NULL,
			client->mac);
	dhcp4_model_pkt_set_dhcpmessagetype(pkt, DHCP4_DHCPMESSAGETYPE_REQUEST);
	dhcp4_model_pkt_set_serverid(pkt, client->pendinglease->serverip);
	dhcp4_model_pkt_set_requestedip(pkt, client->pendinglease->leasedip);
	gsize pktsz;
	guint8* pktbytes = dhcp4_model_pkt_freetobytes(pkt, &pktsz);
	packetsocket_send_udp(client->rawsocket, client->ifidx, 0,
	DHCP4_PORT_CLIENT,
	DHCP4_PORT_SERVER, pktbytes, pktsz);
	g_free(pktbytes);
}

static gboolean dhcp4_client_requesttimeout(gpointer data) {
	Dhcp4Client* client = data;

	if (client->state != DHCP4CS_REQUESTING)
		return FALSE;

	g_message("timeout waiting for ack");
	dhcp4_client_changestate(client, DHCP4CS_DISCOVERING);
	return FALSE;
}

static gboolean dhcp4_client_leasetimeout(gpointer data) {
	Dhcp4Client* client = data;

	if (client->state != DHCP4CS_CONFIGURED)
		return FALSE;

	dhcp4_client_changestate(client, DHCP4CS_DISCOVERING);
	return FALSE;
}

static void dhcp4_client_processdhcppkt(Dhcp4Client* client,
		struct dhcp4_pktcntx* pktcntx) {
	if (pktcntx->header->xid == client->xid) {
		guint8 dhcpmessagetype = dhcp4_model_pkt_get_dhcpmessagetype(pktcntx);

#ifdef D4CDEBUG
		g_message("processing dhcp message type %d", (int ) dhcpmessagetype);
#endif

		switch (client->state) {
		case DHCP4CS_DISCOVERING:
			switch (dhcpmessagetype) {
			case DHCP4_DHCPMESSAGETYPE_OFFER: {
				struct dhcp4_client_lease* lease = g_malloc0(sizeof(*lease));
				dhcp4_model_pkt_get_serverid(pktcntx, lease->serverip);
				memcpy(lease->leasedip, pktcntx->header->yiaddr,
						sizeof(lease->leasedip));
				dhcp4_model_pkt_get_subnetmask(pktcntx, lease->subnetmask);
				dhcp4_model_pkt_get_defaultgw(pktcntx, lease->defaultgw);
				dhcp4_model_pkt_get_leasetime(pktcntx, &lease->leasetime);
				dhcp4_model_pkt_get_domainnameservers(pktcntx,
						(guint8*) lease->nameservers, &lease->numnameservers);
#ifdef D4CDEBUG
				g_message(
						"have dhcp offer of "IP4_ADDRFMT"/"IP4_ADDRFMT" for %u seconds from "IP4_ADDRFMT,
						IP4_ARGS(lease->leasedip), IP4_ARGS(lease->subnetmask),
						lease->leasetime, IP4_ARGS(lease->serverip));
#endif
				client->pendinglease = lease;
				dhcp4_client_changestate(client, DHCP4CS_REQUESTING);
				break;
			}
			default:
				break;
			}
			break;
		case DHCP4CS_REQUESTING:
			switch (dhcpmessagetype) {
			case DHCP4_DHCPMESSAGETYPE_ACK:
				dhcp4_client_changestate(client, DHCP4CS_CONFIGURED);
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	} else
		g_message("ignoring dhcp packet, different xid");
}

static gboolean dhcp4_client_rawsocketcallback(GIOChannel *source,
		GIOCondition condition, gpointer data) {
	Dhcp4Client* client = data;
	guint8* udppkt = g_malloc(1024);
	gssize udppktlen = packetsocket_recv_udp(client->rawsocket,
	DHCP4_PORT_SERVER,
	DHCP4_PORT_CLIENT, udppkt, 1024);

	if (udppktlen > 0) {
		struct dhcp4_pktcntx* pktcntx = dhcp4_model_pkt_parse(udppkt,
				udppktlen);
		if (pktcntx != NULL) {
			dhcp4_client_processdhcppkt(client, pktcntx);
			g_free(pktcntx);
		}
	}
	g_free(udppkt);
	return TRUE;
}

static void dhcp4_client_changestate(Dhcp4Client* client,
		enum dhcp4_clientstate newstate) {
#ifdef D4CDEBUG
	g_message("moving from state %d to %d", client->state, newstate);
#endif

	client->state = newstate;
	switch (client->state) {
	case DHCP4CS_DISCOVERING:
		client->rawsocket = packetsocket_createsocket_udp(client->ifidx,
				client->mac);
		g_assert(client->rawsocket);
		GIOChannel* channel = g_io_channel_unix_new(client->rawsocket);
		client->rawsocketeventsource = g_io_add_watch(channel, G_IO_IN,
				dhcp4_client_rawsocketcallback, client);
		g_io_channel_unref(channel);
		dhcp4_client_send_discover(client);
		g_timeout_add(10 * 1000, dhcp4_client_discoverytimeout, client);
		break;
	case DHCP4CS_REQUESTING:
		dhcp4_client_send_request(client);
		g_timeout_add(10 * 1000, dhcp4_client_requesttimeout, client);
		break;
	case DHCP4CS_CONFIGURED:
		g_source_remove(client->rawsocketeventsource);
		client->rawsocketeventsource = -1;
		close(client->rawsocket);
		client->rawsocket = -1;
		client->currentlease = client->pendinglease;
		client->pendinglease = NULL;
		g_signal_emit(client, signal_lease, detail_lease_obtained,
				client->currentlease);
		g_timeout_add(client->currentlease->leasetime * 1000,
				dhcp4_client_leasetimeout, client);
		break;
	default:
		break;
	}
}

void dhcp4_client_start(Dhcp4Client* client) {
	dhcp4_client_changestate(client, DHCP4CS_DISCOVERING);
}

void dhcp4_client_pause(Dhcp4Client* client) {
	client->paused = TRUE;
}

void dhcp4_client_resume(Dhcp4Client* client) {
	client->paused = FALSE;
}

unsigned dhcp4_client_getifindx(Dhcp4Client* client) {
	return client->ifidx;
}

enum dhcp4_clientstate dhcp4_client_getstate(Dhcp4Client* client) {
	return client->state;
}
struct dhcp4_client_lease* dhcp4_client_getlease(Dhcp4Client* client) {
	return client->currentlease;
}

void dhcp4_client_stop(Dhcp4Client* client) {

}
