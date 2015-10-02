/******************************************************************************
 * MIT open source license (Steve McNeil = Mack@PrototypeIteration.com)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2015/10/1, v1.0 create this file.
*******************************************************************************/

#include "osapi.h"
#include "at_custom.h"
#include "user_interface.h"
#include "wifi.h"

os_timer_t ptimer;

void user_state(void)
{
	static int system_state = 0;
	switch(system_state){
	case 0: // initializing
		system_state = 1;
		break;
	case 1: //Hello World
//		char buffer[64] = {0};
	    uart0_sendStr("\n\rHello World \n\r");
		system_state = 2;
		break;
	default: // done for now
		break;
	}
}

void user_rf_pre_init(void)
{
}

void user_init(void)
{
	struct station_config wifi_config;
    char ssid[32] = ""; // Access point name
	char password[64] = ""; // Access point password
//	os_install_putc1((void *)uart1_write_char);
	os_memcpy(&wifi_config.ssid, ssid,32);
	os_memcpy(&wifi_config.password, password,64);
	wifi_config.bssid_set = 0;
	uart_init(115200,115200);
	system_set_os_print(1);
	os_timer_disarm(&ptimer);
	os_timer_setfn(&ptimer,user_state,NULL);
	os_timer_arm(&ptimer,1000,1);
	wifi_set_opmode(STATION_MODE);
//	wifi_station_set_config(&wifi_config); // uncomment and add ssid/password to set up connection
	wifi_set_event_handler_cb(wifi_handle_event_cb);
//	at_register_uart_rx_intr();
}
