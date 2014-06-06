#include <stdint.h>
#include <stdbool.h>

#include <vector>
#include <algorithm>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>

#include <driverlib/pin_map.h>
#include <driverlib/sysctl.h>
#include <driverlib/interrupt.h>
#include <driverlib/udma.h>

#include "FreeRTOSConfig.h"
#include <FreeRTOS.h>

#include "dma_dev.h"


#ifdef ccs
#pragma DATA_ALIGN(UDMA_CTL_TBL, 1024)
static uint8_t CTL_TBL[1024];
#else
static uint8_t CTL_TBL[1024] __attribute__ ((aligned(1024)));
#endif


void udma_error_isr_handler(void)
{
	dma_dev_t::get().handle_error_isr();
}


void dma_dev_t::handle_error_isr(void)
{
	uint32_t status = uDMAIntStatus();
	uDMAIntClear(status);

	uint32_t error = uDMAErrorStatusGet();
	if (error) {
		uDMAErrorStatusClear();
	}

	portENTER_CRITICAL();
	for (std::vector<dma_listener_t*>::iterator it = error_listeners.begin(); it != error_listeners.end(); ++it) {
		(*it)->handle_error_isr(status);
	}
	portEXIT_CRITICAL();
}


dma_dev_t::dma_dev_t()
{
	IntDisable(INT_UDMAERR);

	// initialize uDMA controller
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);

	uDMAEnable();
	uDMAControlBaseSet(CTL_TBL);

	// initialize SSI0 channel
	uDMAChannelAttributeDisable(UDMA_CHANNEL_SSI0RX, UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST | UDMA_ATTR_HIGH_PRIORITY | UDMA_ATTR_REQMASK);
	uDMAChannelAttributeDisable(UDMA_CHANNEL_SSI0TX, UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST | UDMA_ATTR_HIGH_PRIORITY | UDMA_ATTR_REQMASK);

	IntEnable(INT_UDMAERR);
}

bool dma_dev_t::unsubscribe_error(dma_listener_t * listener)
{
	portENTER_CRITICAL();
	std::vector<dma_listener_t*>::iterator it = std::find(error_listeners.begin(), error_listeners.end(), listener);
	if (it != error_listeners.end()) {
		error_listeners.erase(it);
		portEXIT_CRITICAL();
		return true;
	}
	portEXIT_CRITICAL();
	return false;
}


void dma_dev_t::subscribe_error(dma_listener_t * listener)
{
	portENTER_CRITICAL();
	error_listeners.push_back(listener);
	portEXIT_CRITICAL();
}
