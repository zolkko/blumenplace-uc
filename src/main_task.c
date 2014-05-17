
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "FreeRTOSConfig.h"

#include <FreeRTOS.h>
#include <task.h>

#include "main_task.h"

#include "sht1x.h"


void main_task(void * params)
{
	sht1x_init();

	uint8_t data;
	while (true) {
		printf("Status Register Write\t");
		/*if (sht1x_status_write(SHT1X_SREG_OTP_bm) == SHT1X_ERROR_OK) {
			printf("OK\n");
		} else {
			printf("FAIL\n");
		}*/

		printf("Status Register Read\t");
		if (sht1x_status_read(&data) == SHT1X_ERROR_OK) {
			if (data == SHT1X_SREG_OTP_bm) {
				printf("OK\t=\t%d\n", data);
			} else {
				printf("INVALID\t=\t%d\texpect\t%d\n", data, SHT1X_SREG_OTP_bm);
			}
		} else {
			printf("FAIL\n");
		}

		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}
