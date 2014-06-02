
#ifndef __gpio_intctrl_h__
#define __gpio_intctrl_h__


class gpio_listener_t {
public:
	virtual void handle_isr(uint32_t status, uint32_t value) = 0;
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
			listeners[pin].push_back(listener);
			return true;
		}
		return false;
	}

	void handle_isr(void) {
		uint32_t status = HWREG(Base + GPIO_O_MIS);
		uint32_t pin_values = HWREG(Base + (GPIO_O_DATA + (status << 2)));
		uint32_t isense = HWREG(Base + GPIO_O_IS);

		HWREG(Base + GPIO_O_ICR) = status & ~(isense);

		this->publish(status, pin_values);
	}

	inline uint32_t get_interrupt(void) const {
		switch (Base) {
			case GPIO_PORTA_BASE:
				return INT_GPIOA;

			case GPIO_PORTB_BASE:
				return INT_GPIOB;

			case GPIO_PORTC_BASE:
				return INT_GPIOC;

			case GPIO_PORTD_BASE:
				return INT_GPIOD;

			case GPIO_PORTE_BASE:
				return INT_GPIOE;
		}
	}

private:
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

	gpio_intctrl_t() {	}
	gpio_intctrl_t(const gpio_intctrl_t&) {	}
	void operator=(const gpio_intctrl_t&) {	}

private:
	gpio_listener_vector_t listeners[8];
};

#endif /* __gpio_intctrl_h__ */
