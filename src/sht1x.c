/**
 * SHT1X driver implementation for TM4C123
 */
#include <stdint.h>
#include <stdbool.h>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <inc/hw_gpio.h>

#include <driverlib/sysctl.h>
#include <driverlib/interrupt.h>
#include <driverlib/timer.h>
#include <driverlib/udma.h>
#include <driverlib/gpio.h>

#include "FreeRTOSConfig.h"

#include <FreeRTOS.h>
#include <semphr.h>

#include "sht1x.h"
#include "hw_udma_tbl.h"

/**
 * GPIO PORT Settings
 */
#define SHT1X_GPIO_PERIPH		SYSCTL_PERIPH_GPIOE

#define SHT1X_PORT_BASE			GPIO_PORTE_BASE

#define SHT1X_CLOCK_PIN			GPIO_PIN_1

#define SHT1X_DATA_OUT_PIN		GPIO_PIN_2

#define SHT1X_DATA_IN_PIN		GPIO_PIN_3

#define SHT1X_DATA_INTERRUPT	GPIO_INT_PIN_3

#define SHT1X_GPIO_INTERRUPT	INT_GPIOE

#define SHT1X_OUTPUT_PINS		(SHT1X_CLOCK_PIN | SHT1X_DATA_OUT_PIN)

/**
 * TIMER Settings
 */
#define SHT1X_TIMER_PERIPH		SYSCTL_PERIPH_TIMER0

#define SHT1X_TIMER_BASE		TIMER0_BASE

#define SHT1X_TIMER				TIMER_A

#define SHT1X_TIMER_INTERRUPT	INT_TIMER0A

#define SHT1X_CLOCK_HZ			200000

#define SHT1X_CLOCK_SPEED		(configCPU_CLOCK_HZ / (SHT1X_CLOCK_HZ * 2))

#define SHT1X_CLOCK_WAIT		(configCPU_CLOCK_HZ / 1)

/**
 * uDMA Settings
 */
#define SHT1X_UDMA_PERIPH		SYSCTL_PERIPH_UDMA

#define SHT1X_DST_ADDRESS		((void *)(SHT1X_PORT_BASE + (GPIO_O_DATA + (SHT1X_OUTPUT_PINS << 2))))

#define SHT1X_UDMA_CHANNEL		UDMA_CHANNEL_TMR0A

#define SHT1X_UDMA_CH_NUM		UDMA_DEF_TMR0A_SEC_TMR1A

/**
 * Utility macros
 */
#define sht1x_clock_set()		GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN)
#define sht1x_clock_clear()		GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00)
#define sht1x_data_out_set()	GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_DATA_OUT_PIN, SHT1X_DATA_OUT_PIN)
#define sht1x_data_out_clear()	GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_DATA_OUT_PIN, 0x00)
#define sht1x_data_in_get()		GPIOPinRead(SHT1X_PORT_BASE, SHT1X_DATA_IN_PIN)


inline void sht1x_enable_interrupt()
{
	GPIOIntClear(SHT1X_PORT_BASE, SHT1X_DATA_INTERRUPT);
	GPIOIntEnable(SHT1X_PORT_BASE, SHT1X_DATA_INTERRUPT);
}


inline void sht1x_disable_interrupt()
{
	GPIOIntDisable(SHT1X_PORT_BASE, SHT1X_DATA_INTERRUPT);
}


/**
 * SHT1X Command: measure temperature
 */
static uint8_t measure_temperature_cmd[] = {
	SHT1X_DATA_OUT_PIN | SHT1X_CLOCK_PIN, SHT1X_DATA_OUT_PIN,
	SHT1X_DATA_OUT_PIN | SHT1X_CLOCK_PIN, SHT1X_DATA_OUT_PIN,
	SHT1X_DATA_OUT_PIN | SHT1X_CLOCK_PIN, SHT1X_DATA_OUT_PIN,
	SHT1X_DATA_OUT_PIN | SHT1X_CLOCK_PIN, SHT1X_DATA_OUT_PIN,
	SHT1X_DATA_OUT_PIN | SHT1X_CLOCK_PIN, SHT1X_DATA_OUT_PIN,
	SHT1X_DATA_OUT_PIN | SHT1X_CLOCK_PIN, SHT1X_DATA_OUT_PIN,
	SHT1X_DATA_OUT_PIN | SHT1X_CLOCK_PIN, SHT1X_DATA_OUT_PIN,
	SHT1X_DATA_OUT_PIN | SHT1X_CLOCK_PIN, SHT1X_DATA_OUT_PIN,
	SHT1X_DATA_OUT_PIN | SHT1X_CLOCK_PIN, SHT1X_DATA_OUT_PIN,
	/* transmission start */
	SHT1X_DATA_OUT_PIN | SHT1X_CLOCK_PIN,
	SHT1X_CLOCK_PIN,
	0x00,
	SHT1X_CLOCK_PIN,
	SHT1X_DATA_OUT_PIN | SHT1X_CLOCK_PIN,
	SHT1X_DATA_OUT_PIN,
	0x00,
	/* address */
	0x00, SHT1X_CLOCK_PIN, /* a2 */
	0x00, SHT1X_CLOCK_PIN, /* a1 */
	0x00, SHT1X_CLOCK_PIN, /* a0 */
	/* command 00011 */
	0x00, SHT1X_CLOCK_PIN, /* c4 */
	0x00, SHT1X_CLOCK_PIN, /* c3 */
	0x00, SHT1X_CLOCK_PIN, /* c2 */
	SHT1X_DATA_OUT_PIN, SHT1X_DATA_OUT_PIN | SHT1X_CLOCK_PIN, /* c1 */
	SHT1X_DATA_OUT_PIN, SHT1X_DATA_OUT_PIN | SHT1X_CLOCK_PIN, /* c0 */
};


typedef enum {
	SHT1X_STATE_NUM_END = 0,
	SHT1X_STATE_NUM_CMD = 1,
	SHT1X_STATE_NUM_CMD_FIN = 2,
	SHT1X_STATE_NUM_CMD_ACK_CLOCK = 3,
	SHT1X_STATE_NUM_CMD_ACK_END = 4,
	SHT1X_STATE_NUM_MEASUREMENT_WAIT = 5,
	SHT1X_STATE_NUM_MEASUREMENT_READ = 6,

	SHT1X_STATE_NUM_DATA_15,
	SHT1X_STATE_NUM_DATA_15C,
	SHT1X_STATE_NUM_DATA_14,
	SHT1X_STATE_NUM_DATA_14C,
	SHT1X_STATE_NUM_DATA_13,
	SHT1X_STATE_NUM_DATA_13C,
	SHT1X_STATE_NUM_DATA_12,
	SHT1X_STATE_NUM_DATA_12C,
	SHT1X_STATE_NUM_DATA_11,
	SHT1X_STATE_NUM_DATA_11C,
	SHT1X_STATE_NUM_DATA_10,
	SHT1X_STATE_NUM_DATA_10C,
	SHT1X_STATE_NUM_DATA_9,
	SHT1X_STATE_NUM_DATA_9C,
	SHT1X_STATE_NUM_DATA_8,
	SHT1X_STATE_NUM_DATA_8C,

	SHT1X_STATE_NUM_BYTE2_ACK,
	SHT1X_STATE_NUM_BYTE2_ACKC,

	SHT1X_STATE_NUM_DATA_7,
	SHT1X_STATE_NUM_DATA_7C,
	SHT1X_STATE_NUM_DATA_6,
	SHT1X_STATE_NUM_DATA_6C,
	SHT1X_STATE_NUM_DATA_5,
	SHT1X_STATE_NUM_DATA_5C,
	SHT1X_STATE_NUM_DATA_4,
	SHT1X_STATE_NUM_DATA_4C,
	SHT1X_STATE_NUM_DATA_3,
	SHT1X_STATE_NUM_DATA_3C,
	SHT1X_STATE_NUM_DATA_2,
	SHT1X_STATE_NUM_DATA_2C,
	SHT1X_STATE_NUM_DATA_1,
	SHT1X_STATE_NUM_DATA_1C,
	SHT1X_STATE_NUM_DATA_0,
	SHT1X_STATE_NUM_DATA_0C,

	SHT1X_STATE_NUM_BYTE1_ACK,
	SHT1X_STATE_NUM_BYTE1_ACKC,

	SHT1X_STATE_NUM_CRC_7,
	SHT1X_STATE_NUM_CRC_7C,
	SHT1X_STATE_NUM_CRC_6,
	SHT1X_STATE_NUM_CRC_6C,
	SHT1X_STATE_NUM_CRC_5,
	SHT1X_STATE_NUM_CRC_5C,
	SHT1X_STATE_NUM_CRC_4,
	SHT1X_STATE_NUM_CRC_4C,
	SHT1X_STATE_NUM_CRC_3,
	SHT1X_STATE_NUM_CRC_3C,
	SHT1X_STATE_NUM_CRC_2,
	SHT1X_STATE_NUM_CRC_2C,
	SHT1X_STATE_NUM_CRC_1,
	SHT1X_STATE_NUM_CRC_1C,
	SHT1X_STATE_NUM_CRC_0,
	SHT1X_STATE_NUM_CRC_0C,

	SHT1X_STATE_NUM_CRC_ACK,
	SHT1X_STATE_NUM_CRC_ACKC
} sht1x_state_num_t;


typedef enum {
	SHT1X_ERROR_OK,
	SHT1X_ERROR_CMD,
	SHT1X_ERROR_NO_CMD_PULLDOWN,
	SHT1X_ERROR_NO_CMD_PULLUP,
	SHT1X_ERROR_MEASUREMENT_TIMEOUT,
	SHT1X_ERROR_INVALID_STATE,
	SHT1X_ERROR_UNKNOWN
} sht1x_error_t;


typedef struct {
	sht1x_state_num_t state;
	sht1x_error_t error;
} sht1x_state_t;


static sht1x_state_t device_state;

static xSemaphoreHandle lock = NULL;

static xSemaphoreHandle interrupt_semaphore = NULL;


static uint16_t data;

static uint8_t crc;


/**
 * Initialize GPIO port to support SHT1X I/O operations.
 * Data pin is set in weak pull-up configuration.
 */
void sht1x_gpio_init(void)
{
	SysCtlPeripheralEnable(SHT1X_GPIO_PERIPH);

	GPIOPinTypeGPIOOutput(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN);
	GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);

	GPIODirModeSet(SHT1X_PORT_BASE, SHT1X_DATA_OUT_PIN, GPIO_DIR_MODE_OUT);
	GPIOPadConfigSet(SHT1X_PORT_BASE, SHT1X_DATA_OUT_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
	GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_DATA_OUT_PIN, SHT1X_DATA_OUT_PIN);

	GPIOPinTypeGPIOInput(SHT1X_PORT_BASE, SHT1X_DATA_IN_PIN);
	GPIOIntTypeSet(SHT1X_PORT_BASE, SHT1X_DATA_IN_PIN, GPIO_FALLING_EDGE);
	sht1x_disable_interrupt();

	IntEnable(SHT1X_GPIO_INTERRUPT);
}


void sht1x_timer_start(void)
{
	TimerIntClear(SHT1X_TIMER_BASE, TIMER_TIMA_TIMEOUT);
	TimerIntEnable(SHT1X_TIMER_BASE, TIMER_TIMA_TIMEOUT);
	TimerEnable(SHT1X_TIMER_BASE, SHT1X_TIMER);
}


void sht1x_timer_stop(void)
{
	TimerIntDisable(SHT1X_TIMER_BASE, TIMER_TIMA_TIMEOUT);
	TimerIntClear(SHT1X_TIMER_BASE, TIMER_TIMA_TIMEOUT);
	TimerDisable(SHT1X_TIMER_BASE, SHT1X_TIMER);
}


void sht1x_timer_init(void)
{
	SysCtlPeripheralEnable(SHT1X_TIMER_PERIPH);

	IntDisable(SHT1X_TIMER_INTERRUPT);

	sht1x_timer_stop();

	TimerConfigure(SHT1X_TIMER_BASE, TIMER_CFG_PERIODIC);
	TimerPrescaleSet(SHT1X_TIMER_BASE, SHT1X_TIMER, 0);
	TimerClockSourceSet(SHT1X_TIMER_BASE, TIMER_CLOCK_SYSTEM);
	TimerLoadSet(SHT1X_TIMER_BASE, SHT1X_TIMER, SHT1X_CLOCK_SPEED);

	TimerDMAEventSet(SHT1X_TIMER_BASE, TIMER_DMA_TIMEOUT_A);

	IntEnable(SHT1X_TIMER_INTERRUPT);
}


void sht1x_udma_init(void)
{
	SysCtlPeripheralEnable(SHT1X_UDMA_PERIPH);

	uDMAEnable();
	uDMAControlBaseSet(UDMA_CTL_TBL);

	IntEnable(INT_UDMAERR);

	uDMAChannelAttributeEnable(SHT1X_UDMA_CHANNEL, UDMA_ATTR_USEBURST);
	uDMAChannelAttributeDisable(SHT1X_UDMA_CHANNEL, UDMA_ATTR_ALTSELECT | UDMA_ATTR_HIGH_PRIORITY | UDMA_ATTR_REQMASK);
}


void sht1x_udma_set_buffer(void * data, uint32_t transfer_size)
{
	uDMAChannelControlSet(SHT1X_UDMA_CHANNEL | UDMA_PRI_SELECT, UDMA_SIZE_8 | UDMA_SRC_INC_8 | UDMA_DST_INC_NONE | UDMA_ARB_1);
	uDMAChannelTransferSet(SHT1X_UDMA_CHANNEL | UDMA_PRI_SELECT, UDMA_MODE_BASIC, data, SHT1X_DST_ADDRESS, transfer_size);

	uDMAIntClear(SHT1X_UDMA_CH_NUM);
	uDMAChannelEnable(SHT1X_UDMA_CHANNEL);
}


uint8_t sht1x_read_temperature(void)
{
	xSemaphoreTake(lock, portMAX_DELAY);

	data = 0;
	crc = 0;

	device_state.state = SHT1X_STATE_NUM_CMD;
	device_state.error = SHT1X_ERROR_OK;

	TimerLoadSet(SHT1X_TIMER_BASE, SHT1X_TIMER, SHT1X_CLOCK_SPEED);
	sht1x_disable_interrupt();

	sht1x_udma_set_buffer((void *)measure_temperature_cmd, sizeof(measure_temperature_cmd));
	sht1x_timer_start();

	xSemaphoreTake(interrupt_semaphore, portMAX_DELAY);

	if (device_state.error == SHT1X_ERROR_OK) {
		printf("DATA: %d, CRC: %d\n", data, crc);
		xSemaphoreGive(lock);
		return 1;
	} else {
		xSemaphoreGive(lock);
		return 0;
	}
}


void timer0a_isr_handler(void)
{
	uint32_t udma_status = uDMAIntStatus();

	TimerIntClear(SHT1X_TIMER_BASE, TIMER_TIMA_TIMEOUT);
	uDMAIntClear(udma_status);

	uint32_t value = TimerValueGet(SHT1X_TIMER_BASE, SHT1X_TIMER);

	if (udma_status & SHT1X_UDMA_CH_NUM) {
		if (device_state.state == SHT1X_STATE_NUM_CMD) {
			device_state.state = SHT1X_STATE_NUM_CMD_FIN;
		} else {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);

			device_state.state = SHT1X_STATE_NUM_END;
			device_state.error = SHT1X_ERROR_CMD;

			sht1x_timer_stop();
			xSemaphoreGiveFromISR(interrupt_semaphore, NULL);
		}
	} else {
		if (device_state.state == SHT1X_STATE_NUM_CMD_FIN) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_OUTPUT_PINS, SHT1X_DATA_OUT_PIN);

			device_state.state = SHT1X_STATE_NUM_CMD_ACK_CLOCK;
			device_state.error = SHT1X_ERROR_OK;
		}else if (device_state.state == SHT1X_STATE_NUM_CMD_ACK_CLOCK) {
			uint32_t data = sht1x_data_in_get();

			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			if (!data) {
				device_state.state = SHT1X_STATE_NUM_CMD_ACK_END;
				device_state.error = SHT1X_ERROR_OK;

				sht1x_enable_interrupt();
			} else {
				device_state.state = SHT1X_STATE_NUM_END;
				device_state.error = SHT1X_ERROR_NO_CMD_PULLDOWN;

				GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);

				sht1x_timer_stop();
				xSemaphoreGiveFromISR(interrupt_semaphore, NULL);
			}
		} else if (device_state.state == SHT1X_STATE_NUM_CMD_ACK_END) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_MEASUREMENT_WAIT;
			device_state.error = SHT1X_ERROR_OK;

			TimerLoadSet(SHT1X_TIMER_BASE, SHT1X_TIMER, SHT1X_CLOCK_WAIT);
		} else if (device_state.state == SHT1X_STATE_NUM_MEASUREMENT_WAIT) {
			sht1x_disable_interrupt();

			device_state.state = SHT1X_STATE_NUM_END;
			device_state.error = SHT1X_ERROR_MEASUREMENT_TIMEOUT;

			sht1x_timer_stop();
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			TimerLoadSet(SHT1X_TIMER_BASE, SHT1X_TIMER, SHT1X_CLOCK_SPEED);
			xSemaphoreGiveFromISR(interrupt_semaphore, NULL);
		} else if (device_state.state == SHT1X_STATE_NUM_MEASUREMENT_READ) {
			TimerLoadSet(SHT1X_TIMER_BASE, SHT1X_TIMER, SHT1X_CLOCK_SPEED);
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);

			device_state.state = SHT1X_STATE_NUM_DATA_15;
			device_state.error = SHT1X_ERROR_OK;


		/* DATA STATES */
		} else if (device_state.state == SHT1X_STATE_NUM_DATA_15) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);

			sht1x_data_out_set();

			device_state.state = SHT1X_STATE_NUM_DATA_15C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_DATA_15C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			data |= ((d >> SHT1X_DATA_IN_PIN) & 0x01) << 15;
			device_state.state = SHT1X_STATE_NUM_DATA_14;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_DATA_14) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_DATA_14C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_DATA_14C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			data |= ((d >> SHT1X_DATA_IN_PIN) & 0x01) << 14;
			device_state.state = SHT1X_STATE_NUM_DATA_13;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_DATA_13) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_DATA_13C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_DATA_13C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			data |= ((d >> SHT1X_DATA_IN_PIN) & 0x01) << 13;
			device_state.state = SHT1X_STATE_NUM_DATA_12;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_DATA_12) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_DATA_12C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_DATA_12C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			data |= ((d >> SHT1X_DATA_IN_PIN) & 0x01) << 12;
			device_state.state = SHT1X_STATE_NUM_DATA_11;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_DATA_11) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_DATA_11C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_DATA_11C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			data |= ((d >> SHT1X_DATA_IN_PIN) & 0x01) << 11;
			device_state.state = SHT1X_STATE_NUM_DATA_10;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_DATA_10) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_DATA_10C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_DATA_10C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			data |= ((d >> SHT1X_DATA_IN_PIN) & 0x01) << 10;
			device_state.state = SHT1X_STATE_NUM_DATA_9;
			device_state.error = SHT1X_ERROR_OK;


		} else if (device_state.state == SHT1X_STATE_NUM_DATA_9) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_DATA_9C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_DATA_9C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			data |= ((d >> SHT1X_DATA_IN_PIN) & 0x01) << 9;
			device_state.state = SHT1X_STATE_NUM_DATA_8;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_DATA_8) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_DATA_8C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_DATA_8C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			data |= ((d >> SHT1X_DATA_IN_PIN) & 0x01) << 8;
			device_state.state = SHT1X_STATE_NUM_BYTE2_ACK;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_BYTE2_ACK) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_BYTE2_ACKC;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_BYTE2_ACKC) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			// TODO: test data_in is low
			device_state.state = SHT1X_STATE_NUM_DATA_7;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_DATA_7) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_DATA_7C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_DATA_7C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			data |= ((d >> SHT1X_DATA_IN_PIN) & 0x01) << 7;
			device_state.state = SHT1X_STATE_NUM_DATA_6;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_DATA_6) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_DATA_6C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_DATA_6C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			data |= ((d >> SHT1X_DATA_IN_PIN) & 0x01) << 6;
			device_state.state = SHT1X_STATE_NUM_DATA_5;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_DATA_5) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_DATA_5C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_DATA_5C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			data |= ((d >> SHT1X_DATA_IN_PIN) & 0x01) << 5;
			device_state.state = SHT1X_STATE_NUM_DATA_4;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_DATA_4) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_DATA_4C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_DATA_4C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			data |= ((d >> SHT1X_DATA_IN_PIN) & 0x01) << 4;
			device_state.state = SHT1X_STATE_NUM_DATA_3;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_DATA_3) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_DATA_3C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_DATA_3C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			data |= ((d >> SHT1X_DATA_IN_PIN) & 0x01) << 3;
			device_state.state = SHT1X_STATE_NUM_DATA_2;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_DATA_2) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_DATA_2C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_DATA_2C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			data |= ((d >> SHT1X_DATA_IN_PIN) & 0x01) << 2;
			device_state.state = SHT1X_STATE_NUM_DATA_1;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_DATA_1) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_DATA_1C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_DATA_1C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			data |= ((d >> SHT1X_DATA_IN_PIN) & 0x01) << 1;
			device_state.state = SHT1X_STATE_NUM_DATA_0;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_DATA_0) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_DATA_0C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_DATA_0C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			data |= ((d >> SHT1X_DATA_IN_PIN) & 0x01);
			device_state.state = SHT1X_STATE_NUM_BYTE1_ACK;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_BYTE1_ACK) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_BYTE1_ACKC;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_BYTE1_ACKC) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			// TODO: Test that DATA_IN is low
			device_state.state = SHT1X_STATE_NUM_CRC_7;
			device_state.error = SHT1X_ERROR_OK;

		/* CRC8 */
		} else if (device_state.state == SHT1X_STATE_NUM_CRC_7) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_CRC_7C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_CRC_7C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			crc |= ((d >> SHT1X_DATA_IN_PIN) & 0x01) << 7;
			device_state.state = SHT1X_STATE_NUM_CRC_6;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_CRC_6) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_CRC_6C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_CRC_6C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			crc |= ((d >> SHT1X_DATA_IN_PIN) & 0x01) << 6;
			device_state.state = SHT1X_STATE_NUM_CRC_5;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_CRC_5) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_CRC_5C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_CRC_5C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			crc |= ((d >> SHT1X_DATA_IN_PIN) & 0x01) << 5;
			device_state.state = SHT1X_STATE_NUM_CRC_4;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_CRC_4) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_CRC_4C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_CRC_4C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			crc |= ((d >> SHT1X_DATA_IN_PIN) & 0x01) << 4;
			device_state.state = SHT1X_STATE_NUM_CRC_3;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_CRC_3) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_CRC_3C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_CRC_3C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			crc |= ((d >> SHT1X_DATA_IN_PIN) & 0x01) << 3;
			device_state.state = SHT1X_STATE_NUM_CRC_2;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_CRC_2) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_CRC_2C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_CRC_2C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			crc |= ((d >> SHT1X_DATA_IN_PIN) & 0x01) << 2;
			device_state.state = SHT1X_STATE_NUM_CRC_1;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_CRC_1) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_CRC_1C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_CRC_1C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			crc |= ((d >> SHT1X_DATA_IN_PIN) & 0x01) << 1;
			device_state.state = SHT1X_STATE_NUM_CRC_0;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_CRC_0) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_CRC_0C;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_CRC_0C) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			uint32_t d = sht1x_data_in_get();
			crc |= ((d >> SHT1X_DATA_IN_PIN) & 0x01) << 0;
			device_state.state = SHT1X_STATE_NUM_CRC_ACK;
			device_state.error = SHT1X_ERROR_OK;

		} else if (device_state.state == SHT1X_STATE_NUM_CRC_ACK) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			device_state.state = SHT1X_STATE_NUM_CRC_ACKC;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_CRC_ACKC) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, SHT1X_CLOCK_PIN);
			device_state.state = SHT1X_STATE_NUM_END;
			device_state.error = SHT1X_ERROR_OK;
		} else if (device_state.state == SHT1X_STATE_NUM_END) {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_DATA_OUT_PIN, SHT1X_DATA_OUT_PIN);

			device_state.state = SHT1X_STATE_NUM_END;
			device_state.error = SHT1X_ERROR_OK;

			sht1x_timer_stop();
			xSemaphoreGiveFromISR(interrupt_semaphore, NULL);
		/* Fail state */
		} else {
			GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_CLOCK_PIN, 0x00);

			device_state.state = SHT1X_STATE_NUM_END;
			device_state.error = SHT1X_ERROR_INVALID_STATE;

			sht1x_timer_stop();
			xSemaphoreGiveFromISR(interrupt_semaphore, NULL);
		}
	}
}


void gpioe_isr_handler(void)
{
	uint32_t interrupt_status = GPIOIntStatus(SHT1X_PORT_BASE, false);
	GPIOIntClear(SHT1X_PORT_BASE, interrupt_status);

	if (interrupt_status & SHT1X_DATA_INTERRUPT) {
		sht1x_disable_interrupt();

		if (device_state.state == SHT1X_STATE_NUM_MEASUREMENT_WAIT) {
			uint32_t data = sht1x_data_in_get();
			if (!data) {
				device_state.state = SHT1X_STATE_NUM_MEASUREMENT_READ;
				device_state.error = SHT1X_ERROR_OK;
				return;
			}
		}
	}
}


void udma_error_isr_handler(void)
{
	uint32_t udma_error = uDMAErrorStatusGet();
	uint32_t udma_status = uDMAIntStatus();
	uDMAIntClear(udma_status);

	if (udma_error) {
		uDMAErrorStatusClear();
	}

	xSemaphoreGiveFromISR(interrupt_semaphore, NULL);
}


void sht1x_init(void)
{
	lock = xSemaphoreCreateMutex();
	interrupt_semaphore = xSemaphoreCreateBinary();

	sht1x_gpio_init();

	sht1x_timer_init();
	sht1x_udma_init();
}
