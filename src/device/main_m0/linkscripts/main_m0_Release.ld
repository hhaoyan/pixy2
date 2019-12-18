/*
 * Edited - DO NOT REGENERATE!
 * GENERATED FILE - DO NOT EDIT
 * (c) Code Red Technologies Ltd, 2008-13
 * (c) NXP Semiconductors 2013-2015
 * Generated linker script file for LPC4330-M0
 * Created from generic_c.ld (LPCXpresso v7.6 (2 [Build 326] [2015-02-02] ))
 * By LPCXpresso v7.6.2 [Build 326] [2015-02-02]  on Wed Apr 01 17:57:00 CDT 2015
 */

MEMORY
{
    /* Define each memory region */
    RamAHB32 (rwx) : ORIGIN = 0x20000000, LENGTH = 0x8000 /* 32K bytes */
    RamAHB16 (rwx) : ORIGIN = 0x20008000, LENGTH = 0x4000 /* 16K bytes */
    RamLoc128 (rwx) : ORIGIN = 0x10000000, LENGTH = 0x20000 /* 128K bytes */
    RamLoc72 (rwx) : ORIGIN = 0x10080000, LENGTH = 0x12000 /* 72K bytes */
    RamAHB_ETB16 (rwx) : ORIGIN = 0x2000c000, LENGTH = 0x4000 /* 16K bytes */
}
/* Define a symbol for the top of each memory region */
__top_RamAHB32 = 0x20000000 + 0x8000;
__top_RamAHB16 = 0x20008000 + 0x4000;
__top_RamLoc128 = 0x10000000 + 0x20000;
__top_RamLoc72 = 0x10080000 + 0x12000;
__top_RamAHB_ETB16 = 0x2000c000 + 0x4000;

ENTRY(ResetISR)

SECTIONS
{

    /* MAIN TEXT SECTION */    
    .text : ALIGN(4)
    {
        FILL(0xff)
        __vectors_start__ = ABSOLUTE(.) ;
        KEEP(*(.isr_vector))
        
        /* Global Section Table */
        . = ALIGN(4) ;
        __section_table_start = .;
        __data_section_table = .;
        LONG(LOADADDR(.data));
        LONG(    ADDR(.data));
        LONG(  SIZEOF(.data));
        __data_section_table_end = .;
        __bss_section_table = .;
        LONG(    ADDR(.bss));
        LONG(  SIZEOF(.bss));
        __bss_section_table_end = .;
        __section_table_end = . ;
        /* End of Global Section Table */


        *(.after_vectors*)

    } >RamAHB16
    
    .text : ALIGN(4)    
    {
         *(.text*)
        *(.rodata .rodata.* .constdata .constdata.*)
        *(.init .fini)
        . = ALIGN(4);

    } > RamAHB16

    /*
     * for exception handling/unwind - some Newlib functions (in common
     * with C++ and STDC++) use this. 
     */
    .ARM.extab : ALIGN(4)
    {
    	*(.ARM.extab* .gnu.linkonce.armextab.*)
    } > RamAHB16
    __exidx_start = .;
    
    .ARM.exidx : ALIGN(4)
    {
    	*(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > RamAHB16
    __exidx_end = .;
    
    _etext = .;
    
    /* MAIN DATA SECTION */

	/* Main DATA section */
	.data : ALIGN(4)
	{
	   FILL(0xff)
	   _data = . ;
	   *(vtable)
	   *(.ramfunc*)
	   *(.data*)
	   . = ALIGN(4) ;
	   _edata = . ;
	} > RamAHB16

    /* MAIN BSS SECTION */
    .bss : ALIGN(4)
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4) ;
        _ebss = .;
        PROVIDE(end = .);
    } > RamAHB16
    
    /* DEFAULT NOINIT SECTION */
    .noinit (NOLOAD): ALIGN(4)
    {
        _noinit = .;
        *(.noinit*) 
         . = ALIGN(4) ;
        _end_noinit = .;
    } > RamAHB16
    
    _m0_mem_end = .;

    PROVIDE(_pvHeapStart = DEFINED(__user_heap_base) ? __user_heap_base : _m0_mem_end);
    PROVIDE(_vStackTop = DEFINED(__user_stack_top) ? __user_stack_top : __top_RamAHB16);
    PROVIDE(_pvHeapLimit = _vStackTop - 0x400);

    .heap :
    {
        ASSERT (((_pvHeapStart + 0x400) < _pvHeapLimit), "Heap size is less than 1KB"); 
    }
}
