/******************************************************************************
 * MIT open source license (Steve McNeil = Mack@PrototypeIteration.com)
 *
 * Based on example code by Espressif
 *******************************************************************************/

#include "osapi.h"
#include "user_interface.h"
#include "wifi.h"
#include "gpio.h"
#include "pwm.h"
#include "eagle_soc.h"
#include "driver/sigma_delta.h"
#include "driver/gpio16.h"

#define FUNC_U0RXD                      0


uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}


void user_rf_pre_init(void)
{
}

// TODO Check for potential buffer Overruns with all sprintf() calls

#define HSPI_PIN 2
void user_init(void)
{
	system_update_cpu_freq(80); //standard clock :)

	setup_io_pins();

	//TODO writeRam is test code. Clean it up.
	writeRam();
	serial_init();

	config_sigma_delta();


}

void setup_io_pins(){
	uint8 pin_index;
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
	VOLTAGE_BOOST_ENABLE();
	gpio16_output_conf();
}
