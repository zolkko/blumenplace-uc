;
; TM4C123 FreeRTOS v8.0.0 port
;
	.cdecls  C, NOLIST, WARN, "FreeRTOSConfig.h"

	.thumb
	.text

	.global vTaskSwitchContext
	.global pxCurrentTCB
pxCurrentTCBvalue .word pxCurrentTCB

	.global xPortPendSVHandler
	.global vPortSetInterruptMask
	.global ulPortSetInterruptMask
	.global vPortClearInterruptMask
	.global vPortSVCHandler
	.global vPortStartFirstTask
	.global vPortEnableVFP


;-----------------------------------------------------------

xPortPendSVHandler:
	mrs r0, psp

	; Get the location of the current TCB.
	ldr r3, pxCurrentTCBvalue
	ldr r2, [r3]

	; Is the task using the FPU context?  If so, push high vfp registers.
	tst r14, #0x10
	it EQ
	vstmdbeq r0!, {s16-s31}

	; Save the core registers.
	stmdb r0!, {r4-r11, r14}

	; Save the new top of stack into the first member of the TCB.
	str r0, [r2]

	stmdb sp!, {r3, r14}
	mov r0, #configMAX_SYSCALL_INTERRUPT_PRIORITY
	msr basepri, r0
	bl vTaskSwitchContext
	mov r0, #0
	msr basepri, r0
	ldmia sp!, {r3, r14}

	; The first item in pxCurrentTCB is the task top of stack.
	ldr r1, [r3]
	ldr r0, [r1]

	; Pop the core registers.
	ldmia r0!, {r4-r11, r14}

	; Is the task using the FPU context?  If so, pop the high vfp registers 
	; too.
	tst r14, #0x10
	it EQ
	vldmiaeq r0!, {s16-s31}

	msr psp, r0
	bx r14

;-----------------------------------------------------------

ulPortSetInterruptMask:
	mrs r0, BASEPRI
	mov r1, #configMAX_SYSCALL_INTERRUPT_PRIORITY
	msr BASEPRI, r1
	bx  lr


;-----------------------------------------------------------

vPortClearInterruptMask:
	mov r0, #0
	msr BASEPRI, r0
	bx  lr

;-----------------------------------------------------------

vPortSVCHandler:
	; Get the location of the current TCB.
	ldr r3, pxCurrentTCBvalue
	ldr r1, [r3]
	ldr r0, [r1]
	; Pop the core registers.
	ldmia r0!, {r4-r11, r14}
	msr psp, r0
	mov r0, #0
	msr basepri, r0
	bx r14

;-----------------------------------------------------------

vPortStartFirstTask:
; Vector Table offset.
VTABLE .set 0xE000ED08

	; Use the NVIC offset register to locate the stack.
	MOVW r0, #VTABLE & 0xFFFF
	MOVT r0, #VTABLE >> 16
	ldr r0, [r0]
	ldr r0, [r0]
	; Set the msp back to the start of the stack.
	msr msp, r0
	; Call SVC to start the first task.
	cpsie i
	svc #0

;-----------------------------------------------------------

vPortEnableVFP:
; CAPCR Register address.
CPACR .set 0xE000ED88

	; The FPU enable bits are in the CPACR.
	MOVW     r0, #CPACR & 0xFFFF
	MOVT     r0, #CPACR >> 16
	ldr r1, [r0]

	; Enable CP10 and CP11 coprocessors, then save back.
	orr r1, r1, #( 0xf << 20 )
	str r1, [r0]
	bx  r14

	.end
