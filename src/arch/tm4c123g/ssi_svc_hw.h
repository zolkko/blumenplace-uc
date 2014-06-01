
#ifndef __ssi_svc_hw_h__
#define __ssi_svc_hw_h__

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
	ssi_svc_t svc;

	uint32_t ssi_periph;
	uint32_t ssi_base;

	uint32_t gpio_periph;
	uint32_t gpio_base;
	uint32_t gpio_pins;
	uint32_t gpio_cs;

	uint32_t dma_tx_channel;
	uint32_t dma_rx_channel;

	SemaphoreHandle_t lock;
	SemaphoreHandle_t dma_tx;
} ssi_hw_t;

#ifdef __cplusplus
}
#endif

#endif /* __ssi_svc_hw_h__ */
