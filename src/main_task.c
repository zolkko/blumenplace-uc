
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
	uint8_t expect; //SHT1X_SREG_LOW_RES_bm;
	while (true) {
		for (expect = 0; expect < 2; expect++) {
			printf("============\nExpected value: %d", expect);

			printf("Status Register Write\t");
			if (sht1x_status_write(expect) == SHT1X_ERROR_OK) {
				printf("OK\n");
			} else {
				printf("FAIL\n");
			}

			printf("Status Register Read\t");
			if (sht1x_status_read(&data) == SHT1X_ERROR_OK) {
				if (data == expect) {
					printf("OK=%d\n", data);
				} else {
					printf("Got=%d Expect=%d\n", data, expect);
				}
			} else {
				printf("FAIL\n");
			}

			vTaskDelay(500 / portTICK_PERIOD_MS);
		}

		/*printf("Read temperature");
		if (sht1x_temperature_read(&temper) == SHT1X_ERROR_OK) {
			printf("\tOK = %d\n", temper);
		} else {
			printf("\tFAIL\n");
		}*/
	}
}
