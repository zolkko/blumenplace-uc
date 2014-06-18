/*
 * ccx.c - A driver for cc1201 transceiver
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

#include "FreeRTOSConfig.h"
#include <string.h>
#include <FreeRTOS.h>
#include "ccx.h"


static const uint8_t CCx_CONFIGURATION[] = {
	GDOx_CFG_TX_THR_TX_THR_gc,	// CCx_IOCFG2
	GDOx_CFG_HI_Z,				// CCx_IOGDO1 - default 3-state
	GDOx_CFG_SYNC_WORD_gc,	    // CCx_IOCFG0D

	0x0e,						// CCx_FIFOTHR

	0xd3,						// CCx_SYNC1 - 8 MSB 16-bit sync word
	0x91,						// CCX_SYNC0 - 8 LSB 16-bit sync word

	CCx_PACKT_LEN,				// CCx_PKTLEN

	0x04,						// CCx_PKTCTRL1
	0x05,						// CCx_PKTCTRL0

	0x00,						// CCx_ADDR
	0x00,						// CCx_CHANNR

	0x08,						// CCx_FSCTRL1
	0x00,						// CCx_FSCTRL0

	0x23,						// CCx_FREQ2
	0x31,						// CCx_FREQ1
	0x3B,						// CCx_FREQ0

	0x7B,						// CCx_MDMCFG4
	0x83,						// CCx_MDMCFG3
	0x03,						// CCx_MDMCFG2
	0x22,						// CCx_MDMCFG1
	0xF8,						// CCx_MDMCFG0

	0x42,						// CCx_DEVIATN

	0x07,						// CCx_MCSM2  - 0x07 timeout for sync word search (until end of packet)
	0x30,						// CCx_MCSM1  - 0x30

	0x18,						// CCx_MCSM0

	0x1D,						// CCx_FOCCFG
	0x1C,						// CCx_BSCFG

	0xC7,						// CCx_AGCCTRL2
	0x00,						// CCx_AGCCTRL1
	0xB2,						// CCx_AGCCTRL0

	0x87,						// CCx_WOREVT1 - 0x87
	0x6b,						// CCx_WOREVT0 - 0x6b
	0xf8,						// CCx_WORCTRL - 0xf8

	0xB6,						// CCx_FREND1
	0x10,						// CCx_FREND0

	0xEA,						// CCx_FSCAL3
	0x2A,						// CCx_FSCAL2
	0x00,						// CCx_FSCAL1
	0x1F,						// CCx_FSCAL0

	0x41,						// CCx_RCCTRL1 - 0x41
	0x00,						// CCx_RCCTRL0 - 0x00

	0x59,						// CCx_FSTEST

	0x7f,						// CCx_PTEST   - 0x7f
	0x3f,						// CCx_AGCTEST - 0x3f

	0x81,						// CCx_TEST2
	0x35,						// CCx_TEST1
	0x09,						// CCx_TEST0
};


/*
 * TODO: return enum class instead of uint8_t
 */
uint8_t ccx_t::write(uint8_t addr, uint8_t data)
{
	uint32_t status = ssi.send(addr);
	ssi.send(data);
	return status;
}


uint8_t ccx_t::read(uint8_t addr, uint8_t * data)
{
	uint32_t status = ssi.send(addr | CCx_RW_BIT_bm);
	if (data != NULL) {
		*data = ssi.send(0x00);
	} else {
		ssi.send(0x00);
	}
	return status;
}


uint8_t ccx_t::burst_write(uint8_t addr, const uint8_t * data, uint8_t data_size)
{
	uint8_t out[data_size];

	ssi.send(addr | CCx_BURST_BIT_bm);
	if (ssi.transceive((void *)data, data_size, (void *) out)) {
		return 1;
	} else {
		return 0;
	}
}

void ccx_t::wait_ready(void) {
	while (!gdo1.is_set()) {
	}
}

/*
 * TODO: module locking and unlocking
 */
uint8_t ccx_t::version(void)
{
	ssi.chip_select();
	//wait_ready();

	uint8_t version = 0;
	read(CCx_VERSION, &version);

	ssi.chip_release();

	return version;
}


uint8_t ccx_t::part_number(void)
{
	ssi.chip_select();
	// wait_ready();

	uint8_t part_num = 0;
	read(CCx_PARTNUM, &part_num);

	ssi.wait_ready();
	ssi.chip_release();

	return part_num;
}


void ccx_t::configure(void)
{
	uint8_t obuf[sizeof(CCx_CONFIGURATION)];
	memcpy(obuf, CCx_CONFIGURATION, sizeof(CCx_CONFIGURATION));

	ssi.chip_select();
	wait_ready();
	burst_write(CCx_REG_BEGIN, CCx_CONFIGURATION, sizeof(CCx_CONFIGURATION));
	ssi.chip_release();
}
