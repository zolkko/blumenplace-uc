
#include <stdbool.h>
#include <stdint.h>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <inc/hw_gpio.h>
#include <inc/hw_timer.h>

#include "inc/hw_memmap.h"
#include "inc/hw_ssi.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/interrupt.h"
#include "driverlib/ssi.h"

#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/udma.h"

#include <FreeRTOS.h>
#include <semphr.h>

#include "dma_svc.h"
#include "ssi_svc.h"
#include "arch/tm4c123g/ssi_svc_hw.h"


ssi_hw_t ssi0_svc = {
	{0},

	SYSCTL_PERIPH_SSI0,
	SSI0_BASE,

	SYSCTL_PERIPH_GPIOA,
	GPIO_PORTA_BASE,
	GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_2,
	GPIO_PIN_3,

	(1 << UDMA_CHANNEL_SSI0TX),
	(1 << UDMA_CHANNEL_SSI0RX),

	0,
	0
};


void ssi0_isr_handler(void)
{
	static BaseType_t task_woken;
	uint32_t msk_status = HWREG(ssi0_svc.ssi_base + SSI_O_MIS);
	uint32_t dma_status = uDMAIntStatus();

	HWREG(ssi0_svc.ssi_base + SSI_O_ICR) = SSI_IM_RORIM | SSI_IM_RTIM;
	uDMAIntClear(ssi0_svc.dma_tx_channel | ssi0_svc.dma_rx_channel);

	task_woken = pdFALSE;
	if (dma_status & ssi0_svc.dma_tx_channel) {
		if (ssi0_svc.dma_tx) {
			xSemaphoreGiveFromISR(ssi0_svc.dma_tx, &task_woken);
		}
	}

	if (task_woken != pdFALSE) {
		portYIELD_FROM_ISR(task_woken);
	}
}


/*
 * This implementation controls CS signal in software
 * because TM4C123G does not support advanced transmission mode.
 */
void ssi_svc_init(ssi_svc_t * svc, dma_svc_t * dma_svc)
{
	IntDisable(INT_SSI0);

	svc->dma_svc = dma_svc;

	ssi_hw_t * hw = (ssi_hw_t *)svc;
	hw->lock = xSemaphoreCreateMutex();
	hw->dma_tx = xSemaphoreCreateBinary();

	SysCtlPeripheralEnable(hw->ssi_periph);
	SysCtlPeripheralEnable(hw->gpio_periph);

	GPIOPinTypeSSI(hw->gpio_base, hw->gpio_pins);
	GPIOPinTypeGPIOOutput(hw->gpio_base, hw->gpio_cs);
	GPIOPinWrite(hw->gpio_base, hw->gpio_cs, hw->gpio_cs);

	SSIConfigSetExpClk(hw->ssi_base, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 100000, 8);

	// Master mode, End of Transmission interrupt mode SSI enabled;
	HWREG(hw->ssi_base + SSI_O_CR1) = SSI_CR1_EOT | SSI_CR1_SSE;

	// Enable DMA operations on RX and TX
	HWREG(hw->ssi_base + SSI_O_DMACTL) = SSI_DMA_TX | SSI_DMA_RX;

	// Clear interrupts
	HWREG(hw->ssi_base + SSI_O_ICR) = SSI_IM_RORIM | SSI_IM_RTIM;

	uDMAIntClear(ssi0_svc.dma_tx_channel | ssi0_svc.dma_rx_channel);

	// Enable TX and RX interrupts
	HWREG(hw->ssi_base + SSI_O_IM) = 0;//SSI_IM_TXIM | SSI_IM_RXIM | SSI_IM_RTIM | SSI_IM_RORIM;
}


void ssi_svc_select(ssi_svc_t * svc)
{
	ssi_hw_t * hw = (ssi_hw_t *) svc;
	GPIOPinWrite(hw->gpio_base, hw->gpio_cs, 0);
}


void ssi_svc_release(ssi_svc_t * svc)
{
	ssi_hw_t * hw = (ssi_hw_t *) svc;
	GPIOPinWrite(hw->gpio_base, hw->gpio_cs, 0xff);
}


void ssi_svc_flush(ssi_svc_t * svc)
{
	ssi_hw_t * hw = (ssi_hw_t *) svc;
	uint32_t tmp;
	while (SSIDataGetNonBlocking(hw->ssi_base, &tmp)) {
	}
}


bool ssi_svc_transceive(ssi_svc_t * svc, void * out_data, void * in_data, uint32_t count)
{
	ssi_hw_t * hw = (ssi_hw_t *) svc;

	if (xSemaphoreTake(hw->lock, portMAX_DELAY) == pdTRUE) {
		dma_svc_ssi0_rx(svc->dma_svc, in_data, count);
		dma_svc_ssi0_tx(svc->dma_svc, out_data, count);

		HWREG(hw->ssi_base + SSI_O_ICR) = SSI_IM_RORIM | SSI_IM_RTIM;
		uDMAIntClear(ssi0_svc.dma_tx_channel | ssi0_svc.dma_rx_channel);
		IntEnable(INT_SSI0);
		dma_svc_ssi0_transceive(svc->dma_svc);

		bool result = xSemaphoreTake(hw->dma_tx, portMAX_DELAY) == pdTRUE;
		IntDisable(INT_SSI0);

		xSemaphoreGive(hw->lock);
		return result;
	}

	return false;
}


uint32_t ssi_svc_send(ssi_svc_t * svc, uint32_t data)
{
	uint32_t result;
	ssi_hw_t * hw = (ssi_hw_t *) svc;

	SSIDataPut(hw->ssi_base, data);
	while (SSIBusy(hw->ssi_base)) ;
	SSIDataGet(hw->ssi_base, &result);

	return result;
}


bool ssi_svc_send_async(ssi_svc_t * svc, uint32_t data)
{
	ssi_hw_t * hw = (ssi_hw_t *) svc;
	return SSIDataPutNonBlocking(hw->ssi_base, data);
}


bool ssi_svc_read_async(ssi_svc_t * svc, uint32_t * data)
{
	ssi_hw_t * hw = (ssi_hw_t *) svc;
	return SSIDataGetNonBlocking(hw->ssi_base, data);
}


bool ssi_svc_busy(ssi_svc_t * svc)
{
	ssi_hw_t * hw = (ssi_hw_t *) svc;
	return SSIBusy(hw->ssi_base) || dma_svc_ssi0_busy(svc->dma_svc);
}
