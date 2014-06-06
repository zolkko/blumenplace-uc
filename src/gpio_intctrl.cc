#include <stdint.h>
#include <stdbool.h>
#include <vector>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <inc/hw_gpio.h>

#include "gpio_intctrl.h"


void gpioa_isr_handler(void)
{
	// gpio_intctrl_t<GPIO_PORTA_BASE>::get().handle_isr();
}
