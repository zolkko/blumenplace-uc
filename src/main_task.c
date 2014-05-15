
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "main_task.h"

#include "sht1x.h"


void main_task(void * params)
{
	sht1x_init();

	while (true) {
		if (sht1x_status_write(0x00) == SHT1X_ERROR_OK) {
			printf("Status write succeed\n");
		} else {
			printf("Status write failed\n");
		}
	}
}
