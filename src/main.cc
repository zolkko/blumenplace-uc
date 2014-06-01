
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "sys.h"
#include "main_task.h"
#include "test.h"


int main(void)
{
	xTaskHandle main_task_handle;

	sys_init();

	test_t demo;
	demo.print();

	if (pdPASS != xTaskCreate(main_task, (const char *)MAIN_TASK_NAME, MAIN_TASK_STACK, NULL, MAIN_TASK_PRIORITY, &main_task_handle)) {
		goto reset_controller;
	}

	portENABLE_INTERRUPTS();
	vTaskStartScheduler();

stop_main_task:
	vTaskDelete(&main_task_handle);
	main_task_handle = NULL;

reset_controller:
	while (true) ;
}
