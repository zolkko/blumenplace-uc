
#ifndef __gpio_port_h__
#define __gpio_port_h__

/*
 * This is a factory for pins. Also this class aggregates GPIO port
 * common functions
 */
template<uint32_t Base>
class gpio_port_t {
private:
	gpio_port_t() {}
	gpio_port_t(const gpio_port_t& gpio_port_) {}
	void operator=(const gpio_port_t& gpio_port_) {}

public:
	static gpio_port_t& get(void) {
		static gpio_port_t gpio_port;
		return gpio_port;
	}

	gpio_pin_t<Base> get_pin(const uint32_t pin_mask) {
		gpio_pin_t<Base> pin(pin_mask);
		return pin;
	}
};


#endif /* __gpio_port_h__ */
