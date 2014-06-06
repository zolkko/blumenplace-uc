
#ifndef __ccx_dev_h__
#define __ccx_dev_h__

/*
class ccx_hw_t {
private:
	gpio_pin_t cs_pin;

	gpio_pin_t gdo0_pin;
	gpio_pin_t gdo1_pin;
	gpio_pin_t gdo2_pin;

	ssi_t ssi;

public:
	ccx_hw_t(const gpio_pin_t &acs_pin) : cs_pin(acs_pin) {}

	void chip_select(void) {
		this->cs_pin.clear();
	}

	void chip_release(void) {
		this->cs_pin.clear();
	}

	uint8_t write(uint8_t data) {
		return this->ssi.send(data);
	}

	bool ready(void);

	void wait_ready(void);

	bool gdo0(void);
	void enable_gdo0(void);
	void disable_gdo0(void);

	bool wait_gdo0(portTickType timeout);

	bool gdo1(void);
	void enable_gdo1(void);
	void disable_gdo1(void);
	bool wait_gdo1(portTickType timeout);

	bool gdo2(void);
	void enable_gdo2(void);
	void disable_gdo2(void);
	bool wait_gdo2(portTickType timeout);
};
*/

#endif /* __ccx_dev_h__ */
