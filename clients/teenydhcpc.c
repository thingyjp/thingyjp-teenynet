#include <teenynet/dhcp4_client.h>

static void callback_lease_obtained(struct dhcp4_client_lease* lease) {
	g_message("obtained lease");
}

int main(int argc, char** argv) {

	const guint8 mac[] = { 0x08, 0x0, 0x27, 0xb9, 0x54, 0xa9 };

	Dhcp4Client* dhcp4client = dhcp4_client_new(3, mac);
	g_signal_connect(dhcp4client, DHCP4_CLIENT_DETAILEDSIGNAL_LEASE_OBTAINED,
			G_CALLBACK(callback_lease_obtained), NULL);

	dhcp4_client_start(dhcp4client);

	GMainLoop* mainloop = g_main_loop_new(NULL, FALSE);

	g_main_loop_run(mainloop);

	return 0;
}
