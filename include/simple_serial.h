/******************************************************************************
 * MIT open source license (Steve McNeil = Mack@PrototypeIteration.com)
 *
 * FileName: simple_serial.h
 *
 * Description: Create a simple serial interface for setting wifi configuration
 *
 * Modification history:
 *     2015/10/1, v1.0 create this file.
*******************************************************************************/


#ifndef SIMPLE_SERIAL_H_
#define SIMPLE_SERIAL_H_

#define DISPLAY_MENU() system_os_post(USER_TASK_PRIO_0,1,0)
#define DISPLAY_MENU_W_SPI() system_os_post(USER_TASK_PRIO_0, 1, 3)
#define DISPLAY_MENU_W_SIGMA_DELTA() system_os_post(USER_TASK_PRIO_0, 1, 2)
#define DISPLAY_MENU_W_ADC() system_os_post(USER_TASK_PRIO_0, 1, 1)
#define TURN_OFF_SYSTEM_MESSAGES() system_set_os_print(0)
#define TURN_ON_SYSTEM_MESSAGES() system_set_os_print(1)

void serialInit(void);
LOCAL void uart0_rx_intr_handler(void *para);
void displayConfigMenu(void);
void receiveNext(char recv);
void getNewSSID(char recv);
void getNewPassword(char recv);

#endif /* SIMPLE_SERIAL_H_ */
