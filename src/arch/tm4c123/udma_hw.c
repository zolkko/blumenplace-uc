/*
 * udma_hw.h - An abstract prototype for uDMA module
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

#include <stdint.h>
#include <stdbool.h>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>

#include <driverlib/interrupt.h>
#include <driverlib/udma.h>

#include "FreeRTOSConfig.h"
#include "list.h"


#ifdef ccs
#pragma DATA_ALIGN(UDMA_CTL_TBL, 1024)
static uint8_t UDMA_CTL_TBL[1024];
#else
static uint8_t UDMA_CTL_TBL[1024] __attribute__ ((aligned(1024)));
#endif


void udma_error_isr_handler(void)
{
	uint32_t udma_error = uDMAErrorStatusGet();
	uint32_t udma_status = uDMAIntStatus();
	uDMAIntClear(udma_status);

	if (udma_error) {
		uDMAErrorStatusClear();
	}

	// TODO: call handler is any
}


void udma_software_isr_handler(void)
{
	uint32_t udma_status = uDMAIntStatus();
	uDMAIntClear(udma_status);
	// TODO: call handler if any
}


void udma_init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);

	uDMAEnable();
	uDMAControlBaseSet(UDMA_CTL_TBL);
}


void udma_timer0a_channel_init(void)
{
	uDMAChannelAttributeEnable(UDMA_CHANNEL_TMR0A, UDMA_ATTR_USEBURST);
	uDMAChannelAttributeDisable(UDMA_CHANNEL_TMR0A, UDMA_ATTR_ALTSELECT | UDMA_ATTR_HIGH_PRIORITY | UDMA_ATTR_REQMASK);
}


bool udma_register_software_isr_handler(udma_isr_handler handler)
{
	IntEnable(INT_UDMA);
}


bool udma_register_error_isr_handler(udma_isr_handler)
{
	IntEnable(INT_UDMAERR);
}
