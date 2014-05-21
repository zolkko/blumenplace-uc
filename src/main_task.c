
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
	uint8_t expect;
	float value;

	while (true) {
		for (expect = 0; expect < 8; expect++) {
			printf("=== [ %d ] ===============\n", expect);
			printf("Reseting\t\t");
			if (sht1x_software_reset() == SHT1X_ERROR_OK) {
				printf("OK\n");
				vTaskDelay(11 / portTICK_PERIOD_MS);
			} else {
				printf("FAIL\n");
			}

			printf("Status Register Write\t");
			if (sht1x_status_write(expect) == SHT1X_ERROR_OK) {
				printf("OK\n");
			} else {
				printf("FAIL\n");
			}

			printf("Status Register Read\t");
			if (sht1x_status_read(&data) == SHT1X_ERROR_OK) {
				printf("%d\n", data);
			} else {
				printf("FAIL\n");
			}

			printf("Temperature:\t\t");
			if (sht1x_temperature_read(&value) == SHT1X_ERROR_OK) {
				printf("%0.02f\n", value);
			} else {
				printf("FAIL\n");
			}

			printf("Moisture:\t\t");
			if (sht1x_moisture_read(&value) == SHT1X_ERROR_OK) {
				printf("%0.02f\n", value);
			} else {
				printf("FAIL\n");
			}

			vTaskDelay(500 / portTICK_PERIOD_MS);
		}
	}
}
