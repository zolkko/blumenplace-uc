
#ifndef __ccx_hpp__
#define __ccx_hpp__

#include <stdint.h>
#include <stdbool.h>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <inc/hw_gpio.h>
#include <inc/hw_timer.h>

#include "inc/hw_memmap.h"
#include "inc/hw_ssi.h"

#include <vector>

class gpio_listener_t {
protected:
	virtual void handle_isr(uint32_t status, uint32_t value) = 0;

public:
	virtual uint8_t get_pin(void) = 0;
};


typedef std::vector<gpio_listener_t*> gpio_listener_vector_t;


template<uint32_t Base>
class gpio_intctrl_t {
public:
	static gpio_intctrl_t& get(void) {
		static gpio_intctrl_t gpio_intctrl;
		return gpio_intctrl;
	}

	bool subscribe(gpio_listener_t * listener) {
		uint8_t pin = listener->get_pin();
		if (pin < 8) {
			listener[pin].push_back(listener);
			return true;
		}
		return false;
	}

	void publish(uint32_t status, uint32_t value) {
		uint8_t mask = 0x80;
		for (uint8_t i = 7; i > 0; i--) {
			if (status & mask) {
				for (gpio_listener_vector_t::iterator it = listeners[i].begin(); it != listeners[i].end(); ++it) {
					(*it)->handle_isr(status, value);
				}
			}
			mask >>= 1;
		}
	}

private:
	gpio_listener_vector_t listeners[8];

	gpio_intctrl_t() {
	}

	gpio_intctrl_t(const gpio_intctrl_t&) {
	}

	void operator=(const gpio_intctrl_t&) {
	}
};


/*template<uint32_t Base>
class gpio_pin_t : public gpio_listener_t {
private:
	uint8_t pin;

protected:
	virtual void handle_isr(uint32_t status, uint32_t value) {
		printf("handle_irs::%d, %d", status, value);
	}

public:
	gpio_pin_t(uint8_t apin) : pin(apin) {
		gpio_intctrl_t<Base>::get().subscribe(this);
	}

	virtual uint8_t get_pin(void) {
		return pin;
	}
};*/


#endif /* __ccx_hpp__ */
