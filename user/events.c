/*
 * events.c
 *
 *  Created on: Sep 2, 2016
 *      Author: Mack
 */
#include "events.h"
#include "uart.h"
#include "user_interface.h"
#include "osapi.h"
#include "simple_serial.h"

extern char connection_status[64];
//TODO clean up this code for readability
void // ICACHE_FLASH_ATTR ///////
taskHandler(os_event_t *events)
{
	static bool toggle = true;
	char outBuf[32] = "";
	if (events->sig == 0)
	{ //serial recieve task
		uint8 fifo_len =
				(READ_PERI_REG(UART_STATUS(UART0)) >> UART_RXFIFO_CNT_S)
						& UART_RXFIFO_CNT;
		uint8 d_tmp = 0;
		uint8 idx = 0;
		do
		{
			d_tmp = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
			simpleConfigUI(d_tmp);
		} while ((READ_PERI_REG(UART_STATUS(UART0)) >> UART_RXFIFO_CNT_S)
				& UART_RXFIFO_CNT > 0);
		WRITE_PERI_REG(UART_INT_CLR(UART0),
				UART_RXFIFO_FULL_INT_CLR|UART_RXFIFO_TOUT_INT_CLR);
		uartRxInterruptEnable(UART0);
	}
	if (events->sig == 1)
	{ //Menu task
		displayConfigMenu();
		if (events->par == 1)
		{ // display ADC reading
//			int adc_result = system_adc_read();
			int adc_result = adcValue;
			int scaled_result = (210*adc_result)/1023;
			os_sprintf(outBuf, "ADC = %03X (%d.%d Volts)", adc_result, scaled_result/10, scaled_result%10);
			uart0SendStr(outBuf);
		}
		if (events->par == 2)
		{ // display Sigma Delts settings
			os_sprintf(outBuf, "ΣΔ duty = %d / Pre = %d",
					getSigmaDeltaDuty(), getSigmaDeltaPrescaler());
			uart0SendStr(outBuf);
		}
		if (events->par == 3)
		{ // display First bytes of SPI RAM
// 		os_sprintf(outBuf,"ΣΔ duty cycle = %d / Prescaler = %d", get_sigma_delta_duty(),get_sigma_delta_prescaler());
			readRam(outBuf, 12); // read 12 bytes from beginning of ram.
			outBuf[12] = 0;
			uart0SendStr(outBuf); // and display them.
		}

	}
	if (events->sig == 2)
	{ //Toggle Outputs
		if (toggle)
		{
			GPIO_OUTPUT_SET(12, 1);
//    		GPIO_OUTPUT_SET(13,1);
			GPIO_OUTPUT_SET(14, 0);
		}
		else
		{
			GPIO_OUTPUT_SET(12, 0);
//    		GPIO_OUTPUT_SET(13,0);
			GPIO_OUTPUT_SET(14, 1);
		}
		toggle = !toggle;
	}
}



void adc_read(void)
{
	static uint8 cycle_count = 0;

	uint8 wifi_opmode;
//	if(connection_status[0] == 'C'){
//		wifi_opmode = wifi_get_opmode();
//		wifi_set_opmode_current(NULL_MODE);
		GPIO_OUTPUT_SET(14, 1);
		system_adc_read_fast(&adcValue,1,8);
		GPIO_OUTPUT_SET(14, 0);
		if (display_now & (200 <= cycle_count++))
		{
			DISPLAY_MENU_W_ADC();
			cycle_count=0;
		}
//		wifi_set_opmode_current(wifi_opmode);
//	}
}
void ICACHE_FLASH_ATTR
init_adc_timer(void)
{
	display_now=true;
	os_timer_setfn(&tadc,&adc_read,NULL);
	os_timer_arm(&tadc,5,1); // repeat approximately every 100 mS
}
