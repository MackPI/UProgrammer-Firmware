/*
 * SPIRam.h
 *
 *  Created on: Apr 9, 2016
 *      Author: esp8266
 */

#ifndef SPIRAM_H_
#define SPIRAM_H_

void initSpiRam(); // Set up SPI and Overlap Mode
void disableSpiRam(); // Shut down Overlap Mode
//void writeRam(char*, int length);
void writeRam();
void readRam(char*, int Length);



#endif /* SPIRAM_H_ */
