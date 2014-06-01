/*
 * dma_svc_hw.h - dma_svc implementation for TM4C123G
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
#include <inc/hw_ssi.h>

#include <driverlib/pin_map.h>
#include <driverlib/sysctl.h>
#include <driverlib/interrupt.h>
#include <driverlib/udma.h>
#include <driverlib/ssi.h>

#include "dma_svc.h"


#define DMA_MAX_SVC_COUNT	0x01

/*
 * There is only one DMA controller in TM4C123G part.
 */
dma_svc_t dma_svc0;


#ifdef ccs
#pragma DATA_ALIGN(UDMA_CTL_TBL, 1024)
static uint8_t CTL_TBL[1024];
#else
static uint8_t CTL_TBL[1024] __attribute__ ((aligned(1024)));
#endif


void udma_error_isr_handler(void)
{
	uint32_t udma_error = uDMAErrorStatusGet();
	uint32_t udma_status = uDMAIntStatus();

	uDMAIntClear(udma_status);
	if (udma_error) {
		uDMAErrorStatusClear();
	}

	uint32_t i;
	for (i = 0; i < dma_svc0.error_handler_len; i++) {
		dma_svc0.error_handlers[i].handler(udma_status, dma_svc0.error_handlers[i].data);
	}
}


void dma_svc_init(dma_svc_t * svc)
{
	IntDisable(INT_UDMAERR);

	// initialize svc structure
	svc->error_handler_len = 0;

	// initialize uDMA controller
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);

	uDMAEnable();
	uDMAControlBaseSet(CTL_TBL);

	// initialize SSI0 channel
	uDMAChannelAttributeDisable(UDMA_CHANNEL_SSI0RX, UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST | UDMA_ATTR_HIGH_PRIORITY | UDMA_ATTR_REQMASK);
	uDMAChannelAttributeDisable(UDMA_CHANNEL_SSI0TX, UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST | UDMA_ATTR_HIGH_PRIORITY | UDMA_ATTR_REQMASK);

	IntEnable(INT_UDMAERR);
}


bool dma_svc_register_error_handler(dma_svc_t * svc, dma_error_handler_t error_handler, void * data)
{
	if (error_handler && svc->error_handler_len < DMA_SVC_MAX_ERROR_HANDLERS) {
		IntDisable(INT_UDMAERR);

		svc->error_handlers[svc->error_handler_len].handler = error_handler;
		svc->error_handlers[svc->error_handler_len].data = data;
		svc->error_handler_len++;

		IntEnable(INT_UDMAERR);
		return true;
	} else {
		return false;
	}
}


void dma_svc_ssi0_rx(dma_svc_t * svc, void * data, uint32_t item_count)
{
	uDMAChannelControlSet(UDMA_CHANNEL_SSI0RX | UDMA_PRI_SELECT, UDMA_SIZE_8 | UDMA_SRC_INC_NONE | UDMA_DST_INC_8 | UDMA_ARB_1);
	uDMAChannelTransferSet(UDMA_CHANNEL_SSI0RX | UDMA_PRI_SELECT, UDMA_MODE_BASIC, (void *)(SSI0_BASE + SSI_O_DR), data, item_count);
}


void dma_svc_ssi0_tx(dma_svc_t * svc, void * data, uint32_t item_count)
{
	uDMAChannelControlSet(UDMA_CHANNEL_SSI0TX | UDMA_PRI_SELECT, UDMA_SIZE_8 | UDMA_SRC_INC_8 | UDMA_DST_INC_NONE | UDMA_ARB_1);
	uDMAChannelTransferSet(UDMA_CHANNEL_SSI0TX | UDMA_PRI_SELECT, UDMA_MODE_BASIC, data, (void *)(SSI0_BASE + SSI_O_DR), item_count);
}


void dma_svc_ssi0_transceive(dma_svc_t * svc)
{
	uDMAChannelEnable(UDMA_CHANNEL_SSI0RX);
	uDMAChannelEnable(UDMA_CHANNEL_SSI0TX);
}


bool dma_svc_ssi0_busy(dma_svc_t * svc)
{
	return uDMAChannelIsEnabled(UDMA_CHANNEL_SSI0TX);
}
