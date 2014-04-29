#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>

#include <driverlib/sysctl.h>
#include <driverlib/systick.h>
#include <driverlib/interrupt.h>
#include <driverlib/timer.h>

#include "FreeRTOS.h"
#include "task.h"

#include "main_task.h"


void timer0a_isr_handler(void);
void init_sys(void);

void timer0a_isr_handler(void)
{
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	printf("Time 0 interrupt\n");
}

/**
 * Main clock is 80MHz PLL.
 */
void init_sys(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
}


void init_timer0a(void)
{
	uint32_t period = SysCtlClockGet() / 10 / 2;

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	TimerLoadSet(TIMER0_BASE, TIMER_A, period - 1);

	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();

	TimerEnable(TIMER0_BASE, TIMER_A);
}


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

	init_sys();

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
