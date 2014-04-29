
#include <stdint.h>
#include <stdbool.h>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>

#include <driverlib/sysctl.h>
#include <driverlib/interrupt.h>
#include <driverlib/timer.h>

#include "sht1x.h"


void timer0a_isr_handler(void)
{
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}


void init_timer0a(void)
{
	/* TODO: use FreeRTOS configuration values to compute timer speed */
	uint32_t period = SysCtlClockGet() / 100;

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	TimerLoadSet(TIMER0_BASE, TIMER_A, period - 1);

	/* TODO: remove IntEnable, IntMasterEnable from here */
	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();

	TimerEnable(TIMER0_BASE, TIMER_A);
}


bool sht1x_init(sht1x_t * self)
{
	init_timer0a();
	return true;
}
