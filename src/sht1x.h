
#ifndef __sht1x_h__
#define __sht1x_h__


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Status register bit-masks
 */
#define SHT1X_SREG_HI_RES_bm		1			/* 12bit humidity, 14bit temperature */
#define SHT1X_SREG_OTP_bm			(1 << 1)	/* no reload from OTP */
#define SHT1X_SREG_HEATER_bm		(1 << 2)	/* heater - default off */
#define SHT1X_SREG_LOW_VOLTAGE_bm	(1 << 6)	/* 0 for VDD > 2.47, 1 for VDD < 2.47 */


void sht1x_init(void);

uint8_t sht1x_read_status(void);

uint8_t sht1x_read_temperature(void);

#ifdef __cplusplus
}
#endif

#endif /* __sht1x_h__ */
