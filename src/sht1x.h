
#ifndef __sht1x_h__
#define __sht1x_h__


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Status register bit-masks
 */
#define SHT1X_SREG_LOW_RES_bm		(1)			/* 0: 14bit temperature and 12bit humidity; 1: 12bit temperature and 8bit humidity */
#define SHT1X_SREG_OTP_bm			(1 << 1)	/* no reload from OTP */
#define SHT1X_SREG_HEATER_bm		(1 << 2)	/* heater - default off */
#define SHT1X_SREG_LOW_VOLTAGE_bm	(1 << 6)	/* 0 for VDD > 2.47, 1 for VDD < 2.47 */

typedef enum {
	SHT1X_ERROR_OK,
	SHT1X_ERROR_CMD,
	SHT1X_ERROR_NO_CMD_ACK,
	SHT1X_ERROR_NO_PAYLOAD_ACK,
	SHT1X_ERROR_MEASUREMENT_TIMEOUT,
	SHT1X_ERROR_INVALID_STATE,
	SHT1X_ERROR_BUSY,
	SHT1X_ERROR_UNKNOWN
} sht1x_error_t;

void sht1x_init(void);

sht1x_error_t sht1x_status_read(uint8_t * data);

sht1x_error_t sht1x_status_write(uint8_t status);

sht1x_error_t sht1x_temperature_read(uint16_t * temperature);

sht1x_error_t sht1x_moisture_read(uint16_t * moisture);

#ifdef __cplusplus
}
#endif

#endif /* __sht1x_h__ */
