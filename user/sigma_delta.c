#include "driver/sigma_delta.h"
#include "eagle_soc.h"
#include "gpio.h"
#include "os_type.h"

//void config_sigma_delta(unsigned char pin, unsigned char target, unsigned char prescale){
void configSigmaDelta()
{
	GPIO_REG_WRITE(GPIO_SIGMA_DELTA, 0x10000);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
	GPIO_OUTPUT_SET(4, 1);
	GPIO_REG_WRITE(GPIO_PIN4, 1);
	// connect GPIO 4 to Sigmadelta Channel
	GPIO_REG_WRITE(GPIO_SIGMA_DELTA, 0x10100);

}

unsigned char getSigmaDeltaPrescaler(void)
{
	return (unsigned char) ((GPIO_REG_READ(GPIO_SIGMA_DELTA) >> 8) & 0x0ff); // read Target prescaler
}
unsigned char getSigmaDeltaDuty(void)
{
	return (unsigned char) (GPIO_REG_READ(GPIO_SIGMA_DELTA) & 0x0ff); // set Target duty cycle
}

void setSigmaDeltaDuty(unsigned char duty)
{
	uint8 prescaler;
	prescaler = getSigmaDeltaPrescaler();
	if (GPIO_REG_READ(GPIO_SIGMA_DELTA) & 0x10000)
	{ // check to see if ΣΔ is running
		GPIO_REG_WRITE(GPIO_SIGMA_DELTA, 0);
		// turn off ΣΔ
		GPIO_REG_WRITE(GPIO_SIGMA_DELTA, 0x10000 | (prescaler<<8) | duty);
		// set Target Prescaler and restore duty
	}
	else
	{
		GPIO_REG_WRITE(GPIO_SIGMA_DELTA, (prescaler<<8)|duty);
		// set Target Prescaler and restore duty but leave turned off
	}

}

void setSigmaDeltaPrescaler(unsigned char prescaler)
{
	uint8 duty;
	duty = getSigmaDeltaDuty();
	if (GPIO_REG_READ(GPIO_SIGMA_DELTA) & 0x10000)
	{ // check to see if ΣΔ is running
		GPIO_REG_WRITE(GPIO_SIGMA_DELTA, 0);
		// turn off ΣΔ
		GPIO_REG_WRITE(GPIO_SIGMA_DELTA, 0x10000 | (prescaler<<8) | duty);
		// set Target Prescaler and restore duty
	}
	else
	{
		GPIO_REG_WRITE(GPIO_SIGMA_DELTA, (prescaler<<8)|duty);
		// set Target Prescaler and restore duty but leave turned off
	}

}
