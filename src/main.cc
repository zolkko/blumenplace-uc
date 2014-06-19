
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "sys.h"
#include "main_task.h"

const uint32_t constant = 123;

class demo_t {
public:
	demo_t(const uint32_t d_) : d(d_) {
	}

	void inc(void) {
		d += demo_t::inc_def;
	}
private:
	static const uint32_t inc_def = 1;
	uint32_t d;
};

int main(void)
{
	int a = 1;
	int b = 2;
	int c = a + b + constant;

	demo_t demo(c);
	while (true) {
		demo.inc();
	}
	/*xTaskHandle main_task_handle;

	sys_init();

	if (pdPASS != xTaskCreate(main_task, (const char *)MAIN_TASK_NAME, MAIN_TASK_STACK, NULL, MAIN_TASK_PRIORITY, &main_task_handle)) {
		goto reset_controller;
	}

	portENABLE_INTERRUPTS();
	vTaskStartScheduler();

stop_main_task:
	vTaskDelete(&main_task_handle);
	main_task_handle = NULL;

reset_controller:
	while (true) ;*/
}
