
#ifndef __gpio_port_h__
#define __gpio_port_h__


#include <map>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>


/*
 * This is a factory for pins. Also this class aggregates GPIO port
 * common functions.
 *
 * TODO: template here is used only to support ISRs and nothing more
 */
template<uint32_t Base, uint32_t Pereph>
class gpio_port_t {
public:
	void enable(void) {
		SysCtlPeripheralEnable(Pereph);
	}

	void disable(void) {
		SysCtlPeripheralDisable(Pereph);
	}

	gpio_pin_t * get_pin(uint32_t mask) {
		return new gpio_pin_hw_t<Base>(mask);
	}
};



#endif /* __gpio_port_h__ */
