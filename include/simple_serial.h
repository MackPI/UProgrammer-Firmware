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

void serial_init(void);
LOCAL void ICACHE_FLASH_ATTR ///////
uart_recvTask(os_event_t *);
void display_config_menu(void);

#endif /* SIMPLE_SERIAL_H_ */
