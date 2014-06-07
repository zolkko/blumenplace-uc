
#include <stdbool.h>
#include <stdint.h>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <inc/hw_gpio.h>
#include <inc/hw_timer.h>

#include "inc/hw_memmap.h"
#include "inc/hw_ssi.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/interrupt.h"
#include "driverlib/ssi.h"

#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/udma.h"

#include <FreeRTOS.h>
#include <semphr.h>

#include "gpio_pin.h"
#include "gpio_port.h"
#include "ssi_dev.h"

extern "C" {
	void ssi0_isr_handler(void);
}

void ssi0_isr_handler(void)
{
	ssi_dev_t<SSI0_BASE>::dispatch_isr();
}
