#ifndef SPI_OVERLAP_APP_H
#define SPI_OVERLAP_APP_H

#include "os_type.h"
#define GPIO_SIGMA_DELTA	0x68  //defined in gpio register.xls
#define GPIO_PIN12 			0x58
#define GPIO_PIN4			0x38

//void config_sigma_delta(unsigned char pin, unsigned char target, unsigned char prescale);
void config_sigma_delta();

#endif
