/*
 * sht1x.h - A SHT1x temperature and relative humidity sensor driver for tm4c123
 *
 * Copyright (c) 2014 Alexey Anisimov <zolkko@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef __sht1x_h__
#define __sht1x_h__


#ifdef __cplusplus
extern "C" {
#endif

/* Status register bit-masks */
#define SHT1X_SREG_LOW_RES_bm		(1)			/* 0: 14bit temperature and 12bit humidity; 1: 12bit temperature and 8bit humidity */
#define SHT1X_SREG_OTP_bm			(1 << 1)	/* no reload from OTP */
#define SHT1X_SREG_HEATER_bm		(1 << 2)	/* heater - default off */
#define SHT1X_SREG_LOW_VOLTAGE_bm	(1 << 6)	/* 0 for VDD > 2.47, 1 for VDD < 2.47 */

#define SHT1X_SREG_bm				(SHT1X_SREG_HEATER_bm | SHT1X_SREG_OTP_bm | SHT1X_SREG_LOW_RES_bm)

typedef enum {
	SHT1X_ERROR_OK,
	SHT1X_ERROR_NO_CMD_ACK,
	SHT1X_ERROR_NO_PAYLOAD_ACK,
	SHT1X_ERROR_MEASUREMENT_TIMEOUT,
	SHT1X_ERROR_BUSY,
	SHT1X_ERROR_INVALID_CRC,
	SHT1X_ERROR_UDMA_FAILURE,
	SHT1X_ERROR_UNKNOWN
} sht1x_error_t;


void sht1x_init(void);

sht1x_error_t sht1x_status_read(uint8_t * data);

sht1x_error_t sht1x_status_write(uint8_t status);

sht1x_error_t sht1x_temperature_read(float * temperature);

sht1x_error_t sht1x_moisture_read(float * moisture);

sht1x_error_t sht1x_software_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* __sht1x_h__ */
