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
#pragma DATA_ALIGN(UDMA_CTL_TBL, 1024)
static uint8_t CTL_TBL[1024];
#else
static uint8_t CTL_TBL[1024] __attribute__ ((aligned(1024)));
#endif


extern "C" {
void udma_error_isr_handler(void);
}


void udma_error_isr_handler(void)
{
	dma_dev_t::get().handle_error_isr();
}


void dma_dev_t::initialize(void)
{
	portENTER_CRITICAL();

	SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
	HWREG(UDMA_CFG) = UDMA_CFG_MASTEN;
	HWREG(UDMA_CTLBASE) = (uint32_t)CTL_TBL;

	portEXIT_CRITICAL();
}
