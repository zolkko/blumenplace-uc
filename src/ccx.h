/*
 * ccx.h - A header file for cc1201 and cc1101
 *
 * Copyright (c) 2014 Alexey Anisimov <zolkko@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef __ccx_h__
#define __ccx_h__

#include <stdint.h>
#include <stdbool.h>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <inc/hw_udma.h>
#include <inc/hw_ssi.h>

#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <driverlib/ssi.h>

#include "gpio_pin.h"
#include "ssi_dev.h"

#include "cc120x.h"


class ccx_t {
public:
	ccx_t(ssi_t& ssi_, gpio_pin_t& gdo0_, gpio_pin_t& gdo1_, gpio_pin_t& gdo2_) :
		ssi(ssi_),
		gdo0(gdo0_),
		gdo1(gdo1_),
		gdo2(gdo2_)
	{
	}

	bool reset(void) {
		uint32_t timeout;
		bool result = true;

		ssi.chip_select();
		delay_us(1);
		ssi.chip_release();

		delay_us(50);

//		timeout = 0;
//		while (!gdo1.is_set() && timeout < 0xffff) {
//			timeout++;
//		}

		ssi.chip_select();

		timeout = 0;
		while (gdo1.is_set() && timeout < 0xffff) {
			timeout++;
			if (timeout == 0xffff) {
				result = false;
			}
		}

		ssi.send(CCx_SRES);
		ssi.wait_ready();

		timeout = 0;
		while (gdo1.is_set() && timeout < 0xffff) {
			timeout++;
			if (timeout == 0xffff) {
				result = false;
			}
		}

		ssi.chip_release();

		return result;
	}

	/*
	 * write configuration into IC
	 */
	void configure(void);

	/*
	 * return ICs version
	 */
	uint8_t version(void);

	/*
	 * return part number
	 */
	uint8_t part_number(void);

private:
	void delay_us(uint32_t delay) {
		SysCtlDelay(delay);
	}

private:
	void start_transaction(void);
	void finish_transaction(void);

	uint8_t write(uint8_t addr, uint8_t data);
	uint8_t read(uint8_t addr, uint8_t * data);
	uint8_t burst_write(uint8_t addr, const uint8_t * data, uint8_t data_size);
	uint8_t ext_reg_read(uint8_t ext_reg_addr);

	void wait_ready(void);

private:
	ssi_t& ssi;
	gpio_pin_t& gdo0;
	gpio_pin_t& gdo1;
	gpio_pin_t& gdo2;
};


#endif /* __ccx_h__ */
