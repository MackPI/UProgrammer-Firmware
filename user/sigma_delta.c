#include "driver/sigma_delta.h"
#include "eagle_soc.h"
#include "gpio.h"
#include "os_type.h"
//#include "gpio.h"

// Simplified access to sigma delta use on GPIO

//void config_sigma_delta(unsigned char pin, unsigned char target, unsigned char prescale){
void config_sigma_delta(){
    GPIO_REG_WRITE(GPIO_SIGMA_DELTA, 0x10000); // Enable Sigma Delta (1)
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12);
    GPIO_OUTPUT_SET(12, 0);
    GPIO_REG_WRITE(GPIO_PIN12,1); // connect GPIO 12 to Sigmadelta Channel
    GPIO_REG_WRITE(GPIO_SIGMA_DELTA, 0x1FF40); // Enable Sigma Delta (1)

}

void set_sigma_delta_duty(unsigned char target){
    GPIO_REG_WRITE(GPIO_SIGMA_DELTA, 0x1FF00 | target); // set Target duty cycle

}
