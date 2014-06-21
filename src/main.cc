
#include "FreeRTOS.h"
#include "task.h"

#include "main_task.h"


int main(void)
{
	xTaskHandle main_task_handle;

	if (pdPASS != xTaskCreate(main_task, (const char *)MAIN_TASK_NAME, MAIN_TASK_STACK, NULL, MAIN_TASK_PRIORITY, &main_task_handle)) {
		goto reset_controller;
	}

	portENABLE_INTERRUPTS();
	vTaskStartScheduler();

//stop_main_task:
//	vTaskDelete(&main_task_handle);
//	main_task_handle = NULL;

reset_controller:
	while (true) ;
}
