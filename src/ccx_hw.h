
#ifndef __ccx_dev_h__
#define __ccx_dev_h__


class ccx_hw_t {
public:
	ccx_hw_t() {}

	void chip_select(void);
	uint8_t write(uint8_t data);
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

	void chip_release(void);
};


#endif /* __ccx_dev_h__ */
