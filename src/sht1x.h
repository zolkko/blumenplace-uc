
#ifndef __sht1x_h__
#define __sht1x_h__


#ifdef __cplusplus
extern "C" {
#endif

void sht1x_init(void);

uint8_t sht1x_read_temperature(void);

#ifdef __cplusplus
}
#endif

#endif /* __sht1x_h__ */
