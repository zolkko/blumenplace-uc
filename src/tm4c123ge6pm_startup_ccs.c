#include <stdint.h>

/**
 * System interrupt handlers
 */
static void reset_isr_handler(void);
static void nm_isr_handler(void);
static void fault_isr_handler(void);
static void default_isr_handler(void);

/**
 * Import from FreeRTOS port
 */
extern void xPortPendSVHandler(void);
extern void vPortSVCHandler(void);
extern void xPortSysTickHandler(void);


#define DEFINE_ISR(X)			__attribute__((weak)) void X(void) { default_isr_handler(); }


/**
 * Externally defined interrupts
 */
DEFINE_ISR(timer0a_isr_handler)
DEFINE_ISR(udma_software_isr_handler)
DEFINE_ISR(udma_error_isr_handler)
DEFINE_ISR(gpioe_isr_handler)
DEFINE_ISR(ssi0_isr_handler)


/**
 * External declaration for the reset handler that is to be called when the
 * processor is started
 */
extern void _c_int00(void);

/**
 * Linker variable that marks the top of the stack.
 */
extern uint32_t __STACK_TOP;

/**
 *
 * The vector table.  Note that the proper constructs must be placed on this to
 * ensure that it ends up at physical address 0x0000.0000 or at the start of
 * the program if located at a start address other than 0.
 */
#pragma DATA_SECTION(g_pfnVectors, ".intvecs")
void (* const g_pfnVectors[])(void) =
{
    (void (*)(void))((uint32_t)&__STACK_TOP),
                                            // The initial stack pointer
    reset_isr_handler,                      // The reset handler
    nm_isr_handler,                         // The NMI handler
    fault_isr_handler,                      // The hard fault handler
    default_isr_handler,                    // The MPU fault handler
    default_isr_handler,                    // The bus fault handler
    default_isr_handler,                    // The usage fault handler
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    vPortSVCHandler,                        // SVCall handler
    default_isr_handler,                    // Debug monitor handler
    0,                                      // Reserved
    xPortPendSVHandler,                     // The PendSV handler
    xPortSysTickHandler,                    // The SysTick handler
    default_isr_handler,                    // GPIO Port A
    default_isr_handler,                    // GPIO Port B
    default_isr_handler,                    // GPIO Port C
    default_isr_handler,                    // GPIO Port D
    gpioe_isr_handler,                      // GPIO Port E
    default_isr_handler,                    // UART0 Rx and Tx
    default_isr_handler,                    // UART1 Rx and Tx
    ssi0_isr_handler,	                    // SSI0 Rx and Tx
    default_isr_handler,                    // I2C0 Master and Slave
    default_isr_handler,                    // PWM Fault
    default_isr_handler,                    // PWM Generator 0
    default_isr_handler,                    // PWM Generator 1
    default_isr_handler,                    // PWM Generator 2
    default_isr_handler,                    // Quadrature Encoder 0
    default_isr_handler,                    // ADC Sequence 0
    default_isr_handler,                    // ADC Sequence 1
    default_isr_handler,                    // ADC Sequence 2
    default_isr_handler,                    // ADC Sequence 3
    default_isr_handler,                    // Watchdog timer
    timer0a_isr_handler,                    // Timer 0 subtimer A
    default_isr_handler,                    // Timer 0 subtimer B
    default_isr_handler,                    // Timer 1 subtimer A
    default_isr_handler,                    // Timer 1 subtimer B
    default_isr_handler,                    // Timer 2 subtimer A
    default_isr_handler,                    // Timer 2 subtimer B
    default_isr_handler,                    // Analog Comparator 0
    default_isr_handler,                    // Analog Comparator 1
    default_isr_handler,                    // Analog Comparator 2
    default_isr_handler,                    // System Control (PLL, OSC, BO)
    default_isr_handler,                    // FLASH Control
    default_isr_handler,                    // GPIO Port F
    default_isr_handler,                    // GPIO Port G
    default_isr_handler,                    // GPIO Port H
    default_isr_handler,                    // UART2 Rx and Tx
    default_isr_handler,                    // SSI1 Rx and Tx
    default_isr_handler,                    // Timer 3 subtimer A
    default_isr_handler,                    // Timer 3 subtimer B
    default_isr_handler,                    // I2C1 Master and Slave
    default_isr_handler,                    // Quadrature Encoder 1
    default_isr_handler,                    // CAN0
    default_isr_handler,                    // CAN1
    default_isr_handler,                    // CAN2
    0,                                      // Reserved
    default_isr_handler,                    // Hibernate
    default_isr_handler,                    // USB0
    default_isr_handler,                    // PWM Generator 3
    udma_software_isr_handler,              // uDMA Software Transfer
    udma_error_isr_handler,                 // uDMA Error
    default_isr_handler,                    // ADC1 Sequence 0
    default_isr_handler,                    // ADC1 Sequence 1
    default_isr_handler,                    // ADC1 Sequence 2
    default_isr_handler,                    // ADC1 Sequence 3
    0,                                      // Reserved
    0,                                      // Reserved
    default_isr_handler,                    // GPIO Port J
    default_isr_handler,                    // GPIO Port K
    default_isr_handler,                    // GPIO Port L
    default_isr_handler,                    // SSI2 Rx and Tx
    default_isr_handler,                    // SSI3 Rx and Tx
    default_isr_handler,                    // UART3 Rx and Tx
    default_isr_handler,                    // UART4 Rx and Tx
    default_isr_handler,                    // UART5 Rx and Tx
    default_isr_handler,                    // UART6 Rx and Tx
    default_isr_handler,                    // UART7 Rx and Tx
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    default_isr_handler,                    // I2C2 Master and Slave
    default_isr_handler,                    // I2C3 Master and Slave
    default_isr_handler,                    // Timer 4 subtimer A
    default_isr_handler,                    // Timer 4 subtimer B
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    default_isr_handler,                    // Timer 5 subtimer A
    default_isr_handler,                    // Timer 5 subtimer B
    default_isr_handler,                    // Wide Timer 0 subtimer A
    default_isr_handler,                    // Wide Timer 0 subtimer B
    default_isr_handler,                    // Wide Timer 1 subtimer A
    default_isr_handler,                    // Wide Timer 1 subtimer B
    default_isr_handler,                    // Wide Timer 2 subtimer A
    default_isr_handler,                    // Wide Timer 2 subtimer B
    default_isr_handler,                    // Wide Timer 3 subtimer A
    default_isr_handler,                    // Wide Timer 3 subtimer B
    default_isr_handler,                    // Wide Timer 4 subtimer A
    default_isr_handler,                    // Wide Timer 4 subtimer B
    default_isr_handler,                    // Wide Timer 5 subtimer A
    default_isr_handler,                    // Wide Timer 5 subtimer B
    default_isr_handler,                    // FPU
    0,                                      // Reserved
    0,                                      // Reserved
    default_isr_handler,                    // I2C4 Master and Slave
    default_isr_handler,                    // I2C5 Master and Slave
    default_isr_handler,                    // GPIO Port M
    default_isr_handler,                    // GPIO Port N
    default_isr_handler,                    // Quadrature Encoder 2
    0,                                      // Reserved
    0,                                      // Reserved
    default_isr_handler,                    // GPIO Port P (Summary or P0)
    default_isr_handler,                    // GPIO Port P1
    default_isr_handler,                    // GPIO Port P2
    default_isr_handler,                    // GPIO Port P3
    default_isr_handler,                    // GPIO Port P4
    default_isr_handler,                    // GPIO Port P5
    default_isr_handler,                    // GPIO Port P6
    default_isr_handler,                    // GPIO Port P7
    default_isr_handler,                    // GPIO Port Q (Summary or Q0)
    default_isr_handler,                    // GPIO Port Q1
    default_isr_handler,                    // GPIO Port Q2
    default_isr_handler,                    // GPIO Port Q3
    default_isr_handler,                    // GPIO Port Q4
    default_isr_handler,                    // GPIO Port Q5
    default_isr_handler,                    // GPIO Port Q6
    default_isr_handler,                    // GPIO Port Q7
    default_isr_handler,                    // GPIO Port R
    default_isr_handler,                    // GPIO Port S
    default_isr_handler,                    // PWM 1 Generator 0
    default_isr_handler,                    // PWM 1 Generator 1
    default_isr_handler,                    // PWM 1 Generator 2
    default_isr_handler,                    // PWM 1 Generator 3
    default_isr_handler                     // PWM 1 Fault
};


/**
 * Jump to the CCS C initialization routine.  This will enable the
 * floating-point unit as well, so that does not need to be done here.
 */
void reset_isr_handler(void)
{
	__asm(" .global _c_int00\n"
		  "  b.w     _c_int00");
}


void nm_isr_handler(void)
{
    while (1) {
    }
}


void fault_isr_handler(void)
{
    while (1) {
    }
}


void default_isr_handler(void)
{
    while (1) {
    }
}
