#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

// Enable/disable voltage boost circuit
#define VOLTAGE_BOOST_ENABLE()	gpio16_output_set(0)
#define VOLTAGE_BOOST_DISABLE()	gpio16_output_set(1)

void setup_io_pins(void);

#endif
