/*
 * uart.c
 *
 *  Created on: Sep 2, 2016
 *      Author: Mack
 */
#include "uart.h"
#include "user_interface.h"

extern UartDevice UartDev;
os_event_t uart_recvTaskQueue[uart_recvTaskQueueLen];

LOCAL void uart0RxIntrHandler(void *para);

/******************************************************************************
 * FunctionName : uart_config
 * Description  : Internal used function
 *                UART0 used for data TX/RX, RX buffer size is 0x100, interrupt enabled
 *                UART1 just used for debug output
 * Parameters   : uart_no, use UART0 or UART1 defined ahead
 * Returns      : NONE
 * Provided in Espressif IOT SDK
 *******************************************************************************/LOCAL void // ICACHE_FLASH_ATTR
uartConfig(uint8 uart_no)
{
	if (uart_no == UART1)
	{
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_U1TXD_BK);
	}
	else
	{
		/* rcv_buff size if 0x100 */
		ETS_UART_INTR_ATTACH(uart0RxIntrHandler, &(UartDev.rcv_buff));
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);
	}
	uart_div_modify(uart_no, UART_CLK_FREQ / (UartDev.baut_rate)); //SET BAUDRATE

	WRITE_PERI_REG(UART_CONF0(uart_no),
			((UartDev.exist_parity & UART_PARITY_EN_M) << UART_PARITY_EN_S) //SET BIT AND PARITY MODE
			| ((UartDev.parity & UART_PARITY_M) <<UART_PARITY_S ) | ((UartDev.stop_bits & UART_STOP_BIT_NUM) << UART_STOP_BIT_NUM_S) | ((UartDev.data_bits & UART_BIT_NUM) << UART_BIT_NUM_S));

	//clear rx and tx fifo,not ready
	SET_PERI_REG_MASK(UART_CONF0(uart_no), UART_RXFIFO_RST | UART_TXFIFO_RST);
	//RESET FIFO
	CLEAR_PERI_REG_MASK(UART_CONF0(uart_no), UART_RXFIFO_RST | UART_TXFIFO_RST);

	if (uart_no == UART0)
	{
		//set rx fifo trigger
		WRITE_PERI_REG(UART_CONF1(uart_no),
				((100 & UART_RXFIFO_FULL_THRHD) << UART_RXFIFO_FULL_THRHD_S) |

				(0x02 & UART_RX_TOUT_THRHD) << UART_RX_TOUT_THRHD_S | UART_RX_TOUT_EN| ((0x10 & UART_TXFIFO_EMPTY_THRHD)<<UART_TXFIFO_EMPTY_THRHD_S));
		//wjl
		SET_PERI_REG_MASK(UART_INT_ENA(uart_no),
				UART_RXFIFO_TOUT_INT_ENA |UART_FRM_ERR_INT_ENA);
	}
	else
	{
		WRITE_PERI_REG(UART_CONF1(uart_no),
				((UartDev.rcv_buff.TrigLvl & UART_RXFIFO_FULL_THRHD) << UART_RXFIFO_FULL_THRHD_S));
		//TrigLvl default val == 1
	}
	//clear all interrupt
	WRITE_PERI_REG(UART_INT_CLR(uart_no), 0xffff);
	//enable rx_interrupt
	SET_PERI_REG_MASK(UART_INT_ENA(uart_no),
			UART_RXFIFO_FULL_INT_ENA|UART_RXFIFO_OVF_INT_ENA);
}

/******************************************************************************
 * FunctionName : uart0_rx_intr_handler
 * Description  : Internal used function
 *                UART0 interrupt handler, add self handle code inside
 * Parameters   : void *para - point to ETS_UART_INTR_ATTACH's arg
 * Returns      : NONE
 * Provided in Espressif IOT SDK
 *******************************************************************************/
 LOCAL void uart0RxIntrHandler(void *para)
{
	/* uart0 and uart1 intr combine togther, when interrupt occur, see reg 0x3ff20020, bit2, bit0 represents
	 * uart1 and uart0 respectively
	 */
	uint8 RcvChar;
	uint8 uart_no = UART0; //UartDev.buff_uart_no;
	uint8 fifo_len = 0;
	uint8 buf_idx = 0;
	uint8 temp, cnt;
	//RcvMsgBuff *pRxBuff = (RcvMsgBuff *)para;

	/*ATTENTION:*/
	/*IN NON-OS VERSION SDK, DO NOT USE "ICACHE_FLASH_ATTR" FUNCTIONS IN THE WHOLE HANDLER PROCESS*/
	/*ALL THE FUNCTIONS CALLED IN INTERRUPT HANDLER MUST BE DECLARED IN RAM */
	/*IF NOT , POST AN EVENT AND PROCESS IN SYSTEM TASK */
	if (UART_FRM_ERR_INT_ST
			== (READ_PERI_REG(UART_INT_ST(uart_no)) & UART_FRM_ERR_INT_ST))
	{
		WRITE_PERI_REG(UART_INT_CLR(uart_no), UART_FRM_ERR_INT_CLR);
	}
	else if (UART_RXFIFO_FULL_INT_ST
			== (READ_PERI_REG(UART_INT_ST(uart_no)) & UART_RXFIFO_FULL_INT_ST))
	{
		uartRxInterruptDisable(UART0);
		WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR);
		system_os_post(USER_TASK_PRIO_0, 0, 0); //ToDO Make this a macro
	}
	else if (UART_RXFIFO_TOUT_INT_ST
			== (READ_PERI_REG(UART_INT_ST(uart_no)) & UART_RXFIFO_TOUT_INT_ST))
	{
		uartRxInterruptDisable(UART0);
		WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_TOUT_INT_CLR);
		system_os_post(USER_TASK_PRIO_0, 0, 0);
	}
	else if (UART_TXFIFO_EMPTY_INT_ST
			== (READ_PERI_REG(UART_INT_ST(uart_no)) & UART_TXFIFO_EMPTY_INT_ST))
	{
		/* to output uart data from uart buffer directly in empty interrupt handler*/
		/*instead of processing in system event, in order not to wait for current task/function to quit */
		/*ATTENTION:*/
		/*IN NON-OS VERSION SDK, DO NOT USE "ICACHE_FLASH_ATTR" FUNCTIONS IN THE WHOLE HANDLER PROCESS*/
		/*ALL THE FUNCTIONS CALLED IN INTERRUPT HANDLER MUST BE DECLARED IN RAM */
		CLEAR_PERI_REG_MASK(UART_INT_ENA(UART0), UART_TXFIFO_EMPTY_INT_ENA);
#if UART_BUFF_EN
		tx_start_uart_buffer(UART0);
#endif
		//system_os_post(UART_RECV_TASK_PRIO, 1, 0);
		WRITE_PERI_REG(UART_INT_CLR(uart_no), UART_TXFIFO_EMPTY_INT_CLR);

	}
	else if (UART_RXFIFO_OVF_INT_ST
			== (READ_PERI_REG(UART_INT_ST(uart_no)) & UART_RXFIFO_OVF_INT_ST))
	{
		WRITE_PERI_REG(UART_INT_CLR(uart_no), UART_RXFIFO_OVF_INT_CLR);
	}

}

/******************************************************************************
 * FunctionName : uart1_tx_one_char
 * Description  : Internal used function
 *                Use uart1 interface to transfer one char
 * Parameters   : uint8 TxChar - character to tx
 * Returns      : OK
 *******************************************************************************/
STATUS uartTxOneChar(uint8 uart, uint8 TxChar)
{
	while (true)
	{
		uint32 fifo_cnt = READ_PERI_REG(UART_STATUS(uart))
				& (UART_TXFIFO_CNT << UART_TXFIFO_CNT_S);
		if ((fifo_cnt >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT) < 126)
		{
			break;
		}
	}
	WRITE_PERI_REG(UART_FIFO(uart), TxChar);
	return OK;
}

/******************************************************************************
 * FunctionName : uart0_sendStr
 * Description  : use uart0 to transfer buffer
 * Parameters   : uint8 *buf - point to send buffer
 *                uint16 len - buffer len
 * Returns      :
 *******************************************************************************/
void ICACHE_FLASH_ATTR
uart0SendStr(const char *str)
{
	while (*str)
	{
		uartTxOneChar(UART0, *str++);
	}
}

/******************************************************************************
 * FunctionName : uart_init
 * Description  : user interface for init uart
 * Parameters   : UartBautRate uart0_br - uart0 bautrate
 *                UartBautRate uart1_br - uart1 bautrate
 * Returns      : NONE
 *******************************************************************************/
//LOCAL void ICACHE_FLASH_ATTR ///////
LOCAL void uartRecvTask(os_event_t *events)
{
	if (events->sig == 0)
	{
#if  UART_BUFF_EN
		Uart_rx_buff_enq();
#else
		uint8 fifo_len =
				(READ_PERI_REG(UART_STATUS(UART0)) >> UART_RXFIFO_CNT_S)
						& UART_RXFIFO_CNT;
		uint8 d_tmp = 0;
		uint8 idx = 0;
		for (idx = 0; idx < fifo_len; idx++)
		{
			d_tmp = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
			uartTxOneChar(UART0, d_tmp);
		}
		WRITE_PERI_REG(UART_INT_CLR(UART0),
				UART_RXFIFO_FULL_INT_CLR|UART_RXFIFO_TOUT_INT_CLR);
		uartRxInterruptEnable(UART0);
#endif
	}
	else if (events->sig == 1)
	{
//#if UART_BUFF_EN
//		//already move uart buffer output to uart empty interrupt
//		//tx_start_uart_buffer(UART0);
//#else
//
//#endif
	}
}

void ICACHE_FLASH_ATTR
uartInit(UartBautRate uart0_br, UartBautRate uart1_br)
{
	/*this is a example to process uart data from task,please change the priority to fit your application task if exists*/
	system_os_task(uartRecvTask, UART_RECV_TASK_PRIO, uart_recvTaskQueue,
			uart_recvTaskQueueLen); //demo with a task to process the uart data

	UartDev.baut_rate = uart0_br;
	uartConfig(UART0);
	UartDev.baut_rate = uart1_br;
	uartConfig(UART1);
	ETS_UART_INTR_ENABLE();

//#if UART_BUFF_EN
//	pTxBuffer = Uart_Buf_Init(UART_TX_BUFFER_SIZE);
//	pRxBuffer = Uart_Buf_Init(UART_RX_BUFFER_SIZE);
//#endif

	/*option 1: use default print, output from uart0 , will wait some time if fifo is full */
	//do nothing...
	/*option 2: output from uart1,uart1 output will not wait , just for output debug info */
	/*os_printf output uart data via uart1(GPIO2)*/
	//os_install_putc1((void *)uart1_write_char);    //use this one to output debug information via uart1 //
	/*option 3: output from uart0 will skip current byte if fifo is full now... */
	/*see uart0_write_char_no_wait:you can output via a buffer or output directly */
	/*os_printf output uart data via uart0 or uart buffer*/
	//os_install_putc1((void *)uart0_write_char_no_wait);  //use this to print via uart0
//#if UART_SELFTEST&UART_BUFF_EN
//	os_timer_disarm(&buff_timer_t);
//	os_timer_setfn(&buff_timer_t, uart_test_rx , NULL); //a demo to process the data in uart rx buffer
//	os_timer_arm(&buff_timer_t,10,1);
//#endif
}

void uartRxInterruptDisable(uint8 uart_no)
{
#if 1
	CLEAR_PERI_REG_MASK(UART_INT_ENA(uart_no),
			UART_RXFIFO_FULL_INT_ENA|UART_RXFIFO_TOUT_INT_ENA);
//#else
//	ETS_UART_INTR_DISABLE();
#endif
}

void uartRxInterruptEnable(uint8 uart_no)
{
#if 1
	SET_PERI_REG_MASK(UART_INT_ENA(uart_no),
			UART_RXFIFO_FULL_INT_ENA|UART_RXFIFO_TOUT_INT_ENA);
//#else
//	ETS_UART_INTR_ENABLE();
#endif
}

