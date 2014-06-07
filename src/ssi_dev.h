
#ifndef __ssi_dev_h__
#define __ssi_dev_h__

#include <vector>
#include "dma_svc.h"


template<uint32_t Base>
class ssi_spec_t {
};


template<>
class ssi_spec_t<SSI0_BASE> {
public:
	static const uint32_t rx_dma_channel = (1 << UDMA_CHANNEL_SSI0RX);
	static const uint32_t tx_dma_channel = (1 << UDMA_CHANNEL_SSI0TX);
	static const uint32_t interrupt_num  = INT_SSI0;

	static const uint32_t ssi_periph = SYSCTL_PERIPH_SSI0;
	static const uint32_t gpio_periph = SYSCTL_PERIPH_GPIOA;
	static const uint32_t gpio_base = GPIO_PORTA_BASE;
	static const uint32_t gpio_pins = GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_2;
	static const uint32_t gpio_cs_pin = GPIO_PIN_3;
};


template<uint32_t Base>
class ssi_dev_t {
private:
	static const ssi_spec_t<Base> spec;

	static ssi_dev_t* device;

	SemaphoreHandle_t interrupt_semaphore;
	SemaphoreHandle_t dev_lock;
	dma_svc_t* dma_svc;

	void handle_isr(void) {
		static BaseType_t task_woken;

		uint32_t msk_status = HWREG(Base + SSI_O_MIS);
		uint32_t dma_status = uDMAIntStatus();

		HWREG(Base + SSI_O_ICR) = SSI_IM_RORIM | SSI_IM_RTIM;
		uDMAIntClear(spec.tx_dma_channel | spec.rx_dma_channel);

		task_woken = pdFALSE;
		if (dma_status & spec.tx_dma_channel) {
			xSemaphoreGiveFromISR(interrupt_semaphore, &task_woken);
		}

		if (task_woken != pdFALSE) {
			portYIELD_FROM_ISR(task_woken);
		}
	}

public:

	static void dispatch_isr(void) {
		if (ssi_dev_t::device) {
			ssi_dev_t::device->handle_isr();
		}
	}

	ssi_dev_t(dma_svc_t * dma_svc_) : dma_svc(dma_svc_) {
		IntDisable(spec.interrupt_num);

		ssi_dev_t::device = this;

		interrupt_semaphore = xSemaphoreCreateBinary();
		dev_lock = xSemaphoreCreateMutex();

		SysCtlPeripheralEnable(spec.ssi_periph);
		SysCtlPeripheralEnable(spec.gpio_periph);

		GPIOPinTypeSSI(spec.gpio_base, spec.gpio_pins);
		GPIOPinTypeGPIOOutput(spec.gpio_base, spec.gpio_cs_pin);
		GPIOPinWrite(spec.gpio_base, spec.gpio_cs_pin, spec.gpio_cs_pin);

		SSIConfigSetExpClk(Base, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 100000, 8);
		HWREG(Base + SSI_O_CR1) = SSI_CR1_EOT | SSI_CR1_SSE;	/* Master mode, End of Transmission interrupt mode SSI enabled; */
		HWREG(Base + SSI_O_DMACTL) = SSI_DMA_TX | SSI_DMA_RX;	/* Enable DMA operations on RX and TX */
		HWREG(Base + SSI_O_ICR) = SSI_IM_RORIM | SSI_IM_RTIM;	/* Clear interrupts */

		uDMAIntClear(spec.tx_dma_channel | spec.rx_dma_channel);

		HWREG(Base + SSI_O_IM) = 0; //SSI_IM_TXIM | SSI_IM_RXIM | SSI_IM_RTIM | SSI_IM_RORIM;
	}

	virtual ~ssi_dev_t() {
		ssi_dev_t::device = NULL;
		SysCtlPeripheralDisable(spec.gpio_periph);
		SysCtlPeripheralDisable(spec.ssi_periph);
	}

	void chip_select(void) {
		GPIOPinWrite(spec.gpio_base, spec.gpio_cs_pin, 0x00);
	}

	void chip_release(void) {
		GPIOPinWrite(spec.gpio_base, spec.gpio_cs_pin, spec.gpio_cs_pin);
	}

	void flush(void) {
		uint32_t tmp;
		while (SSIDataGetNonBlocking(Base, &tmp)) ;
	}

	bool transceive(void * out_data, void * in_data, uint32_t count) {
		if (xSemaphoreTake(dev_lock, portMAX_DELAY) == pdTRUE) {
			dma_svc_ssi0_rx(dma_svc, in_data, count);
			dma_svc_ssi0_tx(dma_svc, out_data, count);

			HWREG(Base + SSI_O_ICR) = SSI_IM_RORIM | SSI_IM_RTIM;
			uDMAIntClear(spec.tx_dma_channel | spec.rx_dma_channel);
			IntEnable(spec.interrupt_num);
			dma_svc_ssi0_transceive(dma_svc);

			xQueueReset(interrupt_semaphore);
			bool result = xSemaphoreTake(interrupt_semaphore, portMAX_DELAY) == pdTRUE;
			IntDisable(spec.interrupt_num);

			xSemaphoreGive(dev_lock);
			return result;
		}

		return false;
	}

	uint32_t send(uint32_t data) {
		uint32_t result;
		SSIDataPut(Base, data);
		while (SSIBusy(Base)) ;
		SSIDataGet(Base, &result);
		return result;
	}

	bool send_async(uint32_t data) {
		return SSIDataPutNonBlocking(Base, data);
	}

	bool read_async(uint32_t * data) {
		return SSIDataGetNonBlocking(Base, data);
	}

	bool is_busy(bool) {
		return SSIBusy(Base) || dma_svc_ssi0_busy(dma_svc);
	}
};

template<uint32_t Base> ssi_dev_t<Base> * ssi_dev_t<Base>::device = NULL;

#endif /* __ssi_dev_h__ */
