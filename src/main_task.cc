
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "FreeRTOSConfig.h"

#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include "main_task.h"

#include "sht1x.h"
#include "ccx.h"

#include "ccx.hpp"

#include "driverlib/interrupt.h"
#include <vector>

#include "driverlib/ssi.h"
#include "driverlib/gpio.h"
#include "driverlib/udma.h"
#include "driverlib/sysctl.h"

#include "gpio_pin.h"
#include "gpio_port.h"
#include "ssi_dev.h"


uint8_t in_data[20] __attribute__ ((aligned(32)));
uint8_t out_data[50] __attribute__ ((aligned(32)));


void main_task(void * params)
{
	uint8_t i, j;
	uint8_t input_data;

	dma_dev_t& dma = dma_dev_t::get();
	dma.initialize();

	ssi_dev_t<SSI0_BASE> ssi0(dma);

	ssi0.flush();

	j = 0;
	while (true) {
		//ssi0.chip_select();
		//input_data = ssi0.send(0xaa);
		//ssi0.chip_release();
		//printf("ssi0.send 170 = %d\n", input_data);

		if (j > (255 - sizeof(in_data))) {
			j = 0;
		}

		for (i = 0; i < sizeof(in_data); i++) in_data[i]  = j + i;
		for (i = 0; i < sizeof(out_data); i++) out_data[i] = 0;
		//for (i = 0; i < sizeof(out_data); i++) { printf("%d ", out_data[i]); } printf("\n");
		ssi0.chip_select();
		ssi0.transceive(in_data, sizeof(in_data), out_data);
		ssi0.chip_release();

		for (i = 0; i < sizeof(in_data); i++) { printf("%d ", in_data[i]); } printf("\n");
		for (i = 0; i < sizeof(out_data); i++) { printf("%d ", out_data[i]); } printf("\n");
		printf("------------------\n");

		j += sizeof(in_data);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

	/*

	sht1x_init();

	uint8_t data;
	uint8_t expect;
	float value;*/

	/*ssi_svc_init((ssi_svc_t *) &ssi0_svc, &dma_svc0);
	ssi_svc_flush((ssi_svc_t *) &ssi0_svc);
*/

	while (true) {
		/*ssi_svc_select((ssi_svc_t *) &ssi0_svc);
		// data = ssi_svc_send((ssi_svc_t *) &ssi0_svc, 0xaa);
		ssi_svc_transceive((ssi_svc_t *) &ssi0_svc, out_data, in_data, sizeof(in_data));
		ssi_svc_release((ssi_svc_t *) &ssi0_svc);*/

		//printf("Output: %d\n", data);
		/*for (expect = 0; expect < 8; expect++) {
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
		}*/
		vTaskDelay(50 / portTICK_PERIOD_MS);
	}
}
