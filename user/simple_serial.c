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

#include "uart.h"
#include "osapi.h"
#include "simple_serial.h"
#include "user_interface.h"
//#include "gpio.h"
#include "events.h"

struct station_config wifi_config;
char connection_status[64] = "Disconnected";
#define taskQueueLen    10
os_event_t taskQueue[taskQueueLen];
#define IDLE 0
#define SSID 1
#define PWD 2
unsigned char prescaler = 1;
static int menu_state = IDLE;
static char ssid[32] = ""; // Access point name
static char password[64] = ""; // Access point password
static int string_index = 0;

void display_config_menu(void)
{
	char string[128];
	int string_idx = 0;
	while (wifi_config.password[string_idx] != 0)
		string_idx++; // find null termination
	os_sprintf(string, "\n\r\033[2J SSID:%s, Password Length: %d\n\r",
			wifi_config.ssid, string_idx);
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

void simple_config_ui(char in)
{
	static int string_index = 0;
	char teststr[15];
	switch (menu_state)
	{
	case IDLE:
		receive_next(in);
		break;
	case SSID:
		get_new_ssid(in);
		break;
	case PWD:
		get_new_password(in);
		break;
	default:
		break;
	}
}

void serial_init(void)
{
	// Initialize the serial port for USB serial bridge
	uart_init(115200, 115200);
	TURN_OFF_SYSTEM_MESSAGES();
	system_os_task(task_handler, USER_TASK_PRIO_0, taskQueue, taskQueueLen);
	DISPLAY_MENU();

}

void get_new_ssid(char recv)
{
	uart_tx_one_char(UART0, recv);
	ssid[string_index++] = recv;
	if (recv == '\r')
	{
		ssid[string_index - 1] = 0;
		os_memcpy(&wifi_config.ssid, ssid, 32);
		menu_state = IDLE;
		system_os_post(USER_TASK_PRIO_0, 1, 0); // Display menu
	}
	if (recv == '\n')
		string_index--;
}

void get_new_password(char recv)
{
	uart_tx_one_char(UART0, '*');
	password[string_index++] = recv;
	if (recv == '\r')
	{
		password[string_index - 1] = 0;
		os_memcpy(&wifi_config.password, password, 64);
		menu_state = IDLE;
		system_os_post(USER_TASK_PRIO_0, 1, 0); // Display menu
	}
	if (recv == '\n')
		string_index--;
}

void receive_next(char recvd)
{
	static int string_index = 0;
	switch (recvd)
	{
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
		if (ssid != "")
		{ // Only set up connection if firmware has connection Information
			wifi_config.bssid_set = 0;
			wifi_set_opmode(STATION_MODE);
			wifi_station_set_config(&wifi_config);
			uart0_sendStr("Connecting... \n\r");
			wifi_station_connect();
		}
		menu_state = IDLE;
		string_index = 0; // start at the beginning of the string
		break;
	case 'r':
	case 'R':
		menu_state = IDLE;
		DISPLAY_MENU_W_ADC();
		//system_os_post(USER_TASK_PRIO_0, 1, 1); // Display menu and ADC Value
		break;
	case 't':
	case 'T': // simple test of gpio output with microsecond timing.
		menu_state = IDLE;
		system_os_post(USER_TASK_PRIO_0, 2, 0); // toggle outputs
		break;
	case 'b':
	case 'B':
		menu_state = IDLE;
		DISPLAY_MENU();
		GPIO_OUTPUT_SET(13, 0);
		// Enable Voltage Boost circuit
		gpio16_output_set(0); // Enable voltage boost circuit
		break;
	case '0':
		writeRam("Hello World", 11);
		break;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		set_sigma_delta_duty(((uint8) recvd - 0x30) * 256 / 10);
		menu_state = IDLE;
		DISPLAY_MENU_W_SIGMA_DELTA();
		break;
	case '+':
		prescaler++;
		set_sigma_delta_prescaler(prescaler);
//			set_sigma_delta_prescaler((unsigned char)(get_sigma_delta_prescaler()+1));
		menu_state = IDLE;
		DISPLAY_MENU_W_SIGMA_DELTA();
		break;
	case '-':
		prescaler--;
		set_sigma_delta_prescaler(prescaler);
		menu_state = IDLE;
		DISPLAY_MENU_W_SIGMA_DELTA();
		break;
	default:
		DISPLAY_MENU();
		break;
	}
}


