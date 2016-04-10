/*
 * SPIRam.c
 *
 *  Created on: Apr 9, 2016
 *      Author: esp8266
 */
#include <SPIRam.h>
#include <driver/spi.h>

 // Set up SPI and Overlap Mode
void initSpiRam(){
	ACTIVE_HSPI_DEV_CS();
	HSPI_FALLING_EDGE_SAMPLE();
	hspi_overlap_init();
	spi_master_init(HSPI);


}
// Shut down Overlap Mode
void disableSpiRam(){
	hspi_overlap_deinit();

}
void writeRam(char data[], int length){
	int index;
	spi_mast_byte_write(HSPI,0x02); // Write Command
	spi_mast_byte_write(HSPI,0x00); // Address of 0
	spi_mast_byte_write(HSPI,0x00); //
	spi_mast_byte_write(HSPI,0x00); //
	for (index = 0;index <length;index++){
		spi_mast_byte_write(HSPI,data[index]); //
	}


}
void readRam(char data[], int length){
	int index;
	spi_mast_byte_write(HSPI,0x03); // Write Command
	spi_mast_byte_write(HSPI,0x00); // Address of 0
	spi_mast_byte_write(HSPI,0x00); //
	spi_mast_byte_write(HSPI,0x00); //
	for (index = 0;index <length;index++){
		spi_mast_byte_read(HSPI,data[index]); //
	}

}


