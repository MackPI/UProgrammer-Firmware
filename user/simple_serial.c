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
static int menuState = IDLE;
static char ssid[32] = ""; // Access point name
static char password[64] = ""; // Access point password
static int string_index = 0;

void displayConfigMenu(void)
{
	char string[128];
	int string_idx = 0;
	while (wifi_config.password[string_idx] != 0)
		string_idx++; // find null termination
	os_sprintf(string, "\n\r\033[2J SSID:%s, Password Length: %d\n\r",
			wifi_config.ssid, string_idx);
	uart0SendStr(string);
	uart0SendStr(connection_status);
	uart0SendStr("     Choose");
	uart0SendStr("\n\r   S - Set SSID to connect");
	uart0SendStr("\n\r   P - Set Password");
	uart0SendStr("\n\r   C - Initiate connection");
	uart0SendStr("\n\r   R - Read ADC Must be between 0 and 1 Volt");
	uart0SendStr("\n\r   B - Turn On Voltage Boost Circuit");
	uart0SendStr("\n\r   # - Set duty cycle in multiple of 10 %");
	uart0SendStr("\n\r   + - Increase ΣΔ Prescaler");
	uart0SendStr("\n\r   - - Decrease ΣΔ Prescaler");
	uart0SendStr("\n\r   T - Toggle GPIOs 12,13,14");
	uart0SendStr("\n\r");

}

void simpleConfigUI(char in)
{
	static int string_index = 0;
	char teststr[15];
	switch (menuState)
	{
	case IDLE:
		receiveNext(in);
		break;
	case SSID:
		getNewSSID(in);
		break;
	case PWD:
		getNewPassword(in);
		break;
	default:
		break;
	}
}

void serialInit(void)
{
	// Initialize the serial port for USB serial bridge
	uartInit(115200, 115200);
	TURN_OFF_SYSTEM_MESSAGES();
	system_os_task(taskHandler, USER_TASK_PRIO_0, taskQueue, taskQueueLen);
	DISPLAY_MENU();
}

void getNewSSID(char recv)
{
	uartTxOneChar(UART0, recv);
	ssid[string_index++] = recv;
	if (recv == '\r')
	{
		ssid[string_index - 1] = 0;
		os_memcpy(&wifi_config.ssid, ssid, 32);
		menuState = IDLE;
		DISPLAY_MENU();
//		system_os_post(USER_TASK_PRIO_0, 1, 0); // Display menu
	}
	if (recv == '\n')
		string_index--;
}

void getNewPassword(char recv)
{
	uartTxOneChar(UART0, '*');
	password[string_index++] = recv;
	if (recv == '\r')
	{
		password[string_index - 1] = 0;
		os_memcpy(&wifi_config.password, password, 64);
		menuState = IDLE;
		DISPLAY_MENU();
//  	system_os_post(USER_TASK_PRIO_0, 1, 0); // Display menu
	}
	if (recv == '\n')
		string_index--;
}

void receiveNext(char recvd)
{
	static int string_index = 0;
	switch (recvd)
	{
	case 's': //ssid
	case 'S': //ssid
		uart0SendStr("Enter SSID \n\r");
		menuState = SSID;
		string_index = 0; // start at the beginning of the string
		break;
	case 'p':
	case 'P':
		uart0SendStr("Enter Password \n\r");
		menuState = PWD;
		string_index = 0; // start at the beginning of the string
		break;
	case 'c':
	case 'C':
		if (ssid != "")
		{ // Only set up connection if firmware has connection Information
			wifi_config.bssid_set = 0;
			wifi_set_opmode(STATION_MODE);
			wifi_station_set_config(&wifi_config);
			uart0SendStr("Connecting... \n\r");
			wifi_station_connect();
		}
		menuState = IDLE;
		string_index = 0; // start at the beginning of the string
		break;
	case 'r':
	case 'R':
		menuState = IDLE;
		DISPLAY_MENU_W_ADC();
		//system_os_post(USER_TASK_PRIO_0, 1, 1); // Display menu and ADC Value
		break;
	case 't':
	case 'T': // simple test of gpio output with microsecond timing.
		menuState = IDLE;
		system_os_post(USER_TASK_PRIO_0, 2, 0); // toggle outputs
		break;
	case 'b':
	case 'B':
		menuState = IDLE;
		DISPLAY_MENU();
		GPIO_OUTPUT_SET(13, 0);
		// Enable Voltage Boost circuit
		gpio16OutputSet(0); // Enable voltage boost circuit
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
		setSigmaDeltaDuty(((uint8) recvd - 0x30) * 256 / 10);
		menuState = IDLE;
		DISPLAY_MENU_W_SIGMA_DELTA();
		break;
	case '+':
		prescaler++;
		setSigmaDeltaPrescaler(prescaler);
//			set_sigma_delta_prescaler((unsigned char)(get_sigma_delta_prescaler()+1));
		menuState = IDLE;
		DISPLAY_MENU_W_SIGMA_DELTA();
		break;
	case '-':
		prescaler--;
		setSigmaDeltaPrescaler(prescaler);
		menuState = IDLE;
		DISPLAY_MENU_W_SIGMA_DELTA();
		break;
	case '<': // select Vt
		ADC_SELECT_TARGET();
		//Select Vpp for ADC
		menuState = IDLE;
		DISPLAY_MENU_W_ADC();
		break;
	case '>': //Select Vpp
		ADC_SELECT_VPP();
		//Select Vt for ADC
		menuState = IDLE;
		DISPLAY_MENU_W_ADC();
		break;
	default:
		DISPLAY_MENU();
		break;
	}
}


