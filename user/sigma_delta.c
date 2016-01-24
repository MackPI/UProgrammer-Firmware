#include "driver/sigma_delta.h"
#include "eagle_soc.h"
#include "gpio.h"
#include "os_type.h"
//#include "gpio.h"

// Simplified access to sigma delta use on GPIO

//void config_sigma_delta(unsigned char pin, unsigned char target, unsigned char prescale){
void config_sigma_delta(){
    GPIO_REG_WRITE(GPIO_SIGMA_DELTA, 0x10000); // Enable Sigma Delta (1)
//    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12);
//    GPIO_OUTPUT_SET(12, 0);
//    GPIO_REG_WRITE(GPIO_PIN12,1); // connect GPIO 12 to Sigmadelta Channel
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U,FUNC_GPIO4);
    GPIO_OUTPUT_SET(4, 1);
    GPIO_REG_WRITE(GPIO_PIN4,1); // connect GPIO 4 to Sigmadelta Channel
    GPIO_REG_WRITE(GPIO_SIGMA_DELTA, 0x10100); // Enable Sigma Delta (1)

}

void set_sigma_delta_duty(unsigned char target){
    GPIO_REG_WRITE(GPIO_SIGMA_DELTA, (GPIO_REG_READ(GPIO_SIGMA_DELTA) & 0x1ff00) | target); // set Target duty cycle

}
unsigned char get_sigma_delta_duty(void){
    return (unsigned char)(GPIO_REG_READ(GPIO_SIGMA_DELTA) & 0x0ff); // set Target duty cycle
}
void set_sigma_delta_prescaler(unsigned char target){
    GPIO_REG_WRITE(GPIO_SIGMA_DELTA, (GPIO_REG_READ(GPIO_SIGMA_DELTA) & 0x100ff) | (target<<8)); // set Target Prescaler

}
unsigned char get_sigma_delta_prescaler(void){
    return (unsigned char)((GPIO_REG_READ(GPIO_SIGMA_DELTA)>>8) & 0x0ff); // read Target prescaler
}
