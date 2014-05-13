
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "main_task.h"

#include "sht1x.h"


void main_task(void * params)
{
	sht1x_init();

	while (true) {
		if (sht1x_read_status()) {
			printf("Status read succeed\n");
		} else {
			printf("Status read failed\n");
		}
	}
}
