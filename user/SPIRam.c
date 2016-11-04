#include <SPIRam.h>
#include <driver/spi.h>
#include <driver/spi_overlap.h>
#include <driver/spi_register.h>

// Set up SPI and Overlap Mode
void initSpiRam()
{
	spi_master_init(HSPI);
	hspiOverlapInit();
	ENABLE_HSPI_DEV_CS();
}
void disableSpiRam()
{
	hspiOverlapDeinit();

}
void writeRam()
{
	hspiOverlapInit();
	while (READ_PERI_REG(SPI_CMD(HSPI)) & SPI_USR)
		;

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, 1);

	SET_PERI_REG_MASK(SPI_PIN(HSPI), SPI_CS0_DIS|SPI_CS1_DIS);
	CLEAR_PERI_REG_MASK(SPI_PIN(HSPI), SPI_CS2_DIS);
	WRITE_PERI_REG(SPI_CLOCK(HSPI),
			((3&SPI_CLKCNT_N)<<SPI_CLKCNT_N_S)| ((1&SPI_CLKCNT_H)<<SPI_CLKCNT_H_S)| ((3&SPI_CLKCNT_L)<<SPI_CLKCNT_L_S));
	//clear bit 31,set SPI clock div

//	stack[stack_index++] = READ_PERI_REG(SPI_USER(SPI));
	SET_PERI_REG_MASK(SPI_USER(HSPI),
			SPI_CS_SETUP|SPI_CS_HOLD|SPI_USR_COMMAND|SPI_USR_ADDR|SPI_USR_MOSI);
	CLEAR_PERI_REG_MASK(SPI_USER(HSPI), SPI_FLASH_MODE|SPI_USR_MISO);

//	stack[stack_index++] = READ_PERI_REG(SPI_USER1(SPI));
	WRITE_PERI_REG(SPI_USER1(HSPI),
			(((12*8-1)&SPI_USR_MOSI_BITLEN)<<SPI_USR_MOSI_BITLEN_S)| //12 Bytes of data out
			((7&SPI_USR_MISO_BITLEN)<<SPI_USR_MISO_BITLEN_S)|//Ignored for write
			((23&SPI_USR_ADDR_BITLEN)<<SPI_USR_ADDR_BITLEN_S));
	//address is 24 bits A0-A8

//	stack[stack_index++] = READ_PERI_REG(SPI_ADDR(SPI));
	WRITE_PERI_REG(SPI_ADDR(HSPI), (uint32) 0x000000<<(32-24));
	//write 24-bit address

//	stack[stack_index++] = READ_PERI_REG(SPI_USER2(SPI));
	WRITE_PERI_REG(SPI_USER2(HSPI),
			(((7&SPI_USR_COMMAND_BITLEN)<<SPI_USR_COMMAND_BITLEN_S) | 0x02));
	// Write Command

	WRITE_PERI_REG(SPI_W0(HSPI), 0x48656C6C);
	//"Hell"
	WRITE_PERI_REG(SPI_W1(HSPI), 0x6F20576F);
	//"o Wo"
	WRITE_PERI_REG(SPI_W2(HSPI), 0x726C6400);
	//"rld",NULL

	SET_PERI_REG_MASK(SPI_CMD(HSPI), SPI_USR);
	// Tell hardware to do it.
	while (READ_PERI_REG(SPI_CMD(HSPI)) & SPI_USR)
		;
	hspiOverlapDeinit();

}

void //__attribute__((section(".text")))
readRam(char data[], int length)
{
	uint32 chunk;
	hspiOverlapInit();
	while (READ_PERI_REG(SPI_CMD(HSPI)) & SPI_USR)
		;

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, 1);
	// GPIO0 -> SPICS2

//	stack[stack_index++] = READ_PERI_REG(SPI_PIN(SPI)); // save so it can be restored
	SET_PERI_REG_MASK(SPI_PIN(HSPI), SPI_CS0_DIS|SPI_CS1_DIS);
	CLEAR_PERI_REG_MASK(SPI_PIN(HSPI), SPI_CS2_DIS);
	// Enable CS2 for SPI

//	stack[stack_index++] = READ_PERI_REG(SPI_CLOCK(SPI));
	WRITE_PERI_REG(SPI_CLOCK(HSPI),
			((3&SPI_CLKCNT_N)<<SPI_CLKCNT_N_S)| ((1&SPI_CLKCNT_H)<<SPI_CLKCNT_H_S)| ((3&SPI_CLKCNT_L)<<SPI_CLKCNT_L_S));
	//clear bit 31,set SPI clock div

//	stack[stack_index++] = READ_PERI_REG(SPI_USER(HSPI));
	SET_PERI_REG_MASK(SPI_USER(HSPI),
			SPI_CS_SETUP|SPI_CS_HOLD|SPI_USR_COMMAND|SPI_USR_ADDR|SPI_USR_MISO);
	CLEAR_PERI_REG_MASK(SPI_USER(HSPI), SPI_FLASH_MODE|SPI_USR_MOSI);

//	stack[stack_index++] = READ_PERI_REG(SPI_USER1(SPI));
	WRITE_PERI_REG(SPI_USER1(HSPI),
			((7&SPI_USR_MOSI_BITLEN)<<SPI_USR_MOSI_BITLEN_S)| //Ignored for Read
			(((12*8-1)&SPI_USR_MISO_BITLEN)<<SPI_USR_MISO_BITLEN_S)|//12 bytes of data to read in
			((23&SPI_USR_ADDR_BITLEN)<<SPI_USR_ADDR_BITLEN_S)|2);
	//address is 24 bits A0-A8

//	stack[stack_index++] = READ_PERI_REG(SPI_ADDR(SPI));
	WRITE_PERI_REG(SPI_ADDR(HSPI), (uint32) 0x000000<<(32-24));
	//write 24-bit address

//	stack[stack_index++] = READ_PERI_REG(SPI_USER2(SPI));
	WRITE_PERI_REG(SPI_USER2(HSPI),
			(((7&SPI_USR_COMMAND_BITLEN)<<SPI_USR_COMMAND_BITLEN_S) | 0x03));
	// Read Command

	WRITE_PERI_REG(SPI_W0(HSPI), 0x00000000);
	//"Hell"
	WRITE_PERI_REG(SPI_W1(HSPI), 0x00000000);
	//"o Wo"
	WRITE_PERI_REG(SPI_W2(HSPI), 0x00000000);
	//"rld",NULL

	SET_PERI_REG_MASK(SPI_CMD(HSPI), SPI_USR);
	// Tell hardware to do it.
	while (READ_PERI_REG(SPI_CMD(HSPI)) & SPI_USR)
		; // wait till it's finished

	chunk = READ_PERI_REG(SPI_W0(HSPI)); //"Hell"
	data[0] = (chunk >> 24) & 0xff;
	data[1] = (chunk >> 16) & 0xff;
	data[2] = (chunk >> 8) & 0xff;
	data[3] = (chunk >> 0) & 0xff;

	chunk = READ_PERI_REG(SPI_W1(HSPI)); //"o Wo"
	data[4] = (chunk >> 24) & 0xff;
	data[5] = (chunk >> 16) & 0xff;
	data[6] = (chunk >> 8) & 0xff;
	data[7] = (chunk >> 0) & 0xff;

	chunk = READ_PERI_REG(SPI_W2(HSPI)); //"rld",NULL
	data[8] = (chunk >> 24) & 0xff;
	data[9] = (chunk >> 16) & 0xff;
	data[10] = (chunk >> 8) & 0xff;
	data[11] = (chunk >> 0) & 0xff;
	hspiOverlapDeinit();

}

