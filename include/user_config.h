#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

// Enable/disable voltage boost circuit
#define VOLTAGE_BOOST_ENABLE()	gpio16OutputSet(0)
#define VOLTAGE_BOOST_DISABLE()	gpio16OutputSet(1)

void setupIoPins(void);

#endif
