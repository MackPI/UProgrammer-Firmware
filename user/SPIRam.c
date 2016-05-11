/*
 * SPIRam.c
 *
 *  Created on: Apr 9, 2016
 *      Author: esp8266
 */
#include <SPIRam.h>
#include <driver/spi.h>
#include <driver/spi_overlap.h>
#include <driver/spi_register.h>

 // Set up SPI and Overlap Mode
void initSpiRam(){
	spi_master_init(HSPI);
	hspi_overlap_init();
	ENABLE_HSPI_DEV_CS();
//	HSPI_FALLING_EDGE_SAMPLE();


}
// Shut down Overlap Mode
void disableSpiRam(){
	hspi_overlap_deinit();

}
//void writeRam(char data[], int length){
void writeRam(){
//	uint32 stack[7];
//	uint8 stack_index= 0;
	while(READ_PERI_REG(SPI_CMD(HSPI))&SPI_USR);

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, 1);// GPIO0 -> SPICS2

//	stack[stack_index++] = READ_PERI_REG(SPI_PIN(SPI)); // save so it can be restored
	SET_PERI_REG_MASK(SPI_PIN(HSPI), SPI_CS0_DIS|SPI_CS1_DIS);
	CLEAR_PERI_REG_MASK(SPI_PIN(HSPI), SPI_CS2_DIS ); // Enable CS2 for HSPI in overlap mode

//	stack[stack_index++] = READ_PERI_REG(SPI_CLOCK(SPI));
	WRITE_PERI_REG(SPI_CLOCK(HSPI),
					((3&SPI_CLKCNT_N)<<SPI_CLKCNT_N_S)|
					((1&SPI_CLKCNT_H)<<SPI_CLKCNT_H_S)|
					((3&SPI_CLKCNT_L)<<SPI_CLKCNT_L_S)); //clear bit 31,set SPI clock div

//	stack[stack_index++] = READ_PERI_REG(SPI_USER(SPI));
	SET_PERI_REG_MASK(SPI_USER(HSPI),
			SPI_CS_SETUP|SPI_CS_HOLD|SPI_USR_COMMAND|SPI_USR_ADDR|SPI_USR_MOSI);
	CLEAR_PERI_REG_MASK(SPI_USER(HSPI), SPI_FLASH_MODE|SPI_USR_MISO);

//	stack[stack_index++] = READ_PERI_REG(SPI_USER1(SPI));
	WRITE_PERI_REG(SPI_USER1(HSPI), (((12*8-1)&SPI_USR_MOSI_BITLEN)<<SPI_USR_MOSI_BITLEN_S)| //12 Bytes of data out
	((7&SPI_USR_MISO_BITLEN)<<SPI_USR_MISO_BITLEN_S)| //Ignored for write
	((23&SPI_USR_ADDR_BITLEN)<<SPI_USR_ADDR_BITLEN_S)); //address is 24 bits A0-A8

//	stack[stack_index++] = READ_PERI_REG(SPI_ADDR(SPI));
	WRITE_PERI_REG(SPI_ADDR(HSPI), (uint32) 0x000000<<(32-24)); //write 24-bit address

//	stack[stack_index++] = READ_PERI_REG(SPI_USER2(SPI));
	WRITE_PERI_REG(SPI_USER2(HSPI), (((7&SPI_USR_COMMAND_BITLEN)<<SPI_USR_COMMAND_BITLEN_S) | 0x02)); // Write Command

	WRITE_PERI_REG(SPI_W0(HSPI),0x48656C6C); //"Hell"
	WRITE_PERI_REG(SPI_W1(HSPI),0x6F20576F); //"o Wo"
	WRITE_PERI_REG(SPI_W2(HSPI),0x726C6400); //"rld",NULL

	SET_PERI_REG_MASK(SPI_CMD(HSPI), SPI_USR);// Tell hardware to do it.
	while(READ_PERI_REG(SPI_CMD(HSPI))&SPI_USR);

//	WRITE_PERI_REG(SPI_USER2(SPI),stack[--stack_index]);
//	WRITE_PERI_REG(SPI_ADDR(SPI),stack[--stack_index]);
//	WRITE_PERI_REG(SPI_USER1(SPI),stack[--stack_index]);
//	WRITE_PERI_REG(SPI_USER(SPI),stack[--stack_index]);
//	WRITE_PERI_REG(SPI_CLOCK(SPI),stack[--stack_index]);
//	WRITE_PERI_REG(SPI_PIN(SPI),stack[--stack_index]);
}

void //__attribute__((section(".text")))
readRam(char data[], int length){
//	ETS_INTR_LOCK();
//	uint32 stack[7];
//	uint8 stack_index= 0;
	while(READ_PERI_REG(SPI_CMD(HSPI))&SPI_USR);

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, 1);// GPIO0 -> SPICS2

//	stack[stack_index++] = READ_PERI_REG(SPI_PIN(SPI)); // save so it can be restored
	SET_PERI_REG_MASK(SPI_PIN(HSPI), SPI_CS0_DIS|SPI_CS1_DIS);
	CLEAR_PERI_REG_MASK(SPI_PIN(HSPI), SPI_CS2_DIS ); // Enable CS2 for SPI

//	stack[stack_index++] = READ_PERI_REG(SPI_CLOCK(SPI));
	WRITE_PERI_REG(SPI_CLOCK(HSPI),
					((3&SPI_CLKCNT_N)<<SPI_CLKCNT_N_S)|
					((1&SPI_CLKCNT_H)<<SPI_CLKCNT_H_S)|
					((3&SPI_CLKCNT_L)<<SPI_CLKCNT_L_S)); //clear bit 31,set SPI clock div

//	stack[stack_index++] = READ_PERI_REG(SPI_USER(HSPI));
	SET_PERI_REG_MASK(SPI_USER(HSPI),
			SPI_CS_SETUP|SPI_CS_HOLD|SPI_USR_COMMAND|SPI_USR_ADDR|SPI_USR_MISO);
	CLEAR_PERI_REG_MASK(SPI_USER(HSPI), SPI_FLASH_MODE|SPI_USR_MOSI);

//	stack[stack_index++] = READ_PERI_REG(SPI_USER1(SPI));
	WRITE_PERI_REG(SPI_USER1(HSPI), ((7&SPI_USR_MOSI_BITLEN)<<SPI_USR_MOSI_BITLEN_S)| //Ignored for Read
	(((12*8-1)&SPI_USR_MISO_BITLEN)<<SPI_USR_MISO_BITLEN_S)| //12 bytes of data to read in
	((23&SPI_USR_ADDR_BITLEN)<<SPI_USR_ADDR_BITLEN_S)|2); //address is 24 bits A0-A8

//	stack[stack_index++] = READ_PERI_REG(SPI_ADDR(SPI));
	WRITE_PERI_REG(SPI_ADDR(HSPI), (uint32) 0x000000<<(32-24)); //write 24-bit address

//	stack[stack_index++] = READ_PERI_REG(SPI_USER2(SPI));
	WRITE_PERI_REG(SPI_USER2(HSPI), (((7&SPI_USR_COMMAND_BITLEN)<<SPI_USR_COMMAND_BITLEN_S) | 0x03)); // Write Command

	WRITE_PERI_REG(SPI_W0(HSPI),0x00000000); //"Hell"
	WRITE_PERI_REG(SPI_W1(HSPI),0x00000000); //"o Wo"
	WRITE_PERI_REG(SPI_W2(HSPI),0x00000000); //"rld",NULL

	SET_PERI_REG_MASK(SPI_CMD(HSPI), SPI_USR);// Tell hardware to do it.
	while(READ_PERI_REG(SPI_CMD(HSPI))&SPI_USR); // wait till it's finished

//	WRITE_PERI_REG(SPI_USER2(SPI),stack[--stack_index]);
//	WRITE_PERI_REG(SPI_ADDR(SPI),stack[--stack_index]);
//	WRITE_PERI_REG(SPI_USER1(SPI),stack[--stack_index]);
//	WRITE_PERI_REG(SPI_USER(SPI),stack[--stack_index]);
//	WRITE_PERI_REG(SPI_CLOCK(SPI),stack[--stack_index]);
//	WRITE_PERI_REG(SPI_PIN(SPI),stack[--stack_index]);
//	ETS_INTR_UNLOCK();

}


