
#include <stdint.h>
#include <stdbool.h>

#include <memory>
#include <vector>

#include "FreeRTOSConfig.h"

#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include <driverlib/ssi.h>
#include <driverlib/gpio.h>
#include <driverlib/udma.h>
#include <driverlib/sysctl.h>
#include <driverlib/interrupt.h>

#include "main_task.h"
#include "gpio_pin.h"
#include "gpio_port.h"
#include "ssi_dev.h"
#include "sht1x.h"
#include "ccx.h"


uint8_t in_data[20] __attribute__ ((aligned(32)));
uint8_t out_data[50] __attribute__ ((aligned(32)));


void main_task(void * params)
{
	gpio_port_t<GPIO_PORTA_BASE, SYSCTL_PERIPH_GPIOA> porta;
	porta.enable();

	gpio_port_t<GPIO_PORTE_BASE, SYSCTL_PERIPH_GPIOE> porte;
	porte.enable();

	std::unique_ptr<gpio_pin_t> gdo0_pin_ptr = std::unique_ptr<gpio_pin_t>(porta.get_pin(GPIO_PIN_7));
	gdo0_pin_ptr->set_mode_input();

	std::unique_ptr<gpio_pin_t> gdo2_pin_ptr = std::unique_ptr<gpio_pin_t>(porta.get_pin(GPIO_PIN_6));
	gdo2_pin_ptr->set_mode_input();

	std::unique_ptr<gpio_pin_t> gdo1_pin_ptr = std::unique_ptr<gpio_pin_t>(porte.get_pin(GPIO_PIN_5));
	gdo1_pin_ptr->set_mode_input();

	dma_dev_t dma;

	ssi_dev_t<SSI0_BASE> ssi0(dma);
	ssi0.flush();

	ccx_t ccx(ssi0, *gdo0_pin_ptr, *gdo1_pin_ptr, *gdo2_pin_ptr);

	while (true) {
		if (ccx.reset()) {
			ccx.part_number();
		}
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
