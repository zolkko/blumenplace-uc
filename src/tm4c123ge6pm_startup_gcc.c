//*****************************************************************************
//
// Startup code for use with TI's Code Composer Studio and GNU tools.
//
// Copyright (c) 2011-2013 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Software License Agreement
//
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
//
//*****************************************************************************

#include <stdint.h>
#include "sys.h"

#ifndef HWREG
#define HWREG(x) (*((volatile uint32_t *)(x)))
#endif


extern uint32_t __etext;
extern uint32_t __data_start__;
extern uint32_t __data_end__;
extern uint32_t __bss_start__;
extern uint32_t __bss_end__;

/* address defined in LD script */
extern uint32_t __StackTop;

/* Forward declaration of the default fault handlers. */
static void reset_isr_handler(void);
static void default_isr_handler(void);
static void nmi_isr_handler(void);
static void fault_isr_handler(void);

void __attribute__((weak)) systick_isr_handler(void);
void __attribute__((weak, naked)) svc_isr_handler(void);
void __attribute__((weak, naked)) pendsv_isr_handler(void);

extern void _mainCRTStartup(void);


/* The vector table. */
__attribute__ ((section(".isr_vector")))
void (* const g_pfnVectors[])(void) = {
	(void (*)(void))(&__StackTop),	// The initial stack pointer
	reset_isr_handler,				// The reset handler
	nmi_isr_handler,				// The NMI handler
	fault_isr_handler,				// The hard fault handler
	default_isr_handler,			// The MPU fault handler
	default_isr_handler,			// The bus fault handler
	default_isr_handler,			// The usage fault handler
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	svc_isr_handler,				// SVCall handler
	default_isr_handler,			// Debug monitor handler
	0,								// Reserved
	pendsv_isr_handler,				// The PendSV handler
	systick_isr_handler,			// The SysTick handler
	default_isr_handler,			// GPIO Port A
	default_isr_handler,			// GPIO Port B
	default_isr_handler,			// GPIO Port C
	default_isr_handler,			// GPIO Port D
	default_isr_handler,			// GPIO Port E
	default_isr_handler,			// UART0 Rx and Tx
	default_isr_handler,			// UART1 Rx and Tx
	default_isr_handler,			// SSI0 Rx and Tx
	default_isr_handler,			// I2C0 Master and Slave
	default_isr_handler,			// PWM Fault
	default_isr_handler,			// PWM Generator 0
	default_isr_handler,			// PWM Generator 1
	default_isr_handler,			// PWM Generator 2
	default_isr_handler,			// Quadrature Encoder 0
	default_isr_handler,			// ADC Sequence 0
	default_isr_handler,			// ADC Sequence 1
	default_isr_handler,			// ADC Sequence 2
	default_isr_handler,			// ADC Sequence 3
	default_isr_handler,			// Watchdog timer
	default_isr_handler,			// Timer 0 subtimer A
	default_isr_handler,			// Timer 0 subtimer B
	default_isr_handler,			// Timer 1 subtimer A
	default_isr_handler,			// Timer 1 subtimer B
	default_isr_handler,			// Timer 2 subtimer A
	default_isr_handler,			// Timer 2 subtimer B
	default_isr_handler,			// Analog Comparator 0
	default_isr_handler,			// Analog Comparator 1
	default_isr_handler,			// Analog Comparator 2
	default_isr_handler,			// System Control (PLL, OSC, BO)
	default_isr_handler,			// FLASH Control
	default_isr_handler,			// GPIO Port F
	default_isr_handler,			// GPIO Port G
	default_isr_handler,			// GPIO Port H
	default_isr_handler,			// UART2 Rx and Tx
	default_isr_handler,			// SSI1 Rx and Tx
	default_isr_handler,			// Timer 3 subtimer A
	default_isr_handler,			// Timer 3 subtimer B
	default_isr_handler,			// I2C1 Master and Slave
	default_isr_handler,			// Quadrature Encoder 1
	default_isr_handler,			// CAN0
	default_isr_handler,			// CAN1
	0,								// Reserved
	0,								// Reserved
	default_isr_handler,			// Hibernate
	default_isr_handler,			// USB0
	default_isr_handler,			// PWM Generator 3
	default_isr_handler,			// uDMA Software Transfer
	default_isr_handler,			// uDMA Error
	default_isr_handler,			// ADC1 Sequence 0
	default_isr_handler,			// ADC1 Sequence 1
	default_isr_handler,			// ADC1 Sequence 2
	default_isr_handler,			// ADC1 Sequence 3
	0,								// Reserved
	0,								// Reserved
	default_isr_handler,			// GPIO Port J
	default_isr_handler,			// GPIO Port K
	default_isr_handler,			// GPIO Port L
	default_isr_handler,			// SSI2 Rx and Tx
	default_isr_handler,			// SSI3 Rx and Tx
	default_isr_handler,			// UART3 Rx and Tx
	default_isr_handler,			// UART4 Rx and Tx
	default_isr_handler,			// UART5 Rx and Tx
	default_isr_handler,			// UART6 Rx and Tx
	default_isr_handler,			// UART7 Rx and Tx
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	default_isr_handler,			// I2C2 Master and Slave
	default_isr_handler,			// I2C3 Master and Slave
	default_isr_handler,			// Timer 4 subtimer A
	default_isr_handler,			// Timer 4 subtimer B
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	0,								// Reserved
	default_isr_handler,			// Timer 5 subtimer A
	default_isr_handler,			// Timer 5 subtimer B
	default_isr_handler,			// Wide Timer 0 subtimer A
	default_isr_handler,			// Wide Timer 0 subtimer B
	default_isr_handler,			// Wide Timer 1 subtimer A
	default_isr_handler,			// Wide Timer 1 subtimer B
	default_isr_handler,			// Wide Timer 2 subtimer A
	default_isr_handler,			// Wide Timer 2 subtimer B
	default_isr_handler,			// Wide Timer 3 subtimer A
	default_isr_handler,			// Wide Timer 3 subtimer B
	default_isr_handler,			// Wide Timer 4 subtimer A
	default_isr_handler,			// Wide Timer 4 subtimer B
	default_isr_handler,			// Wide Timer 5 subtimer A
	default_isr_handler,			// Wide Timer 5 subtimer B
	default_isr_handler,			// FPU
	0,								// Reserved
	0,								// Reserved
	default_isr_handler,			// I2C4 Master and Slave
	default_isr_handler,			// I2C5 Master and Slave
	default_isr_handler,			// GPIO Port M
	default_isr_handler,			// GPIO Port N
	default_isr_handler,			// Quadrature Encoder 2
	0,								// Reserved
	0,								// Reserved
	default_isr_handler,			// GPIO Port P (Summary or P0)
	default_isr_handler,			// GPIO Port P1
	default_isr_handler,			// GPIO Port P2
	default_isr_handler,			// GPIO Port P3
	default_isr_handler,			// GPIO Port P4
	default_isr_handler,			// GPIO Port P5
	default_isr_handler,			// GPIO Port P6
	default_isr_handler,			// GPIO Port P7
	default_isr_handler,			// GPIO Port Q (Summary or Q0)
	default_isr_handler,			// GPIO Port Q1
	default_isr_handler,			// GPIO Port Q2
	default_isr_handler,			// GPIO Port Q3
	default_isr_handler,			// GPIO Port Q4
	default_isr_handler,			// GPIO Port Q5
	default_isr_handler,			// GPIO Port Q6
	default_isr_handler,			// GPIO Port Q7
	default_isr_handler,			// GPIO Port R
	default_isr_handler,			// GPIO Port S
	default_isr_handler,			// PWM 1 Generator 0
	default_isr_handler,			// PWM 1 Generator 1
	default_isr_handler,			// PWM 1 Generator 2
	default_isr_handler,			// PWM 1 Generator 3
	default_isr_handler				// PWM 1 Fault
};


__attribute__ ((section(".isr_handler")))
void reset_isr_handler(void)
{
	/* Copy the data segment initializers from flash to SRAM. */
	uint32_t * data_load_addr = &__etext;
	uint32_t * data_sram_addr = &__data_start__;
	for ( ; data_sram_addr < &__data_end__; ) {
		*data_sram_addr++ = *data_load_addr++;
	}

	/* Zero fill the bss segment. */
	__asm("    ldr     r0, =__bss_start__\n"
		  "    ldr     r1, =__bss_end__\n"
		  "    mov     r2, #0\n"
		  "    .thumb_func\n"
		  "zero_loop:\n"
		  "    cmp     r0, r1\n"
		  "    it      lt\n"
		  "    strlt   r2, [r0], #4\n"
		  "    blt     zero_loop");

	/* Enable the floating-point unit. */
	HWREG(0xE000ED88) = ((HWREG(0xE000ED88) & ~0x00F00000) | 0x00F00000);

	sys_init();

	_mainCRTStartup();
}


__attribute__ ((section(".isr_handler")))
static void nmi_isr_handler(void)
{
	while (1) {
	}
}


__attribute__ ((section(".isr_handler")))
static void fault_isr_handler(void)
{
	while (1) {
	}
}


__attribute__ ((section(".isr_handler")))
static void
default_isr_handler(void)
{
	while (1) {
	}
}


#pragma weak systick_isr_handler = default_isr_handler
#pragma weak svc_isr_handler = default_isr_handler
#pragma weak pendsv_isr_handler = default_isr_handler
