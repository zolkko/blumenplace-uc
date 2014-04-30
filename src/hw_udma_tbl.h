
#ifndef __hw_udma_ctbl_h__
#define __hw_udma_ctbl_h__

#ifdef __cplusplus
extern "C" {
#endif


#ifdef ccs
#pragma DATA_ALIGN(UDMA_CTL_TBL, 1024)
uint8_t UDMA_CTL_TBL[1024];
#else
uint8_t UDMA_CTL_TBL[1024] __attribute__ ((aligned(1024)));
#endif


#ifdef __cplusplus
}
#endif


#endif /* __hw_udma_ctbl_h__ */
