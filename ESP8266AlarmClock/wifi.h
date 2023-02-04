// wifi.h

#ifndef HEADER_WIFI
#define HEADER_WIFI

namespace wifi {

	void setup (void);

	//void loop (void);

	String wifi_ssid (void);
	String wifi_ip4 (void);
	String wifi_status (void);
	int wifi_status_code ();
	void stop_ap_mode ();
	void start_sta_mode ();
	void newSSIDs ();

} // namespace

#endif

// vim: set ts=4 sw=4 tw=0 noet :
