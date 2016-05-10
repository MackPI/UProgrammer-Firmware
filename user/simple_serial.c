/******************************************************************************
 * MIT open source license (Steve McNeil = Mack@PrototypeIteration.com)
 *
 * FileName: simple_serial.c
 *
 * Description: Create a simple serial interface for setting wifi configuration
 *
 * Modification history:
 *     2015/10/1, v1.0 create this file.
*******************************************************************************/

#include "ets_sys.h"
#include "uart.h"
#include "osapi.h"
#include "uart_register.h"
#include "mem.h"
#include "os_type.h"
#include "simple_serial.h"
#include "user_interface.h"
#include "gpio.h"
#include "pwm.h"
//#include "driver/gpio16.h"


extern UartDevice    UartDev;
struct station_config wifi_config;
char connection_status[64] = "Disconnected";
//extern struct station_config wifi_config;
//extern char connection_status[64];
#define taskQueueLen    10
os_event_t    taskQueue[taskQueueLen];
//extern int system_state;
// Menu states
#define IDLE 0
#define SSID 1
#define PWD 2

void display_config_menu(void){
	char string[128];
	int string_idx= 0;
	while (wifi_config.password[string_idx]!=0) string_idx++; // find null termination
	os_sprintf(string,"\n\r\033[2J SSID:%s, Password Length: %d\n\r", wifi_config.ssid,string_idx);
    uart0_sendStr(string);
    uart0_sendStr(connection_status);
    uart0_sendStr("     Choose");
    uart0_sendStr("\n\r   S - Set SSID to connect");
    uart0_sendStr("\n\r   P - Set Password");
    uart0_sendStr("\n\r   C - Initiate connection");
    uart0_sendStr("\n\r   R - Read ADC Must be between 0 and 1 Volt");
    uart0_sendStr("\n\r   B - Turn On Voltage Boost Circuit");
    uart0_sendStr("\n\r   # - Set duty cycle in multiple of 10 %");
    uart0_sendStr("\n\r   + - Increase ΣΔ Prescaler");
    uart0_sendStr("\n\r   - - Decrease ΣΔ Prescaler");
    uart0_sendStr("\n\r   T - Toggle GPIOs 12,13,14");
    uart0_sendStr("\n\r");

}

void simple_config_ui(char recvd){
	static char ssid[32] = ""; // Access point name
	static char password[64] = ""; // Access point password
	static int string_index = 0;
	static int menu_state = IDLE;
	char teststr[15];
	switch (menu_state){
	case IDLE:
		switch (recvd) {
		case 's': //ssid
		case 'S': //ssid
		    uart0_sendStr("Enter SSID \n\r");
		    menu_state = SSID;
		    string_index = 0; // start at the beginning of the string
		    break;
		case 'p':
		case 'P':
		    uart0_sendStr("Enter Password \n\r");
		    menu_state = PWD;
		    string_index = 0; // start at the beginning of the string
		    break;
		case 'c':
		case 'C':
			if (ssid != ""){ // Only set up connection if firmware has connection Information
				wifi_config.bssid_set = 0;
				wifi_set_opmode(STATION_MODE);
				wifi_station_set_config(&wifi_config);
			    uart0_sendStr("Connecting... \n\r");
			}
		    menu_state = IDLE;
		    string_index = 0; // start at the beginning of the string
		    break;
		case 'r':
		case 'R':
			menu_state = IDLE;
		    system_os_post(USER_TASK_PRIO_0,1,1); // Display menu and ADC Value
		    break;
		case 'i':
		case 'I': // simple test of gpio output with microsecond timing.
			menu_state = IDLE;
		    system_os_post(USER_TASK_PRIO_0,1,0); // Display menu
		    break;
		case 't':
		case 'T': // simple test of gpio output with microsecond timing.
			menu_state = IDLE;
		    system_os_post(USER_TASK_PRIO_0,2,0); // toggle outputs
		    break;
		case 'b':
		case 'B':
			menu_state = IDLE;
		    system_os_post(USER_TASK_PRIO_0,1,0); // Display menu
			GPIO_OUTPUT_SET(13, 0); // Enable Voltage Boost circuit
//		    gpio16_output_set(0); // Enable voltage boost circuit
		    break;
		case '0':
			writeRam("Hello World",11);
			break;
		case '1':
			os_memset(teststr,' ',14);
			teststr[14]=0;
		    uart0_sendStr(teststr);
			readRam(teststr,11);
		    uart0_sendStr(teststr);
			break;
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			set_sigma_delta_duty(((uint8)recvd-0x30)*256/10);
			menu_state = IDLE;
		    system_os_post(USER_TASK_PRIO_0,1,2); // Display menu
		    break;
		case '+':
			set_sigma_delta_prescaler((unsigned char)(get_sigma_delta_prescaler()+1));
			menu_state = IDLE;
		    system_os_post(USER_TASK_PRIO_0,1,2); // Display menu
		    break;
		case '-':
			set_sigma_delta_prescaler((unsigned char)(get_sigma_delta_prescaler()-1));
			menu_state = IDLE;
		    system_os_post(USER_TASK_PRIO_0,1,2); // Display menu
		    break;
		default:
		    system_os_post(USER_TASK_PRIO_0,1,0); // Display menu
			break;
		}
		break;
	case SSID:
        uart_tx_one_char(UART0, recvd);
		ssid[string_index++] = recvd;
		if(recvd == '\r'){
			ssid[string_index-1] = 0;
			os_memcpy(&wifi_config.ssid, ssid,32);
			menu_state = IDLE;
		    system_os_post(USER_TASK_PRIO_0,1,0); // Display menu
		}
		if (recvd == '\n')
			string_index--;
		break;
	case PWD:
        uart_tx_one_char(UART0, '*');
		password[string_index++] = recvd;
		if(recvd == '\r'){
			password[string_index-1] = 0;
			os_memcpy(&wifi_config.password, password,64);
			menu_state = IDLE;
		    system_os_post(USER_TASK_PRIO_0,1,0); // Display menu
		}
		if (recvd == '\n')
			string_index--;
		break;
	default:
		break;
	}
}



void serial_init(void)
{
//	gpio16_output_set(0); // Disable voltage boost circuit
//	gpio16_output_conf();

	// Initialize the serial port for USB serial bridge
	uart_init(115200,115200);
    /*this is a example to process uart data from task,please change the priority to fit your application task if exists*/
    system_os_task(task_handler, USER_TASK_PRIO_0, taskQueue, taskQueueLen);  //demo with a task to process the uart data
    system_os_post(USER_TASK_PRIO_0,1,0); // Display menu
//    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12);
//    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U,FUNC_GPIO13);
//    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U,FUNC_GPIO14);


}

LOCAL void ICACHE_FLASH_ATTR ///////
task_handler(os_event_t *events)
{
	static bool toggle = true;
	char outBuf[32] = "";
    if(events->sig == 0){ //serial recieve task
        uint8 fifo_len = (READ_PERI_REG(UART_STATUS(UART0))>>UART_RXFIFO_CNT_S)&UART_RXFIFO_CNT;
        uint8 d_tmp = 0;
        uint8 idx=0;
        do {
            d_tmp = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
        	simple_config_ui(d_tmp);
        } while ((READ_PERI_REG(UART_STATUS(UART0))>>UART_RXFIFO_CNT_S)&UART_RXFIFO_CNT>0);
        WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR|UART_RXFIFO_TOUT_INT_CLR);
        uart_rx_intr_enable(UART0);
    }
    if(events->sig == 1){ //Menu task
		display_config_menu();
		if (events->par == 1){// display ADC reading
 		os_sprintf(outBuf,"ADC = %d", system_adc_read());
		uart0_sendStr(outBuf);
		}
		if (events->par == 2){// display ADC reading
 		os_sprintf(outBuf,"ΣΔ duty cycle = %d / Prescaler = %d", get_sigma_delta_duty(),get_sigma_delta_prescaler());
		uart0_sendStr(outBuf);
		}
    }
    if(events->sig == 2){ //Toggle Outputs
    	if (toggle){
    		GPIO_OUTPUT_SET(12,1);
//    		GPIO_OUTPUT_SET(13,1);
    		GPIO_OUTPUT_SET(14,0);
    	}else{
    		GPIO_OUTPUT_SET(12,0);
//    		GPIO_OUTPUT_SET(13,0);
    		GPIO_OUTPUT_SET(14,1);
    	}
    	toggle = !toggle;
    }
}

/******************************************************************************
 * FunctionName : uart_config
 * Description  : Internal used function
 *                UART0 used for data TX/RX, RX buffer size is 0x100, interrupt enabled
 *                UART1 just used for debug output
 * Parameters   : uart_no, use UART0 or UART1 defined ahead
 * Returns      : NONE
 * Provided in Espressif IOT SDK
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
uart_config(uint8 uart_no)
{
    if (uart_no == UART1){
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_U1TXD_BK);
    }else{
        /* rcv_buff size if 0x100 */
        ETS_UART_INTR_ATTACH(uart0_rx_intr_handler,  &(UartDev.rcv_buff));
        PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);
    }
    uart_div_modify(uart_no, UART_CLK_FREQ / (UartDev.baut_rate));//SET BAUDRATE

    WRITE_PERI_REG(UART_CONF0(uart_no), ((UartDev.exist_parity & UART_PARITY_EN_M)  <<  UART_PARITY_EN_S) //SET BIT AND PARITY MODE
                                                                        | ((UartDev.parity & UART_PARITY_M)  <<UART_PARITY_S )
                                                                        | ((UartDev.stop_bits & UART_STOP_BIT_NUM) << UART_STOP_BIT_NUM_S)
                                                                        | ((UartDev.data_bits & UART_BIT_NUM) << UART_BIT_NUM_S));

    //clear rx and tx fifo,not ready
    SET_PERI_REG_MASK(UART_CONF0(uart_no), UART_RXFIFO_RST | UART_TXFIFO_RST);    //RESET FIFO
    CLEAR_PERI_REG_MASK(UART_CONF0(uart_no), UART_RXFIFO_RST | UART_TXFIFO_RST);

    if (uart_no == UART0){
        //set rx fifo trigger
        WRITE_PERI_REG(UART_CONF1(uart_no),
        ((100 & UART_RXFIFO_FULL_THRHD) << UART_RXFIFO_FULL_THRHD_S) |

		(0x02 & UART_RX_TOUT_THRHD) << UART_RX_TOUT_THRHD_S |
        UART_RX_TOUT_EN|
        ((0x10 & UART_TXFIFO_EMPTY_THRHD)<<UART_TXFIFO_EMPTY_THRHD_S));//wjl
        SET_PERI_REG_MASK(UART_INT_ENA(uart_no), UART_RXFIFO_TOUT_INT_ENA |UART_FRM_ERR_INT_ENA);
    }else{
        WRITE_PERI_REG(UART_CONF1(uart_no),((UartDev.rcv_buff.TrigLvl & UART_RXFIFO_FULL_THRHD) << UART_RXFIFO_FULL_THRHD_S));//TrigLvl default val == 1
    }
    //clear all interrupt
    WRITE_PERI_REG(UART_INT_CLR(uart_no), 0xffff);
    //enable rx_interrupt
    SET_PERI_REG_MASK(UART_INT_ENA(uart_no), UART_RXFIFO_FULL_INT_ENA|UART_RXFIFO_OVF_INT_ENA);
}

/******************************************************************************
 * FunctionName : uart0_rx_intr_handler
 * Description  : Internal used function
 *                UART0 interrupt handler, add self handle code inside
 * Parameters   : void *para - point to ETS_UART_INTR_ATTACH's arg
 * Returns      : NONE
 * Provided in Espressif IOT SDK
*******************************************************************************/
LOCAL void
uart0_rx_intr_handler(void *para)
{
    /* uart0 and uart1 intr combine togther, when interrupt occur, see reg 0x3ff20020, bit2, bit0 represents
    * uart1 and uart0 respectively
    */
    uint8 RcvChar;
    uint8 uart_no = UART0;//UartDev.buff_uart_no;
    uint8 fifo_len = 0;
    uint8 buf_idx = 0;
    uint8 temp,cnt;
    //RcvMsgBuff *pRxBuff = (RcvMsgBuff *)para;

    	/*ATTENTION:*/
	/*IN NON-OS VERSION SDK, DO NOT USE "ICACHE_FLASH_ATTR" FUNCTIONS IN THE WHOLE HANDLER PROCESS*/
	/*ALL THE FUNCTIONS CALLED IN INTERRUPT HANDLER MUST BE DECLARED IN RAM */
	/*IF NOT , POST AN EVENT AND PROCESS IN SYSTEM TASK */
    if(UART_FRM_ERR_INT_ST == (READ_PERI_REG(UART_INT_ST(uart_no)) & UART_FRM_ERR_INT_ST)){
        WRITE_PERI_REG(UART_INT_CLR(uart_no), UART_FRM_ERR_INT_CLR);
    }else if(UART_RXFIFO_FULL_INT_ST == (READ_PERI_REG(UART_INT_ST(uart_no)) & UART_RXFIFO_FULL_INT_ST)){
        uart_rx_intr_disable(UART0);
        WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR);
        system_os_post(USER_TASK_PRIO_0, 0, 0);
    }else if(UART_RXFIFO_TOUT_INT_ST == (READ_PERI_REG(UART_INT_ST(uart_no)) & UART_RXFIFO_TOUT_INT_ST)){
        uart_rx_intr_disable(UART0);
        WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_TOUT_INT_CLR);
        system_os_post(USER_TASK_PRIO_0, 0, 0);
    }else if(UART_TXFIFO_EMPTY_INT_ST == (READ_PERI_REG(UART_INT_ST(uart_no)) & UART_TXFIFO_EMPTY_INT_ST)){
	/* to output uart data from uart buffer directly in empty interrupt handler*/
	/*instead of processing in system event, in order not to wait for current task/function to quit */
	/*ATTENTION:*/
	/*IN NON-OS VERSION SDK, DO NOT USE "ICACHE_FLASH_ATTR" FUNCTIONS IN THE WHOLE HANDLER PROCESS*/
	/*ALL THE FUNCTIONS CALLED IN INTERRUPT HANDLER MUST BE DECLARED IN RAM */
	CLEAR_PERI_REG_MASK(UART_INT_ENA(UART0), UART_TXFIFO_EMPTY_INT_ENA);
	#if UART_BUFF_EN
		tx_start_uart_buffer(UART0);
	#endif
        //system_os_post(uart_recvTaskPrio, 1, 0);
        WRITE_PERI_REG(UART_INT_CLR(uart_no), UART_TXFIFO_EMPTY_INT_CLR);

    }else if(UART_RXFIFO_OVF_INT_ST  == (READ_PERI_REG(UART_INT_ST(uart_no)) & UART_RXFIFO_OVF_INT_ST)){
        WRITE_PERI_REG(UART_INT_CLR(uart_no), UART_RXFIFO_OVF_INT_CLR);
    }

}

