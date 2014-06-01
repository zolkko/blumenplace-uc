
#ifndef __dma_svc_h__
#define __dma_svc_h__

#ifdef __cplusplus
extern "C" {
#endif


typedef void (*dma_error_handler_t) (uint32_t status, void * data);


typedef struct {
	dma_error_handler_t handler;
	void * data;
} dma_svc_error_handler_item_t;


#define DMA_SVC_MAX_ERROR_HANDLERS	(0x04)


typedef struct {
	uint32_t error_handler_len;
	dma_svc_error_handler_item_t error_handlers[DMA_SVC_MAX_ERROR_HANDLERS];
} dma_svc_t;


void dma_svc_init(dma_svc_t * svc);

void dma_svc_ssi0_rx(dma_svc_t * svc, void * data, uint32_t item_count);

void dma_svc_ssi0_tx(dma_svc_t * svc, void * data, uint32_t item_count);

void dma_svc_ssi0_transceive(dma_svc_t * svc);

bool dma_svc_ssi0_busy(dma_svc_t * svc);

bool dma_svc_register_error_handler(dma_svc_t * svc, dma_error_handler_t error_handler, void * data);

#ifdef __cplusplus
}
#endif


#endif /* __dma_svc_h__ */
