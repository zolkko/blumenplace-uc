
#ifndef __ssi_svc_h__
#define __ssi_svc_h__

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
	dma_svc_t * dma_svc;
} ssi_svc_t;


void ssi_svc_init(ssi_svc_t * svc, dma_svc_t * dma_svc);

void ssi_svc_select(ssi_svc_t * svc);

void ssi_svc_release(ssi_svc_t * svc);

void ssi_svc_flush(ssi_svc_t * svc);

bool ssi_svc_transceive(ssi_svc_t * svc, void * out_data, void * in_data, uint32_t count);

uint32_t ssi_svc_send(ssi_svc_t * svc, uint32_t data);

bool ssi_svc_send_async(ssi_svc_t * svc, uint32_t data);

bool ssi_svc_read_async(ssi_svc_t * svc, uint32_t * data);

bool ssi_svc_busy(ssi_svc_t * svc);


#ifdef __cplusplus
}
#endif

#endif /* __ssi_svc_h__ */
