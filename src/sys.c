
#include <stdint.h>
#include <stdbool.h>
#include <driverlib/sysctl.h>

#include "sys.h"


/**
 * Main clock is 80MHz PLL.
 */
void sys_init(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
}
