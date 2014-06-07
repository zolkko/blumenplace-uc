/*
 * gpio_pin.h
 *
 *  Created on: Jun 7, 2014
 *      Author: zolkko
 */

#ifndef __gpio_pin_h__
#define __gpio_pin_h__

template<uint32_t Base>
class gpio_pin_t {
private:
	uint32_t pin;

public:
	gpio_pin_t(const uint32_t pin_mask) : pin(pin_mask) {
	}

	void clear(void) {
		GPIOPinWrite(Base, pin, 0x00);
	}

	void set(void) {
		GPIOPinWrite(Base, pin, pin);
	}
};

#endif /* __gpio_pin_h__ */
