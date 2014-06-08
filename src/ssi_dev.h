
#ifndef __ssi_dev_h__
#define __ssi_dev_h__

#include <vector>
#include "dma_dev.h"


template<uint32_t Base>
class ssi_spec_t {
};


template<>
class ssi_spec_t<SSI0_BASE> {
public:
	static const uint32_t rx_dma_channel = (1 << UDMA_CHANNEL_SSI0RX);
	static const uint32_t rx_dma_channel_number = UDMA_CHANNEL_SSI0RX;
	static const uint32_t rx_dma_channel_control = UDMA_CHCTL_DSTSIZE_8 | UDMA_CHCTL_SRCSIZE_8 | UDMA_CHCTL_SRCINC_NONE |
			UDMA_CHCTL_DSTINC_8 | UDMA_CHCTL_ARBSIZE_1 | UDMA_CHCTL_XFERMODE_BASIC;

	static const uint32_t tx_dma_channel = (1 << UDMA_CHANNEL_SSI0TX);
	static const uint32_t tx_dma_channel_number = UDMA_CHANNEL_SSI0TX;
	static const uint32_t tx_dma_channel_control =  UDMA_CHCTL_DSTSIZE_8 | UDMA_CHCTL_SRCSIZE_8 | UDMA_CHCTL_DSTINC_NONE |
			UDMA_CHCTL_SRCINC_8 | UDMA_CHCTL_ARBSIZE_1 | UDMA_CHCTL_XFERMODE_BASIC;

	static const uint32_t interrupt_number  = INT_SSI0;

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

	dma_dev_t& dma;
	SemaphoreHandle_t interrupt_semaphore;
	SemaphoreHandle_t dev_lock;

	void handle_isr(void) {
		static BaseType_t task_woken;

		uint32_t msk_status = get_interrupt_status();
		uint32_t raw_status = get_raw_interrupt_status();
		uint32_t dma_status = dma.get_interrupt_status();

		clear_interrupt(SSI_IM_RORIM | SSI_IM_RTIM);
		dma.clear_interrupt(spec.tx_dma_channel | spec.rx_dma_channel);

		task_woken = pdFALSE;
		if (dma_status & spec.tx_dma_channel) {
			xSemaphoreGiveFromISR(interrupt_semaphore, &task_woken);
		} else {
			int a;
			a = 10;
		}

		if (task_woken != pdFALSE) {
			portYIELD_FROM_ISR(task_woken);
		}
	}

	inline uint32_t get_interrupt_status(void) const {
		return HWREG(Base + SSI_O_MIS);
	}

	inline uint32_t get_raw_interrupt_status(void) const {
		return HWREG(Base + SSI_O_RIS);
	}

	inline void clear_interrupt(uint32_t mask) const {
		HWREG(Base + SSI_O_ICR) = mask;
	}

	inline void enable_interrupt(void) const {
		IntEnable(spec.interrupt_number);
	}

	inline void disable_interrupt(void) const {
		IntDisable(spec.interrupt_number);
	}

public:

	static void dispatch_isr(void) {
		if (ssi_dev_t::device) {
			ssi_dev_t::device->handle_isr();
		}
	}

	ssi_dev_t(dma_dev_t& dma_) : dma(dma_) {
		disable_interrupt();

		ssi_dev_t::device = this;

		interrupt_semaphore = xSemaphoreCreateBinary();
		dev_lock = xSemaphoreCreateMutex();

		SysCtlPeripheralEnable(spec.ssi_periph);
		SysCtlPeripheralEnable(spec.gpio_periph);

		GPIOPinTypeSSI(spec.gpio_base, spec.gpio_pins);
		GPIOPinTypeGPIOOutput(spec.gpio_base, spec.gpio_cs_pin);
		GPIOPinWrite(spec.gpio_base, spec.gpio_cs_pin, spec.gpio_cs_pin);

		SSIConfigSetExpClk(Base, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 100000, 8);
		HWREG(Base + SSI_O_DMACTL) = SSI_DMA_TX | SSI_DMA_RX;	/* Enable DMA operations on RX and TX */
		clear_interrupt(SSI_IM_RORIM | SSI_IM_RTIM);
		HWREG(Base + SSI_O_IM) = 0;								/* SSI_IM_TXIM | SSI_IM_RXIM | SSI_IM_RTIM | SSI_IM_RORIM */
		HWREG(Base + SSI_O_CR1) = /*SSI_CR1_EOT |*/ SSI_CR1_SSE;	/* Master mode, End of Transmission interrupt mode SSI enabled; */

		dma.set_attribute(spec.rx_dma_channel | spec.tx_dma_channel, UDMA_ATTRIBUTE_NONE);
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
		while (read_non_blocking(NULL)) {
		}
	}

	bool transceive(void * in_data, uint32_t count, void * out_data) {
		if (xSemaphoreTake(dev_lock, portMAX_DELAY) == pdTRUE) {
			dma.set_channel_control(spec.tx_dma_channel_number, spec.tx_dma_channel_control,
					in_data, (void *)(Base + SSI_O_DR), count);

			dma.set_channel_control(spec.rx_dma_channel_number, spec.rx_dma_channel_control,
					(void *)(Base + SSI_O_DR), out_data, count);

			clear_interrupt(SSI_IM_RORIM | SSI_IM_RTIM);
			dma.clear_interrupt(spec.tx_dma_channel | spec.rx_dma_channel);

			xQueueReset(interrupt_semaphore);

			enable_interrupt();
			dma.enable_channel(spec.tx_dma_channel | spec.rx_dma_channel);

			bool result = xSemaphoreTake(interrupt_semaphore, portMAX_DELAY) == pdTRUE;

			disable_interrupt();

			xSemaphoreGive(dev_lock);
			return result;
		}

		return false;
	}

	uint32_t send(uint32_t data) {
		while (!is_transmit_fifo_not_full()) {}
		write(data);

		while (!is_receive_fifo_not_empty()) {}
		return read();
	}

	bool send_non_blocking(uint32_t data) {
		if (is_transmit_fifo_not_full()) {
			write(data);
			return true;
		} else {
			return false;
		}
	}

	bool read_non_blocking(uint32_t * data) {
		if (is_receive_fifo_not_empty()) {
			uint32_t result = read();
			if (data) {
				*data = result;
			}
			return true;
		} else {
			return false;
		}
	}

	void write(uint32_t data) {
		HWREG(Base + SSI_O_DR) = data;
	}

	uint32_t read(void) {
		return HWREG(Base + SSI_O_DR);
	}

	uint32_t get_status(void) const {
		return HWREG(Base + SSI_O_SR);
	}

	bool is_busy(void) const {
		return get_status() & SSI_SR_BSY;
	}

	bool is_receive_fifo_full(void) const {
		return get_status() & SSI_SR_RFF;
	}

	bool is_receive_fifo_not_empty(void) const {
		return get_status() & SSI_SR_RNE;
	}

	bool is_transmit_fifo_not_full(void) const {
		return get_status() & SSI_SR_TNF;
	}

	bool is_transmit_fifo_empty(void) const {
		return get_status() & SSI_SR_TFE;
	}
};

template<uint32_t Base> ssi_dev_t<Base> * ssi_dev_t<Base>::device = NULL;

#endif /* __ssi_dev_h__ */
