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
			os_sprintf(outBuf, "ADC = %d", system_adc_read());
			uart0SendStr(outBuf);
		}
		if (events->par == 2)
		{ // display ADC reading
			os_sprintf(outBuf, "ΣΔ duty = %d / Pre = %d",
					getSigmaDeltaDuty(), getSigmaDeltaPrescaler());
			uart0SendStr(outBuf);
		}
		if (events->par == 3)
		{ // display ADC reading
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
