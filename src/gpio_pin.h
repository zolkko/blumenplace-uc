/*
 * gpio_pin.h
 *
 *  Created on: Jun 7, 2014
 *      Author: zolkko
 */

#ifndef __gpio_pin_h__
#define __gpio_pin_h__


/*
 * abstract GPIO pin type
 */
class gpio_pin_t {
public:
	virtual void clear(void) = 0;
	virtual void set(void) = 0;
	virtual uint32_t get(void) = 0;
	virtual bool is_set(void) = 0;
	virtual void set_mode_input(void) = 0;
};


template<uint32_t Base>
class gpio_pin_hw_t : public gpio_pin_t {
public:
	gpio_pin_hw_t(const uint32_t pin_mask) : pin(pin_mask) {
	}

	virtual void set_mode_input(void) {
		GPIOPinTypeGPIOInput(Base, pin);
	}

	virtual void clear(void) {
		GPIOPinWrite(Base, pin, 0x00);
	}

	virtual void set(void) {
		GPIOPinWrite(Base, pin, pin);
	}

	virtual uint32_t get(void) {
		return GPIOPinRead(Base, pin);
	}

	virtual bool is_set(void) {
		return (this->get() & this->pin) == this->pin;
	}

private:
	uint32_t pin;

};

#endif /* __gpio_pin_h__ */
