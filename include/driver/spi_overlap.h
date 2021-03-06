#ifndef SPI_OVERLAP_APP_H
#define SPI_OVERLAP_APP_H

#include "ets_sys.h"
#include "spi_flash.h"
#define HSPI_OVERLAP
//#define NO_HSPI_DEVICE
#define HOST_INF_SEL 0x3ff00028
#define FUNC_SPI_CS2 1
#define FUNC_SPI_CS1 1
#define reg_cspi_overlap  (BIT7)

#define SPI_FLASH_BYTES_LEN                24
#define IODATA_START_ADDR                 BIT0
#define SPI_BUFF_BYTE_NUM                    32

#define PERIPHS_IO_MUX_BACKUP		0
#define SPI_USER_BACKUP  	1
#define SPI_CTRL_BACKUP  	2
#define SPI_CLOCK_BACKUP 	3
#define SPI_USER1_BACKUP	4
#define SPI_USER2_BACKUP	5
#define SPI_CMD_BACKUP		6
#define SPI_PIN_BACKUP		7
#define SPI_SLAVE_BACKUP	8

#define HSPI_CS_DEV			0
#define SPI_CS1_DEV			1
#define SPI_CS2_DEV			2
#define SPI_CS0_FLASH		3
#define HSPI_IDLE			4

#define SPI_FLASH_READ_MODE_MASK 0x196000
#define WAIT_HSPI_IDLE() 	while(READ_PERI_REG(SPI_EXT2(HSPI))||(READ_PERI_REG(SPI_CMD(HSPI))&0xfffc0000));
#define CONF_HSPI_CLK_DIV(div)	WRITE_PERI_REG(SPI_CLOCK(HSPI), (((div<<1)+1)<<12)+(div<<6)+(div<<1)+1)
#define HSPI_FALLING_EDGE_SAMPLE()		SET_PERI_REG_MASK(SPI_USER(HSPI),  SPI_CK_OUT_EDGE)
#define HSPI_RISING_EDGE_SAMPLE()			CLEAR_PERI_REG_MASK(SPI_USER(HSPI),  SPI_CK_OUT_EDGE)
#define ACTIVE_HSPI_CS0	 	CLEAR_PERI_REG_MASK(SPI_PIN(HSPI), SPI_CS0_DIS);\
						SET_PERI_REG_MASK(SPI_PIN(HSPI), SPI_CS1_DIS |SPI_CS2_DIS)
#define ACTIVE_HSPI_CS1		CLEAR_PERI_REG_MASK(SPI_PIN(HSPI), SPI_CS1_DIS);\
						SET_PERI_REG_MASK(SPI_PIN(HSPI), SPI_CS0_DIS |SPI_CS2_DIS)
#define ACTIVE_HSPI_CS2		CLEAR_PERI_REG_MASK(SPI_PIN(HSPI), SPI_CS2_DIS);\
						SET_PERI_REG_MASK(SPI_PIN(HSPI), SPI_CS0_DIS |SPI_CS1_DIS)
#define ENABLE_HSPI_DEV_CS()		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, 2)
#define DISABLE_HSPI_DEV_CS()		GPIO_OUTPUT_SET(15, 1);\
									PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15)

struct hspi_device_config
{
	uint8 active :1;
	uint8 clk_polar :1;
	uint8 res :1;
	uint8 clk_div :5;
};

struct hspi_device_register
{
	uint32 hspi_flash_reg_backup[9];
	uint32 hspi_dev_reg_backup[9];
	struct hspi_device_config hspi_dev_conf[4];
	uint8 selected_dev_num :3;
	uint8 spi_io_80m :1;
	uint8 hspi_reg_backup_flag :1;
	uint8 res :3;
};

void hspiOverlapInit(void);
void hspiOverlapDeinit(void);
void spiRegRecover(uint8 spi_no, uint32* backup_mem);
void spiRegBackup(uint8 spi_no, uint32* backup_mem);

void hspiMasterDevInit(uint8 dev_no, uint8 clk_polar, uint8 clk_div);
void hspiDeviceSel(uint8 dev_no);

void hspiOverlapFlashInit(void);
SpiFlashOpResult hspiOverlapReadFlashData(SpiFlashChip * spi,
		uint32 flash_addr, uint32 * addr_dest, uint32 byte_length);

#endif
