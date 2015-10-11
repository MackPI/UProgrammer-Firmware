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
//#include "at_custom.h"
#include "user_interface.h"
#include "wifi.h"

os_timer_t ptimer;
struct station_config wifi_config;
char connection_status[64] = "Disconnected";
int system_state;

void user_state(void)
{
	switch(system_state){
	case 0: // initializing
		system_state = 1;
		break;
	case 1: //Display Menu
		display_config_menu();
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
	system_state = 0;
	serial_init();
	os_timer_disarm(&ptimer);
	os_timer_setfn(&ptimer,user_state,NULL);
	os_timer_arm(&ptimer,1000,1);
	wifi_set_event_handler_cb(wifi_handle_event_cb);
}
