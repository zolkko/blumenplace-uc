
#ifndef __dma_dev_h__
#define __dma_dev_h__

#include <vector>
#include <algorithm>
#include <inc/hw_udma.h>
#include <driverlib/udma.h>


typedef struct {
	volatile void * src_addr;
	volatile void * dst_addr;
	volatile uint32_t control;
	volatile uint32_t spare;
} dma_control_table_t;


class dma_error_listener_t {
public:
	virtual void handle_error_isr(uint32_t status) const = 0;
};


class dma_soft_listener_t {
public:
	virtual void handle_soft_isr(uint32_t status) const = 0;
};


class dma_listener_t : public dma_error_listener_t /* is not used at the moment , dma_soft_listener_t*/ {
};


typedef enum {
	UDMA_ATTRIBUTE_NONE = 0,
	UDMA_ATTRIBUTE_ALTSELECT = (1),
	UDMA_ATTRIBUTE_USEBURST = (1 << 1),
	UDMA_ATTRIBUTE_HIGH_PRIORITY = (1 << 2),
	UDMA_ATTRIBUTE_REQMASK = (1 << 3),
} dma_attribute_t;


class dma_dev_t {
private:
	std::vector<dma_listener_t*> error_listeners;

	dma_dev_t() { }

	dma_dev_t(const dma_dev_t& dma_dev_) {}

	void operator=(const dma_dev_t& dma_dev_) {}

public:
	static dma_dev_t& get(void) {
		static dma_dev_t dma_dev;
		return dma_dev;
	}

	void initialize(void);

	void deinitialize(void) {
		HWREG(UDMA_CFG) = UDMA_CFG_MASTEN;
		SysCtlPeripheralDisable(SYSCTL_PERIPH_UDMA);
	}

	void subscribe_error(dma_listener_t * listener) {
		portENTER_CRITICAL();
		error_listeners.push_back(listener);
		portEXIT_CRITICAL();
	}

	bool unsubscribe_error(dma_listener_t * listener) {
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

	void handle_error_isr(void) {
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

	void clear_interrupt(uint32_t channel_mask) {
		HWREG(UDMA_CHIS) = channel_mask;
	}

	uint32_t get_interrupt_status(void) {
		return HWREG(UDMA_CHIS);
	}

	void set_attribute(uint32_t channel_mask, dma_attribute_t attribute) {
		if (attribute & UDMA_ATTRIBUTE_USEBURST) {
			HWREG(UDMA_USEBURSTSET) = channel_mask;
		} else {
			HWREG(UDMA_USEBURSTCLR) = channel_mask;
		}

		if (attribute & UDMA_ATTRIBUTE_ALTSELECT) {
			HWREG(UDMA_ALTSET) = channel_mask;
		} else {
			HWREG(UDMA_ALTCLR) = channel_mask;
		}

		if (attribute & UDMA_ATTRIBUTE_HIGH_PRIORITY) {
			HWREG(UDMA_PRIOSET) = channel_mask;
		} else {
			HWREG(UDMA_PRIOCLR) = channel_mask;
		}

		if (attribute & UDMA_ATTRIBUTE_REQMASK) {
			HWREG(UDMA_REQMASKSET) = channel_mask;
		} else {
			HWREG(UDMA_REQMASKCLR) = channel_mask;
		}
	}

	void set_channel_control(uint32_t channel, uint32_t control, void * src_addr, void *dst_addr, uint32_t transfer_size) {
		uint32_t inc;

		uint32_t mode = control & UDMA_CHCTL_XFERMODE_M;
		dma_control_table_t * control_table = (dma_control_table_t *)HWREG(UDMA_CTLBASE);

		channel &= 0x3f;

		// Get the address increment value for the source, from the control word.
		inc = control & UDMA_CHCTL_SRCINC_M;
		if (inc != UDMA_CHCTL_SRCINC_NONE) {
			control_table[channel].src_addr = (void *)((uint32_t)src_addr + (transfer_size << (inc >> 26)) - 1);
		} else {
			control_table[channel].src_addr = src_addr;
		}

		// Get the address increment value for the destination, from the control word.
		inc = control & UDMA_CHCTL_DSTINC_M;
		if (inc != UDMA_DST_INC_NONE) {
			if (mode == UDMA_CHCTL_XFERMODE_MEM_SG || mode == UDMA_CHCTL_XFERMODE_MEM_SGA || mode == UDMA_CHCTL_XFERMODE_PER_SG || mode == UDMA_CHCTL_XFERMODE_PER_SGA) {
				control_table[channel].dst_addr = (void *)&control_table[channel].spare;
			} else {
				control_table[channel].dst_addr = (void *)((uint32_t)dst_addr + (transfer_size << (inc >> 30)) - 1);
			}
		} else {
			control_table[channel].dst_addr = dst_addr;
		}

		// Set the transfer size and mode in the control word (but don't write the control word yet as it could kick off a transfer).
		control_table[channel].control = control | (((transfer_size - 1) << 4) & UDMA_CHCTL_XFERSIZE_M);;
	}

	bool is_channel_enabled(uint32_t channel_mask) {
		return HWREG(UDMA_ENASET) & channel_mask;
	}

	void enable_channel(uint32_t channel_mask) {
		HWREG(UDMA_ENASET) = channel_mask;
	}

	void disable_channel(uint32_t channel_mask) {
		HWREG(UDMA_ENACLR) = channel_mask;
	}
};

#endif /* __dma_dev_h__ */
