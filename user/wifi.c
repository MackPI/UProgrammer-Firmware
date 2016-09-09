/******************************************************************************
 * MIT open source license (Steve McNeil = Mack@PrototypeIteration.com)
 *
 * FileName: wifi.c
 *
 * Description: Basic custom Wifi functionality
 *
 * Modification history:
 *     2015/10/1, v1.0 create this file.
 *******************************************************************************/

#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "espconn.h"
#include "wifi.h"
//#include "uart.h"
//#include "ets_sys.h"
extern struct station_config wifi_config;
extern char connection_status[64];
void wifiInit()
{
	wifi_set_event_handler_cb(wifiHandleEventCB);
	wifi_set_opmode(STATION_MODE);
}

void wifiHandleEventCB(System_Event_t *evt)
{
	char txbuffer[127];
	os_sprintf(txbuffer, "event %x\r\n", evt->event);
	uart0SendStr(txbuffer);
	switch (evt->event)
	{
	case EVENT_STAMODE_CONNECTED:
		os_sprintf(txbuffer, "connect to ssid %s, channel %d\r\n",
				evt->event_info.connected.ssid,
				evt->event_info.connected.channel);
		uart0SendStr(txbuffer);
		os_memcpy(&wifi_config.ssid, evt->event_info.connected.ssid, 32);
		break;
	case EVENT_STAMODE_DISCONNECTED:
		os_sprintf(txbuffer, "disconnect from ssid %s, reason %d\r\n",
				evt->event_info.disconnected.ssid,
				evt->event_info.disconnected.reason);
		uart0SendStr(txbuffer);
		os_sprintf(connection_status, "Disconnected");
		break;
	case EVENT_STAMODE_AUTHMODE_CHANGE:
		os_sprintf(txbuffer, "mode: %d -> %d\r\n",
				evt->event_info.auth_change.old_mode,
				evt->event_info.auth_change.new_mode);
		uart0SendStr(txbuffer);
		break;
	case EVENT_STAMODE_GOT_IP:
		os_sprintf(txbuffer,
				"ip:" IPSTR "\n\rmask:" IPSTR "\n\rgw:" IPSTR "\n\r",
				IP2STR(&evt->event_info.got_ip.ip),
				IP2STR(&evt->event_info.got_ip.mask),
				IP2STR(&evt->event_info.got_ip.gw));
		uart0SendStr(txbuffer);
		os_sprintf(connection_status, "Connected IP = " IPSTR "\n\r",
				IP2STR(&evt->event_info.got_ip.ip));
//		uart0_sendStr("\n");
		break;
	case EVENT_SOFTAPMODE_STACONNECTED:
		os_sprintf(txbuffer, "station: " MACSTR "join, AID = %d\r\n",
				MAC2STR(evt->event_info.sta_connected.mac),
				evt->event_info.sta_connected.aid);
		uart0SendStr(txbuffer);
		break;
	case EVENT_SOFTAPMODE_STADISCONNECTED:
		os_sprintf(txbuffer, "station: " MACSTR "leave, AID = %d\r\n",
				MAC2STR(evt->event_info.sta_disconnected.mac),
				evt->event_info.sta_disconnected.aid);
		uart0SendStr(txbuffer);
		break;
	default:
		break;
	}
}
