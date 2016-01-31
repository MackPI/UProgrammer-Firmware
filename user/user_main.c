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
#include "user_interface.h"
#include "wifi.h"
//#include "driver/spi_overlap.h"
#include "gpio.h"
#include "pwm.h"
#include "eagle_soc.h"
#include "driver/sigma_delta.h"
#include "driver/gpio16.h"
//#include "driver/spi_overlap.h"
void user_rf_pre_init(void)
{
}

void user_init(void)
{
//	hspi_overlap_init();
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15); // GPIO15 = HSPICS
	GPIO_OUTPUT_SET(15, 1); // Disable SPI RAM
	system_update_cpu_freq(80); //standard clock :)
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U,FUNC_GPIO13);
	GPIO_OUTPUT_SET(13, 1); // Disable Voltage Boost circuit

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U,FUNC_GPIO4);
//    GPIO_OUTPUT_SET(4, 1);

	serial_init();
	wifi_set_event_handler_cb(wifi_handle_event_cb);


	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U,FUNC_GPIO2);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U,FUNC_GPIO5);

/* having problems every time I try to add GPIO16
 * Moved voltage boost enable to GPIO13 for further testing
 */
	gpio16_output_set(1); // Disable voltage boost circuit
	gpio16_output_conf();

	GPIO_OUTPUT_SET(5, 0); //Set High to GND VPP
	GPIO_OUTPUT_SET(2, 1); //Set High to output VPP
	config_sigma_delta();

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, 2); // GPIO15 = HSPICS
//	GPIO_OUTPUT_SET(15, 1); // Disable SPI RAM
	hspi_overlap_init();
}
