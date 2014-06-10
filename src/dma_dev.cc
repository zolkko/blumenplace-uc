#include <stdint.h>
#include <stdbool.h>

#include <vector>
#include <algorithm>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <inc/hw_udma.h>

#include <driverlib/pin_map.h>
#include <driverlib/sysctl.h>
#include <driverlib/interrupt.h>
#include <driverlib/udma.h>

#include "FreeRTOSConfig.h"
#include <FreeRTOS.h>

#include "dma_dev.h"


#ifdef ccs
#pragma DATA_ALIGN(CTL_TBL, 1024)
static uint8_t CTL_TBL[1024];
#else
static uint8_t CTL_TBL[1024] __attribute__ ((aligned(1024)));
#endif


extern "C" {
void udma_error_isr_handler(void);
}


dma_dev_t * dma_dev_t::device = NULL;


void udma_error_isr_handler(void)
{
	dma_dev_t::dispatch_dma_error_isr();
}


dma_dev_t::dma_dev_t()
{
	portENTER_CRITICAL();

	SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
	SysCtlPeripheralReset(SYSCTL_PERIPH_UDMA);
	HWREG(UDMA_CFG) = UDMA_CFG_MASTEN;
	HWREG(UDMA_CTLBASE) = (uint32_t)CTL_TBL;

	dma_dev_t::device = this;

	portEXIT_CRITICAL();
}


dma_dev_t::~dma_dev_t() {
	portENTER_CRITICAL();

	dma_dev_t::device = NULL;
	HWREG(UDMA_CFG) = UDMA_CFG_MASTEN;
	SysCtlPeripheralDisable(SYSCTL_PERIPH_UDMA);

	portEXIT_CRITICAL();
}
