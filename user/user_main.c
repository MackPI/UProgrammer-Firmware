/******************************************************************************
 * MIT open source license (Steve McNeil = Mack@PrototypeIteration.com)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2015/10/1, v1.0 create this file.
 *     2016/6/9, V1.0a moved to NONOS SDK V1.5.4
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
#define FUNC_U0RXD                      0

//#include "driver/spi_overlap.h"
void user_rf_pre_init(void)
{
}

#define HSPI_PIN 2
void user_init(void)
{
	uint8 pin_index;
	system_update_cpu_freq(80); //standard clock :)

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0); // Analog Switch
	GPIO_OUTPUT_SET(0, 0); //High= Vt Low = Vpp
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD); // Uart0 Controlled by API
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO2); //Vpp High Drive
	GPIO_OUTPUT_SET(2, 1); //Set High to output VPP
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_U0RXD); // Uart0 Controlled by API
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U,FUNC_GPIO4); // ΣΔ pulse stream for Voltage pump
    GPIO_OUTPUT_SET(4, 1);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U,FUNC_GPIO5); //Vpp Low Drive
    GPIO_OUTPUT_SET(5, 0); //Set High to GND VPP
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_CLK_U,FUNC_SPICLK); //GPIO6 SPI SCLK
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA0_U,FUNC_SPIQ); //GPIO7 SPI MISO
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA1_U,FUNC_SPID); //GPIO8 SPI MOSI
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA2_U,FUNC_SPIHD); //GPIO9 SPI QIO2
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA3_U,FUNC_SPIWP); //GPIO10 SPI QIO3
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_CMD_U,FUNC_SPICS0); //GPIO11 SPI CS0
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12); //Target data pin
    GPIO_OUTPUT_SET(12, 0);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U,FUNC_GPIO13); //Target data pin
	GPIO_OUTPUT_SET(13, 0); // Set to known state
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U,FUNC_GPIO14); //Target data pin
	for (pin_index = 0;pin_index <15;pin_index++){
		GPIO_REG_WRITE((pin_index*4)+0x60000328,0); // disconnect all pins from ΣΔ
	}

//	hspi_overlap_init();
////	CLEAR_PERI_REG_MASK(PERIPHS_IO_MUX, BIT9);
//	writeRam();
	serial_init();
//	wifi_set_event_handler_cb(wifi_handle_event_cb);



/* having problems every time I try to add GPIO16
 * Moved voltage boost enable to GPIO13 for further testing
 */
	gpio16_output_set(1); // Disable voltage boost circuit
//	gpio16_output_set(0); // Enable voltage boost circuit
	gpio16_output_conf();

	config_sigma_delta();

	wifi_set_opmode(NULL_MODE);
	wifi_fpm_set_sleep_type (MODEM_SLEEP_T);
	wifi_fpm_open(); // force modem to sleep

}
