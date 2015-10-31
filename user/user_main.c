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
#include "driver/spi_overlap.h"
#include "gpio.h"
#include "pwm.h"
#include "eagle_soc.h"
#include "driver/sigma_delta.h"

uint32 duty;
os_timer_t ptimer;
struct station_config wifi_config;
char connection_status[64] = "Disconnected";
int system_state;
uint32 io_info[][3] = {{PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12,12}};
unsigned char duty_adjust = 0;
void user_state(void)
{
	char outBuf[32] = "";
	switch(system_state){
	case 0: // initializing
		system_state = 1;
		break;
	case 1: //Display Menu
		display_config_menu();
		system_state = 15;
		break;
	case 2: //Display Menu
		display_config_menu();
		system_state = 15;
		os_sprintf(outBuf,"ADC = %d", system_adc_read());
	    uart0_sendStr(outBuf);
		break;

	default: // done for now
		set_sigma_delta_duty(duty_adjust);
		duty_adjust += 0x20;
		break;
	}
}

void user_rf_pre_init(void)
{
}

void user_init(void)
{
	duty = 500;
	system_state = 0;
	serial_init();
	os_timer_disarm(&ptimer);
	os_timer_setfn(&ptimer,user_state,NULL);
	os_timer_arm(&ptimer,1000,1);
	wifi_set_event_handler_cb(wifi_handle_event_cb);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U,FUNC_GPIO2);
	GPIO_OUTPUT_SET(2, 1);
//	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12);
//	GPIO_OUTPUT_SET(12, 1);
//	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12);
//	GPIO_OUTPUT_SET(12, 0);
	config_sigma_delta();
//	set_sigma_target(128);
//	set_sigma_prescale(0);
//	pwm_init(1000,&duty,1,io_info);
//	pwm_start();
}
