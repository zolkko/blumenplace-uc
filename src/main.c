#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "sys.h"
#include "main_task.h"
#include "sht1x.h"


void second_task(void * params)
{
	int i = 0;
	while (true) {
		if (i > 1000) {
			i = 600;
		}
	}
}


int main(void)
{
	xTaskHandle main_task_handle;

	sys_init();
	sht1x_init(NULL);

    if (pdPASS != xTaskCreate(main_task, (const char *)MAIN_TASK_NAME, MAIN_TASK_STACK, NULL, MAIN_TASK_PRIORITY, &main_task_handle)) {
        goto reset_controller;
    }

    if (pdPASS != xTaskCreate(second_task, (const char *)"second-task", 512, NULL, 1, NULL)) {
    	goto stop_main_task;
    }

	vTaskStartScheduler();

stop_main_task:
	vTaskDelete(&main_task_handle);
	main_task_handle = NULL;

reset_controller:
	while (true) ;
}
