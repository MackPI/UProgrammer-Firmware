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
#include "osapi.h"
#include "uart.h"
#include "osapi.h"
#include "uart_register.h"
#include "mem.h"
#include "os_type.h"
#include "simple_serial.h"
#include "user_interface.h"

extern struct station_config wifi_config;
extern char connection_status[64];
#define uart_recvTaskPrio        0
#define uart_recvTaskQueueLen    10
os_event_t    uart_recvTaskQueue[uart_recvTaskQueueLen];
extern int system_state;
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
    uart0_sendStr("\n\r   C - Initiate connection\n\r");
}

void simple_config_ui(char recvd){
	static char ssid[32] = ""; // Access point name
	static char password[64] = ""; // Access point password
	static int string_index = 0;
	static int menu_state = IDLE;
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
		//		os_memcpy(&wifi_config.ssid, ssid,32);
		//		os_memcpy(&wifi_config.password, password,64);
				wifi_config.bssid_set = 0;
				wifi_set_opmode(STATION_MODE);
				wifi_station_set_config(&wifi_config);
				system_state = 1;//				display_config_menu();
			    uart0_sendStr("Connecting... \n\r");
			}
		    menu_state = IDLE;
		    string_index = 0; // start at the beginning of the string
		    break;
		default:
		    uart0_sendStr("Unknown \n\r");
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
			system_state = 1;//			display_config_menu();
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
			system_state = 1;//			display_config_menu();
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
	// Initialize the serial port for USB serial bridge
	uart_init(115200,115200);
    /*this is a example to process uart data from task,please change the priority to fit your application task if exists*/
    system_os_task(uart_recvTask, uart_recvTaskPrio, uart_recvTaskQueue, uart_recvTaskQueueLen);  //demo with a task to process the uart data


}

LOCAL void ICACHE_FLASH_ATTR ///////
uart_recvTask(os_event_t *events)
{
    if(events->sig == 0){
        uint8 fifo_len = (READ_PERI_REG(UART_STATUS(UART0))>>UART_RXFIFO_CNT_S)&UART_RXFIFO_CNT;
        uint8 d_tmp = 0;
        uint8 idx=0;
        do {
//        for(idx=0;idx<fifo_len;idx++) {
            d_tmp = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
        	simple_config_ui(d_tmp);
//            uart_tx_one_char(UART0, d_tmp);
        } while ((READ_PERI_REG(UART_STATUS(UART0))>>UART_RXFIFO_CNT_S)&UART_RXFIFO_CNT>0);
        WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR|UART_RXFIFO_TOUT_INT_CLR);
        uart_rx_intr_enable(UART0);
    }
}
