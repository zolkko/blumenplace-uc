/**
 * SHT1X driver implementation for TM4C123
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <inc/hw_gpio.h>
#include <inc/hw_timer.h>

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

#define SHT1X_DIN_PIN_bp		(3)

#define SHT1X_DATA_INTERRUPT	GPIO_INT_PIN_3

#define SHT1X_GPIO_INTERRUPT	INT_GPIOE

#define SHT1X_OUTPUT_PINS		(SHT1X_CLOCK_PIN | SHT1X_DATA_OUT_PIN)

#define SHT1X_BOTH_PINS			(SHT1X_CLOCK_PIN | SHT1X_DATA_OUT_PIN)

#define DEBUG_PIN				GPIO_PIN_4

/**
 * TIMER Settings
 */
#define SHT1X_TIMER_PERIPH		SYSCTL_PERIPH_TIMER0

#define SHT1X_TIMER_BASE		TIMER0_BASE

#define SHT1X_TIMER				TIMER_A

#define SHT1X_TIMER_INTERRUPT	INT_TIMER0A

#define SHT1X_CLOCK_HZ			50000

/* Timer normal speed  */
#define SHT1X_CLK_NR			(configCPU_CLOCK_HZ / (SHT1X_CLOCK_HZ * 2))

/* Timer slow speed is used to create timeout operation */
#define SHT1X_CLK_LO			(configCPU_CLOCK_HZ / 1)


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

/**
 * State index
 */
typedef enum {
	SHT1X_SIDX_END = 0,

	SHT1X_SIDX_CMD = 1,
	SHT1X_SIDX_CMD_ACK = 2,
	SHT1X_SIDX_CMD_ACKC = 3,
	SHT1X_SIDX_CMD_ACKE = 4,
	SHT1X_SIDX_MWAIT = 5,

	SHT1X_SIDX_DATA_15 = 6,
	SHT1X_SIDX_DATA_15C = 7,
	SHT1X_SIDX_DATA_14 = 8,
	SHT1X_SIDX_DATA_14C = 9,
	SHT1X_SIDX_DATA_13 = 10,
	SHT1X_SIDX_DATA_13C = 11,
	SHT1X_SIDX_DATA_12 = 12,
	SHT1X_SIDX_DATA_12C = 13,
	SHT1X_SIDX_DATA_11 = 14,
	SHT1X_SIDX_DATA_11C = 15,
	SHT1X_SIDX_DATA_10 = 16,
	SHT1X_SIDX_DATA_10C = 17,
	SHT1X_SIDX_DATA_9 = 18,
	SHT1X_SIDX_DATA_9C = 19,
	SHT1X_SIDX_DATA_8 = 20,
	SHT1X_SIDX_DATA_8C = 21,

	SHT1X_SIDX_BYTE2_ACKS = 22,
	SHT1X_SIDX_BYTE2_ACK = 23,
	SHT1X_SIDX_BYTE2_ACKC = 24,

	SHT1X_SIDX_DATA_7 = 25,
	SHT1X_SIDX_DATA_7C = 26,
	SHT1X_SIDX_DATA_6 = 27,
	SHT1X_SIDX_DATA_6C = 28,
	SHT1X_SIDX_DATA_5 = 29,
	SHT1X_SIDX_DATA_5C = 30,
	SHT1X_SIDX_DATA_4 = 31,
	SHT1X_SIDX_DATA_4C = 32,
	SHT1X_SIDX_DATA_3 = 33,
	SHT1X_SIDX_DATA_3C = 34,
	SHT1X_SIDX_DATA_2 = 35,
	SHT1X_SIDX_DATA_2C = 36,
	SHT1X_SIDX_DATA_1 = 37,
	SHT1X_SIDX_DATA_1C = 38,
	SHT1X_SIDX_DATA_0 = 39,
	SHT1X_SIDX_DATA_0C = 40,

	SHT1X_SIDX_BYTE1_ACKS = 41,
	SHT1X_SIDX_BYTE1_ACK = 42,
	SHT1X_SIDX_BYTE1_ACKC = 43,

	SHT1X_SIDX_CRC_7 = 44,
	SHT1X_SIDX_CRC_7C = 45,
	SHT1X_SIDX_CRC_6 = 46,
	SHT1X_SIDX_CRC_6C = 47,
	SHT1X_SIDX_CRC_5 = 48,
	SHT1X_SIDX_CRC_5C = 49,
	SHT1X_SIDX_CRC_4 = 50,
	SHT1X_SIDX_CRC_4C = 51,
	SHT1X_SIDX_CRC_3 = 52,
	SHT1X_SIDX_CRC_3C = 53,
	SHT1X_SIDX_CRC_2 = 54,
	SHT1X_SIDX_CRC_2C = 55,
	SHT1X_SIDX_CRC_1 = 56,
	SHT1X_SIDX_CRC_1C = 57,
	SHT1X_SIDX_CRC_0 = 58,
	SHT1X_SIDX_CRC_0C = 59,

	SHT1X_SIDX_CRC_ACK = 60,
	SHT1X_SIDX_CRC_ACKC = 61,

	SHT1X_SIDX_FAIL = 62,

	/* Status Register Read */
	SHT1X_SIDX_SREGR_ACK,
	SHT1X_SIDX_SREGR_ACKC,
	SHT1X_SIDX_SREGR_ACKE,

	SHT1X_SIDX_SREGR_DATA_7,
	SHT1X_SIDX_SREGR_DATA_7C,
	SHT1X_SIDX_SREGR_DATA_6,
	SHT1X_SIDX_SREGR_DATA_6C,
	SHT1X_SIDX_SREGR_DATA_5,
	SHT1X_SIDX_SREGR_DATA_5C,
	SHT1X_SIDX_SREGR_DATA_4,
	SHT1X_SIDX_SREGR_DATA_4C,
	SHT1X_SIDX_SREGR_DATA_3,
	SHT1X_SIDX_SREGR_DATA_3C,
	SHT1X_SIDX_SREGR_DATA_2,
	SHT1X_SIDX_SREGR_DATA_2C,
	SHT1X_SIDX_SREGR_DATA_1,
	SHT1X_SIDX_SREGR_DATA_1C,
	SHT1X_SIDX_SREGR_DATA_0,
	SHT1X_SIDX_SREGR_DATA_0C,

	SHT1X_SIDX_SREGR_DATA_ACKS,
	SHT1X_SIDX_SREGR_DATA_ACK,
	SHT1X_SIDX_SREGR_DATA_ACKC,

	SHT1X_SIDX_SREGR_CRC_7,
	SHT1X_SIDX_SREGR_CRC_7C,
	SHT1X_SIDX_SREGR_CRC_6,
	SHT1X_SIDX_SREGR_CRC_6C,
	SHT1X_SIDX_SREGR_CRC_5,
	SHT1X_SIDX_SREGR_CRC_5C,
	SHT1X_SIDX_SREGR_CRC_4,
	SHT1X_SIDX_SREGR_CRC_4C,
	SHT1X_SIDX_SREGR_CRC_3,
	SHT1X_SIDX_SREGR_CRC_3C,
	SHT1X_SIDX_SREGR_CRC_2,
	SHT1X_SIDX_SREGR_CRC_2C,
	SHT1X_SIDX_SREGR_CRC_1,
	SHT1X_SIDX_SREGR_CRC_1C,
	SHT1X_SIDX_SREGR_CRC_0,
	SHT1X_SIDX_SREGR_CRC_0C,

	SHT1X_SIDX_SREGR_CRC_ACKS,
	SHT1X_SIDX_SREGR_CRC_ACK,
	SHT1X_SIDX_SREGR_CRC_ACKC,

	/* Status Register Write */
	SHT1X_SIDX_SREGW_ACK,
	SHT1X_SIDX_SREGW_ACKC,
	SHT1X_SIDX_SREGW_ACKE,

	SHT1X_SIDX_SREGW_PAYLOAD,
	SHT1X_SIDX_SREGW_PAYLOAD_ACK,
	SHT1X_SIDX_SREGW_PAYLOAD_ACKC,
	SHT1X_SIDX_SREGW_PAYLOAD_ACKE,
} sht1x_sidx_t;

/**
 * Types of output which a state produces.
 */
typedef enum {
	SHT1X_SOUT_NONE = 0,
	SHT1X_SOUT_GPIO = (1 << 1),
	SHT1X_SOUT_DATA = (1 << 2),
	SHT1X_SOUT_CRC = (1 << 3),
	SHT1X_SOUT_TSPEED = (1 << 4),
	SHT1X_SOUT_TSTOP = (1 << 5),
	SHT1X_SOUT_TSTART = (1 << 6),
	SHT1X_SOUT_ISTART = (1 << 7),
	SHT1X_SOUT_ISTOP = (1 << 8),
	SHT1X_SOUT_DMA = (1 << 9),
	SHT1X_SOUT_DONE= (1 << 10)
} sht1x_sout_t;


typedef struct {
	sht1x_sidx_t state;
	sht1x_error_t error;

	uint16_t data;
	uint8_t crc;

	uint8_t cmd[64];
	uint8_t cmd_payload[16];
} sht1x_device_t;


typedef struct {
	uint32_t output;

	uint32_t gpio;
	uint32_t speed;
	uint8_t * dma_buff;
	size_t dma_items_count;

	sht1x_sidx_t next_state[2];
	sht1x_error_t next_error[2];
} sht1x_state_t;

/**
 * Transmission start and Register address sequence pattern
 */
static uint8_t trans_start_pattern[] = {
	/* reset sequence */
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
};

/**
 * The bit pattern for "Measure Temperature" command.
 * 0x00011
 */
static uint8_t measure_temperature_cmd[] = {
	0x00, SHT1X_CLOCK_PIN, /* c4 */
	0x00, SHT1X_CLOCK_PIN, /* c3 */
	0x00, SHT1X_CLOCK_PIN, /* c2 */
	SHT1X_DATA_OUT_PIN, SHT1X_DATA_OUT_PIN | SHT1X_CLOCK_PIN, /* c1 */
	SHT1X_DATA_OUT_PIN, SHT1X_DATA_OUT_PIN | SHT1X_CLOCK_PIN, /* c0 */
};

/**
 * The bit pattern for "Measure Relative Humidity" command.
 * 0x00101
 */
static uint8_t measure_moisture_cmd[] = {
	0x00, SHT1X_CLOCK_PIN, /* c4 */
	0x00, SHT1X_CLOCK_PIN, /* c3 */
	0x00, SHT1X_CLOCK_PIN, /* c2 */
	SHT1X_DATA_OUT_PIN, SHT1X_DATA_OUT_PIN | SHT1X_CLOCK_PIN, /* c1 */
	SHT1X_DATA_OUT_PIN, SHT1X_DATA_OUT_PIN | SHT1X_CLOCK_PIN, /* c0 */
};

/**
 * The bit pattern for "Status Register Read" command.
 * 0b00111
 */
static uint8_t sreg_read_pattern[] = {
	0x00, SHT1X_CLOCK_PIN, /* c4 */
	0x00, SHT1X_CLOCK_PIN, /* c3 */
	SHT1X_DATA_OUT_PIN, SHT1X_BOTH_PINS, /* c2 */
	SHT1X_DATA_OUT_PIN, SHT1X_BOTH_PINS, /* c1 */
	SHT1X_DATA_OUT_PIN, SHT1X_BOTH_PINS, /* c3 */
};

/**
 * The bit pattern for "Status Register Read" command.
 * 0b00110
 */
static const uint8_t sreg_write_pattern[] = {
	/* command */
	0x00, SHT1X_CLOCK_PIN, /* c4 */
	0x00, SHT1X_CLOCK_PIN, /* c3 */
	SHT1X_DATA_OUT_PIN, SHT1X_BOTH_PINS, /* c2 */
	SHT1X_DATA_OUT_PIN, SHT1X_BOTH_PINS, /* c1 */
	0x00, SHT1X_CLOCK_PIN, /* c0 */
};


static sht1x_device_t device;


static sht1x_state_t states[] = {
	/* SHT1X_SIDX_END */		{SHT1X_SOUT_GPIO | SHT1X_SOUT_ISTOP |
								 SHT1X_SOUT_TSTOP | SHT1X_SOUT_DONE,	SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_END, SHT1X_SIDX_END}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},

	/* SHT1X_SIDX_CMD */		{SHT1X_SOUT_NONE,						0,					0,	NULL,	0,	{SHT1X_SIDX_CMD_ACK, SHT1X_SIDX_CMD_ACK}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_CMD_ACK */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_CMD_ACKC, SHT1X_SIDX_CMD_ACKC}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_CMD_ACKC */	{SHT1X_SOUT_GPIO,						SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_CMD_ACKE, SHT1X_SIDX_FAIL}, {SHT1X_ERROR_OK, SHT1X_ERROR_NO_CMD_ACK}},
	/* SHT1X_SIDX_CMD_ACKE */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_TSPEED |
								 SHT1X_SOUT_ISTART,						SHT1X_DATA_OUT_PIN,	SHT1X_CLK_LO,	NULL,	0, {SHT1X_SIDX_MWAIT, SHT1X_SIDX_MWAIT}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_MWAIT */		{SHT1X_SOUT_GPIO | SHT1X_SOUT_ISTOP |
								 SHT1X_SOUT_TSPEED,						0,					SHT1X_CLK_NR,	NULL,	0,	{SHT1X_SIDX_DATA_15, SHT1X_SIDX_DATA_15}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},

	/* SHT1X_SIDX_DATA_15 */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_DATA_15C, SHT1X_SIDX_DATA_15C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_15C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_DATA,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_DATA_14, SHT1X_SIDX_DATA_14}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_14 */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_DATA_14C, SHT1X_SIDX_DATA_14C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_14C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_DATA,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_DATA_13, SHT1X_SIDX_DATA_13}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_13 */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_DATA_13C, SHT1X_SIDX_DATA_13C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_13C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_DATA,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_DATA_12, SHT1X_SIDX_DATA_12}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_12 */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_DATA_12C, SHT1X_SIDX_DATA_12C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_12C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_DATA,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_DATA_11, SHT1X_SIDX_DATA_11}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_11 */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_DATA_11C, SHT1X_SIDX_DATA_11C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_11C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_DATA,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_DATA_10, SHT1X_SIDX_DATA_10}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_10 */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_DATA_10C, SHT1X_SIDX_DATA_10C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_10C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_DATA,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_DATA_9, SHT1X_SIDX_DATA_9}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_9 */		{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_DATA_9C, SHT1X_SIDX_DATA_9C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_9C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_DATA,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_DATA_8, SHT1X_SIDX_DATA_8}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_8 */		{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_DATA_8C, SHT1X_SIDX_DATA_8C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_8C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_DATA,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_BYTE2_ACKS, SHT1X_SIDX_BYTE2_ACKS}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},

	/* SHT1X_SIDX_BYTE2_ACKS */ {SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN, 0,	NULL,	0,	{SHT1X_SIDX_BYTE2_ACK, SHT1X_SIDX_BYTE2_ACK}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_BYTE2_ACK */	{SHT1X_SOUT_GPIO,						0x00,				0,	NULL,	0,	{SHT1X_SIDX_BYTE2_ACKC, SHT1X_SIDX_BYTE2_ACKC}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_BYTE2_ACKC */	{SHT1X_SOUT_GPIO,						SHT1X_CLOCK_PIN,	0,	NULL,	0,	{SHT1X_SIDX_DATA_7, SHT1X_SIDX_DATA_7}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},

	/* SHT1X_SIDX_DATA_7 */		{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_DATA_7C, SHT1X_SIDX_DATA_7C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_7C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_DATA,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_DATA_6, SHT1X_SIDX_DATA_6}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_6 */		{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_DATA_6C, SHT1X_SIDX_DATA_6C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_6C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_DATA,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_DATA_5, SHT1X_SIDX_DATA_5}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_5 */		{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_DATA_5C, SHT1X_SIDX_DATA_5C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_5C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_DATA,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_DATA_4, SHT1X_SIDX_DATA_4}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_4 */		{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_DATA_4C, SHT1X_SIDX_DATA_4C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_4C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_DATA,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_DATA_3, SHT1X_SIDX_DATA_3}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_3 */		{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_DATA_3C, SHT1X_SIDX_DATA_3C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_3C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_DATA,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_DATA_2, SHT1X_SIDX_DATA_2}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_2 */		{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_DATA_2C, SHT1X_SIDX_DATA_2C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_2C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_DATA,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_DATA_1, SHT1X_SIDX_DATA_1}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_1 */		{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_DATA_1C, SHT1X_SIDX_DATA_1C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_1C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_DATA,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_DATA_0, SHT1X_SIDX_DATA_0}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_0 */		{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_DATA_0C, SHT1X_SIDX_DATA_0C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_DATA_0C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_DATA,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_BYTE1_ACKS, SHT1X_SIDX_BYTE1_ACKS}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},

	/* SHT1X_SIDX_BYTE1_ACKS */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_BYTE1_ACK, SHT1X_SIDX_BYTE1_ACK}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_BYTE1_ACK */	{SHT1X_SOUT_GPIO,						0x00,				0,	NULL,	0,	{SHT1X_SIDX_BYTE1_ACKC, SHT1X_SIDX_BYTE1_ACKC}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_BYTE1_ACKC */	{SHT1X_SOUT_GPIO,						SHT1X_CLOCK_PIN,	0,	NULL,	0,	{SHT1X_SIDX_CRC_7, SHT1X_SIDX_CRC_7}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},

	/* SHT1X_SIDX_CRC_7 */		{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_CRC_7C, SHT1X_SIDX_CRC_7C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_CRC_7C */		{SHT1X_SOUT_GPIO | SHT1X_SOUT_CRC,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_CRC_6, SHT1X_SIDX_CRC_6}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_CRC_6 */		{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_CRC_6C, SHT1X_SIDX_CRC_6C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_CRC_6C */		{SHT1X_SOUT_GPIO | SHT1X_SOUT_CRC,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_CRC_5, SHT1X_SIDX_CRC_5}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_CRC_5 */		{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_CRC_5C, SHT1X_SIDX_CRC_5C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_CRC_5C */		{SHT1X_SOUT_GPIO | SHT1X_SOUT_CRC,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_CRC_4, SHT1X_SIDX_CRC_4}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_CRC_4 */		{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_CRC_4C, SHT1X_SIDX_CRC_4C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_CRC_4C */		{SHT1X_SOUT_GPIO | SHT1X_SOUT_CRC,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_CRC_3, SHT1X_SIDX_CRC_3}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_CRC_3 */		{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_CRC_3C, SHT1X_SIDX_CRC_3C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_CRC_3C */		{SHT1X_SOUT_GPIO | SHT1X_SOUT_CRC,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_CRC_2, SHT1X_SIDX_CRC_2}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_CRC_2 */		{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_CRC_2C, SHT1X_SIDX_CRC_2C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_CRC_2C */		{SHT1X_SOUT_GPIO | SHT1X_SOUT_CRC,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_CRC_1, SHT1X_SIDX_CRC_1}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_CRC_1 */		{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_CRC_1C, SHT1X_SIDX_CRC_1C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_CRC_1C */		{SHT1X_SOUT_GPIO | SHT1X_SOUT_CRC,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_CRC_0, SHT1X_SIDX_CRC_0}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_CRC_0 */		{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_CRC_0C, SHT1X_SIDX_CRC_0C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_CRC_0C */		{SHT1X_SOUT_GPIO | SHT1X_SOUT_CRC,		SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_CRC_ACK, SHT1X_SIDX_CRC_ACK}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},

	/* SHT1X_SIDX_CRC_ACK */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_CRC_ACKC, SHT1X_SIDX_CRC_ACKC}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_CRC_ACKC */	{SHT1X_SOUT_GPIO,						SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_END, SHT1X_SIDX_END}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},

	/* SHT1X_SIDX_FAIL */		{SHT1X_SOUT_GPIO | SHT1X_SOUT_TSTOP |
								 SHT1X_SOUT_ISTOP | SHT1X_SOUT_DONE,	SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_FAIL, SHT1X_SIDX_FAIL}, {SHT1X_ERROR_UNKNOWN, SHT1X_ERROR_UNKNOWN}},

	/* Status Register Read */
	/* SHT1X_SIDX_SREGR_ACK */		{SHT1X_SOUT_GPIO |
									 SHT1X_SOUT_TSTART,	SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_SREGR_ACKC, SHT1X_SIDX_SREGR_ACKC}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_ACKC */		{SHT1X_SOUT_GPIO,	SHT1X_BOTH_PINS,	0,	NULL,	0,	{SHT1X_SIDX_SREGR_ACKE, SHT1X_SIDX_FAIL}, {SHT1X_ERROR_OK, SHT1X_ERROR_NO_CMD_ACK}},
	/* SHT1X_SIDX_SREGR_ACKE */		{SHT1X_SOUT_GPIO,	SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_SREGR_DATA_7, SHT1X_SIDX_SREGR_DATA_7}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},

	/* SHT1X_SIDX_SREGR_DATA_7 */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_SREGR_DATA_7C, SHT1X_SIDX_SREGR_DATA_7C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_DATA_7C */	{SHT1X_SOUT_GPIO |
									 SHT1X_SOUT_DATA,	SHT1X_BOTH_PINS,	0,				NULL,	0,	{SHT1X_SIDX_SREGR_DATA_6, SHT1X_SIDX_SREGR_DATA_6}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_DATA_6 */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,NULL,	0,	{SHT1X_SIDX_SREGR_DATA_6C, SHT1X_SIDX_SREGR_DATA_6C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_DATA_6C */	{SHT1X_SOUT_GPIO |
									 SHT1X_SOUT_DATA,	SHT1X_BOTH_PINS,	0,					NULL,	0,	{SHT1X_SIDX_SREGR_DATA_5, SHT1X_SIDX_SREGR_DATA_5}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_DATA_5 */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_SREGR_DATA_5C, SHT1X_SIDX_SREGR_DATA_5C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_DATA_5C */	{SHT1X_SOUT_GPIO |
									 SHT1X_SOUT_DATA,	SHT1X_BOTH_PINS,	0,					NULL,	0,	{SHT1X_SIDX_SREGR_DATA_4, SHT1X_SIDX_SREGR_DATA_4}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_DATA_4 */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_SREGR_DATA_4C, SHT1X_SIDX_SREGR_DATA_4C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_DATA_4C */	{SHT1X_SOUT_GPIO |
									SHT1X_SOUT_DATA,	SHT1X_BOTH_PINS,	0,					NULL,	0,	{SHT1X_SIDX_SREGR_DATA_3, SHT1X_SIDX_SREGR_DATA_3}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_DATA_3 */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_SREGR_DATA_3C, SHT1X_SIDX_SREGR_DATA_3C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_DATA_3C */	{SHT1X_SOUT_GPIO |
									SHT1X_SOUT_DATA,	SHT1X_BOTH_PINS,	0,					NULL,	0,	{SHT1X_SIDX_SREGR_DATA_2, SHT1X_SIDX_SREGR_DATA_2}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_DATA_2 */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_SREGR_DATA_2C, SHT1X_SIDX_SREGR_DATA_2C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_DATA_2C */	{SHT1X_SOUT_GPIO |
									 SHT1X_SOUT_DATA,	SHT1X_BOTH_PINS,	0,					NULL,	0,	{SHT1X_SIDX_SREGR_DATA_1, SHT1X_SIDX_SREGR_DATA_1}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_DATA_1 */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_SREGR_DATA_1C, SHT1X_SIDX_SREGR_DATA_1C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_DATA_1C */	{SHT1X_SOUT_GPIO |
									 SHT1X_SOUT_DATA,	SHT1X_BOTH_PINS,	0,					NULL,	0,	{SHT1X_SIDX_SREGR_DATA_0, SHT1X_SIDX_SREGR_DATA_0}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_DATA_0 */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_SREGR_DATA_0C, SHT1X_SIDX_SREGR_DATA_0C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_DATA_0C */	{SHT1X_SOUT_GPIO |
									 SHT1X_SOUT_DATA,	SHT1X_BOTH_PINS,	0,					NULL,	0,	{SHT1X_SIDX_SREGR_DATA_ACKS, SHT1X_SIDX_SREGR_DATA_ACKS}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},

	/* SHT1X_SIDX_SREGR_DATA_ACKS */{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,	NULL,	0,	{SHT1X_SIDX_SREGR_DATA_ACK, SHT1X_SIDX_SREGR_DATA_ACK}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_DATA_ACK */	{SHT1X_SOUT_GPIO,						0x00,				0,	NULL,	0,	{SHT1X_SIDX_SREGR_DATA_ACKC, SHT1X_SIDX_SREGR_DATA_ACKC}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_DATA_ACKC */{SHT1X_SOUT_GPIO,						SHT1X_CLOCK_PIN,	0,	NULL,	0,	{SHT1X_SIDX_SREGR_CRC_7, SHT1X_SIDX_SREGR_CRC_7}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},

	/* SHT1X_SIDX_SREGR_CRC_7 */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,				NULL,	0,	{SHT1X_SIDX_SREGR_CRC_7C, SHT1X_SIDX_SREGR_CRC_7C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_CRC_7C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_CRC,		SHT1X_BOTH_PINS,	0,				NULL,	0,	{SHT1X_SIDX_SREGR_CRC_6, SHT1X_SIDX_SREGR_CRC_6}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_CRC_6 */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,				NULL,	0,	{SHT1X_SIDX_SREGR_CRC_6C, SHT1X_SIDX_SREGR_CRC_6C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_CRC_6C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_CRC,		SHT1X_BOTH_PINS,	0,				NULL,	0,	{SHT1X_SIDX_SREGR_CRC_5, SHT1X_SIDX_SREGR_CRC_5}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_CRC_5 */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,				NULL,	0,	{SHT1X_SIDX_SREGR_CRC_5C, SHT1X_SIDX_SREGR_CRC_5C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_CRC_5C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_CRC,		SHT1X_BOTH_PINS,	0,				NULL,	0,	{SHT1X_SIDX_SREGR_CRC_4, SHT1X_SIDX_SREGR_CRC_4}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_CRC_4 */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,				NULL,	0,	{SHT1X_SIDX_SREGR_CRC_4C, SHT1X_SIDX_SREGR_CRC_4C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_CRC_4C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_CRC,		SHT1X_BOTH_PINS,	0,				NULL,	0,	{SHT1X_SIDX_SREGR_CRC_3, SHT1X_SIDX_SREGR_CRC_3}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_CRC_3 */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,				NULL,	0,	{SHT1X_SIDX_SREGR_CRC_3C, SHT1X_SIDX_SREGR_CRC_3C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_CRC_3C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_CRC,		SHT1X_BOTH_PINS,	0,				NULL,	0,	{SHT1X_SIDX_SREGR_CRC_2, SHT1X_SIDX_SREGR_CRC_2}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_CRC_2 */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,				NULL,	0,	{SHT1X_SIDX_SREGR_CRC_2C, SHT1X_SIDX_SREGR_CRC_2C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_CRC_2C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_CRC,		SHT1X_BOTH_PINS,	0,				NULL,	0,	{SHT1X_SIDX_SREGR_CRC_1, SHT1X_SIDX_SREGR_CRC_1}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_CRC_1 */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,				NULL,	0,	{SHT1X_SIDX_SREGR_CRC_1C, SHT1X_SIDX_SREGR_CRC_1C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_CRC_1C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_CRC,		SHT1X_BOTH_PINS,	0,				NULL,	0,	{SHT1X_SIDX_SREGR_CRC_0, SHT1X_SIDX_SREGR_CRC_0}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_CRC_0 */	{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,				NULL,	0,	{SHT1X_SIDX_SREGR_CRC_0C, SHT1X_SIDX_SREGR_CRC_0C}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_CRC_0C */	{SHT1X_SOUT_GPIO | SHT1X_SOUT_CRC,		SHT1X_BOTH_PINS,	0,				NULL,	0,	{SHT1X_SIDX_SREGR_CRC_ACKS, SHT1X_SIDX_SREGR_CRC_ACKS}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},

	/* SHT1X_SIDX_SREGR_CRC_ACKS */{SHT1X_SOUT_GPIO,						SHT1X_DATA_OUT_PIN,	0,				NULL,	0,	{SHT1X_SIDX_SREGR_CRC_ACK, SHT1X_SIDX_SREGR_CRC_ACK}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_CRC_ACK */	{SHT1X_SOUT_GPIO,						0x00,				0,				NULL,	0,	{SHT1X_SIDX_SREGR_CRC_ACKC, SHT1X_SIDX_SREGR_CRC_ACKC}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGR_CRC_ACKC */{SHT1X_SOUT_GPIO,						SHT1X_CLOCK_PIN,	0,				NULL,	0,	{SHT1X_SIDX_END, SHT1X_SIDX_END}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},

	/* Status Register Write */
	/* SHT1X_SIDX_SREGW_ACK */			{SHT1X_SOUT_GPIO |
										 SHT1X_SOUT_TSTART,	SHT1X_DATA_OUT_PIN,	0,	NULL,				0,	{SHT1X_SIDX_SREGW_ACKC, SHT1X_SIDX_SREGW_ACKC}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGW_ACKC */			{SHT1X_SOUT_GPIO,	SHT1X_BOTH_PINS,	0,	NULL,				0,	{SHT1X_SIDX_SREGW_ACKE, SHT1X_SIDX_FAIL}, {SHT1X_ERROR_OK, SHT1X_ERROR_NO_CMD_ACK}},
	/* SHT1X_SIDX_SREGW_ACKE */			{SHT1X_SOUT_GPIO,	SHT1X_DATA_OUT_PIN,	0,	NULL,				0,	{SHT1X_SIDX_SREGW_PAYLOAD, SHT1X_SIDX_SREGW_PAYLOAD}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGW_PAYLOAD */		{SHT1X_SOUT_DMA,	0,					0,	device.cmd_payload, 16,	{SHT1X_SIDX_SREGW_PAYLOAD_ACK, SHT1X_SIDX_SREGW_PAYLOAD_ACK}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGW_PAYLOAD_ACK */	{SHT1X_SOUT_GPIO |
										 SHT1X_SOUT_TSTART,	SHT1X_DATA_OUT_PIN,	0,	NULL,				0,	{SHT1X_SIDX_SREGW_PAYLOAD_ACKC, SHT1X_SIDX_SREGW_PAYLOAD_ACKC}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
	/* SHT1X_SIDX_SREGW_PAYLOAD_ACKC */	{SHT1X_SOUT_GPIO,	SHT1X_BOTH_PINS,	0,	NULL,				0,	{SHT1X_SIDX_SREGW_PAYLOAD_ACKE, SHT1X_SIDX_FAIL}, {SHT1X_ERROR_OK, SHT1X_ERROR_NO_PAYLOAD_ACK}},
	/* SHT1X_SIDX_SREGW_PAYLOAD_ACKE */	{SHT1X_SOUT_GPIO,	SHT1X_DATA_OUT_PIN,	0,	NULL,				0,	{SHT1X_SIDX_END, SHT1X_SIDX_END}, {SHT1X_ERROR_OK, SHT1X_ERROR_OK}},
};


static xSemaphoreHandle lock = NULL;

static xSemaphoreHandle interrupt_semaphore = NULL;


inline void sht1x_debug_toggle()
{
	uint32_t dv = GPIOPinRead(SHT1X_PORT_BASE, DEBUG_PIN);
	if (dv) {
		GPIOPinWrite(SHT1X_PORT_BASE, DEBUG_PIN, 0x00);
	} else {
		GPIOPinWrite(SHT1X_PORT_BASE, DEBUG_PIN, DEBUG_PIN);
	}
}

inline void sht1x_timera_value_set(uint32_t value)
{
	HWREG(SHT1X_TIMER_BASE + TIMER_O_TAV) = value;
}


inline uint32_t sht1x_timera_value_get()
{
	return HWREG(SHT1X_TIMER_BASE + TIMER_O_TAV);
}


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

	GPIOPinTypeGPIOOutput(SHT1X_PORT_BASE, DEBUG_PIN);
	GPIOPinWrite(SHT1X_PORT_BASE, DEBUG_PIN, DEBUG_PIN);

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

	TimerConfigure(SHT1X_TIMER_BASE, TIMER_CFG_A_PERIODIC);
	TimerPrescaleSet(SHT1X_TIMER_BASE, SHT1X_TIMER, 0);
	TimerClockSourceSet(SHT1X_TIMER_BASE, TIMER_CLOCK_SYSTEM);
	TimerLoadSet(SHT1X_TIMER_BASE, SHT1X_TIMER, SHT1X_CLK_NR);

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

	device.data = 0;
	device.crc = 0;
	device.state = SHT1X_SIDX_CMD;
	device.error = SHT1X_ERROR_OK;

	sht1x_disable_interrupt();

	sht1x_udma_set_buffer((void *)measure_temperature_cmd, sizeof(measure_temperature_cmd));
	TimerLoadSet(SHT1X_TIMER_BASE, SHT1X_TIMER, SHT1X_CLK_NR);
	sht1x_timer_start();

	xSemaphoreTake(interrupt_semaphore, portMAX_DELAY);

	if (device.error == SHT1X_ERROR_OK) {
		xSemaphoreGive(lock);
		return 1;
	} else {
		xSemaphoreGive(lock);
		return 0;
	}
}

sht1x_error_t sht1x_status_write(uint8_t status)
{
	xSemaphoreTake(lock, portMAX_DELAY);

	memcpy(device.cmd, trans_start_pattern, sizeof(trans_start_pattern));
	memcpy(&device.cmd[sizeof(trans_start_pattern)], sreg_write_pattern, sizeof(sreg_write_pattern));

	uint8_t i;
	uint8_t mask;
	for (mask = 0x80; mask; mask >>= 1, i += 2) {
		if (status & mask) {
			device.cmd_payload[i] = SHT1X_DATA_OUT_PIN;
			device.cmd_payload[i + 1] = SHT1X_BOTH_PINS;
		} else {
			device.cmd_payload[i] = 0x00;
			device.cmd_payload[i + 1] = SHT1X_CLOCK_PIN;
		}
	}

	device.data = 0;
	device.crc = 0;
	device.state = SHT1X_SIDX_SREGW_ACK;
	device.error = SHT1X_ERROR_OK;

	sht1x_disable_interrupt();

	sht1x_udma_set_buffer(device.cmd, sizeof(trans_start_pattern) + sizeof(sreg_write_pattern));

	TimerLoadSet(SHT1X_TIMER_BASE, SHT1X_TIMER, SHT1X_CLK_NR);
	TimerIntClear(SHT1X_TIMER_BASE, TIMER_TIMA_TIMEOUT);
	TimerEnable(SHT1X_TIMER_BASE, SHT1X_TIMER);

	xSemaphoreTake(interrupt_semaphore, portMAX_DELAY);

	sht1x_error_t result = device.error;
	xSemaphoreGive(lock);

	return result;
}


sht1x_error_t sht1x_status_read(uint8_t * status)
{
	xSemaphoreTake(lock, portMAX_DELAY);

	memcpy(device.cmd, trans_start_pattern, sizeof(trans_start_pattern));
	memcpy(&device.cmd[sizeof(trans_start_pattern)], sreg_read_pattern, sizeof(sreg_read_pattern));

	device.data = 0;
	device.crc = 0;
	device.state = SHT1X_SIDX_SREGR_ACK;
	device.error = SHT1X_ERROR_OK;

	sht1x_disable_interrupt();

	sht1x_udma_set_buffer((void *) device.cmd, sizeof(trans_start_pattern) + sizeof(sreg_read_pattern));

	TimerLoadSet(SHT1X_TIMER_BASE, SHT1X_TIMER, SHT1X_CLK_NR);
	TimerIntClear(SHT1X_TIMER_BASE, TIMER_TIMA_TIMEOUT);
	TimerEnable(SHT1X_TIMER_BASE, SHT1X_TIMER);

	xSemaphoreTake(interrupt_semaphore, portMAX_DELAY);

	sht1x_error_t result = device.error;

	if (result == SHT1X_ERROR_OK && status != NULL) {
		*status = device.data & 0xff;
	}

	xSemaphoreGive(lock);
	return result;
}


void sht1x_process()
{
	const sht1x_state_t * state = &states[device.state];
	uint32_t cur_output = state->output;

	uint8_t input = (sht1x_data_in_get() & SHT1X_DATA_IN_PIN) >> SHT1X_DIN_PIN_bp;

	device.state = state->next_state[input];
	device.error = state->next_error[input];

	if (cur_output & SHT1X_SOUT_ISTOP) {
		sht1x_disable_interrupt();
	}

	if (cur_output & SHT1X_SOUT_DMA) {
		TimerIntDisable(SHT1X_TIMER_BASE, TIMER_TIMA_TIMEOUT);
		sht1x_udma_set_buffer(state->dma_buff, state->dma_items_count);
	}

	if (cur_output & SHT1X_SOUT_GPIO) {
		GPIOPinWrite(SHT1X_PORT_BASE, SHT1X_OUTPUT_PINS, state->gpio);
	}

	if (cur_output & SHT1X_SOUT_DATA) {
		device.data <<= 1;
		device.data |= input;
	}

	if (cur_output & SHT1X_SOUT_CRC) {
		device.crc <<= 1;
		device.crc |= input;
	}

	if (cur_output & SHT1X_SOUT_TSPEED) {
		sht1x_timer_stop();
		TimerLoadSet(SHT1X_TIMER_BASE, SHT1X_TIMER, state->speed);
		sht1x_timer_start();
	}

	if (cur_output & SHT1X_SOUT_ISTART) {
		sht1x_enable_interrupt();
	}

	if (cur_output & SHT1X_SOUT_TSTOP) {
		sht1x_timer_stop();
	}

	if (cur_output & SHT1X_SOUT_TSTART) {
		TimerIntEnable(SHT1X_TIMER_BASE, TIMER_TIMA_TIMEOUT);
	}

	if (cur_output & SHT1X_SOUT_DONE) {
		xSemaphoreGiveFromISR(interrupt_semaphore, NULL);
		return;
	}

	if (device.error != SHT1X_ERROR_OK) {
		sht1x_disable_interrupt();
		sht1x_timer_stop();
		xSemaphoreGiveFromISR(interrupt_semaphore, NULL);
	}
}


void timer0a_isr_handler(void)
{
	uint32_t udma_status = uDMAIntStatus();
	uint32_t timer_status = TimerIntStatus(SHT1X_TIMER_BASE, false);

	TimerIntClear(SHT1X_TIMER_BASE, timer_status);
	uDMAIntClear(udma_status);

	if (timer_status & TIMER_TIMA_TIMEOUT) {
		sht1x_process();
	}
}


void gpioe_isr_handler(void)
{
	uint32_t gpio_status = GPIOIntStatus(SHT1X_PORT_BASE, false);
	GPIOIntClear(SHT1X_PORT_BASE, gpio_status);

	if (gpio_status & SHT1X_DATA_INTERRUPT) {
		sht1x_process();
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
