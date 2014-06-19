
MEMORY
{
	FLASH (RX) : ORIGIN = 0x00000000, LENGTH = 0x00020000
	SRAM (WX)  : ORIGIN = 0x20000000, LENGTH = 0x00008000
}


SECTIONS {
	.isr_vector ( ORIGIN(FLASH) ) : AT ( ORIGIN(FLASH) ) {
		KEEP (*(.isr_vector))
	} > FLASH


	.isr_handlers : {
		. = ALIGN(0x4);
		KEEP (*(.isr_handler))
		*(.isr_handler.*)
	} > FLASH


	.text : {
		CREATE_OBJECT_SYMBOLS

		/* init and fini */
		. = ALIGN(0x04);
		KEEP(*(.init))
		KEEP(*(.fini*))

		/* .ctors */
		. = ALIGN(0x04);
		*crtbegin.o(.ctors)
		*crtbegin?.o(.ctors)
		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
		*(SORT(.ctors.*))
		*(.ctors)

		/* .dtors */
		. = ALIGN(0x04);
		*crtbegin.o(.dtors)
		*crtbegin?.o(.dtors)
		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
		*(SORT(.dtors.*))
		*(.dtors)

		/* code goes here */
		. = ALIGN(0x04);
		KEEP (*(.text))
		*(.text.*)

		/* preinit data */
		. = ALIGN(0x04);
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP(*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);
		
		/* init data */
		. = ALIGN(4);
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array))
		PROVIDE_HIDDEN (__init_array_end = .);

		/* finit data */
		. = ALIGN(4);
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP(*(SORT(.fini_array.*)))
		KEEP(*(.fini_array))
		PROVIDE_HIDDEN (__fini_array_end = .);
	} > FLASH


	.rodata : {
		*(.rodata)
		*(.rodata*)
	} > FLASH


	.eh_frames : {
		KEEP(*(.eh_frame*))
	} > FLASH


	.ARM.extab : {
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} > FLASH


	.ARM.extab : {
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} > REGION_ARM_EXTAB


	__exidx_start = .;
	.ARM.exidx : {
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > FLASH
	__exidx_end = .;


	__etext = .;
	_etext = .;
	etext = .;

	.data : AT (__etext) {
		__data_load__ = LOADADDR(.data);
		__data_start__ = .;
		KEEP (*(.vtable))
		*(vtable)
		KEEP (*(.data))
		KEEP (*(.data*))
		KEEP (*(.jcr*))

		/* all data end */
		. = ALIGN(4);
		__data_end__ = .;
	} > SRAM AT> FLASH


	.bss : {
		. = ALIGN(4);
		__bss_start__ = .;
		*(.shbss)
		KEEP (*(.bss))
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end__ = .;
	} > SRAM


	.heap (COPY): {
		__heap_start__ = .;
		end = .;
		_end = .;
		__end = .;
		__end__ = .;
		PROVIDE (end = .);
		KEEP (*(.heap*))
		__HeapLimit = .;
	} > SRAM


    .stack_dummy (COPY): {
		*(.stack*)
	} > SRAM


	/* stack points to the highest address in SRAM */
	PROVIDE(__StackTop  = ALIGN(ORIGIN(SRAM) + LENGTH(SRAM), 0x04));
	.stack (__StackTop)  : AT (__StackTop) {
		_stack  = .;
		__stack = .;
	} > SRAM


	PROVIDE (__StackLimit = __StackTop - SIZEOF(.stack_dummy))


	/* Check if data + heap + stack exceeds RAM limit */
	ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")
}

