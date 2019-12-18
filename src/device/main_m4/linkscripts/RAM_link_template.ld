MEMORY
{
    /* Define each memory region */
    SPIflash (rx) : ORIGIN = 0x14000000, LENGTH = 0x30000 /* 192K bytes */
    RamAHB32 (rwx) : ORIGIN = 0x20000000, LENGTH = 0x8000 /* 32K bytes */
    RamAHB16 (rwx) : ORIGIN = 0x20008000, LENGTH = 0x4000 /* 16K bytes */
    RamLoc128 (rwx) : ORIGIN = 0x10000000, LENGTH = 0x30000 /* 128K bytes */
    RamLoc72 (rwx) : ORIGIN = 0x10080000, LENGTH = 0x12000 /* 72K bytes */
    RamAHB_ETB16 (rwx) : ORIGIN = 0x2000c000, LENGTH = 0x4000 /* 16K bytes */
}
/* Define a symbol for the top of each memory region */
__top_SPIflash = 0x14000000 + 0x30000;
__top_RamAHB32 = 0x20000000 + 0x8000;
__top_RamAHB16 = 0x20008000 + 0x4000;
__top_RamLoc128 = 0x10000000 + 0x20000;
__top_RamLoc72 = 0x10080000 + 0x12000;
__top_RamAHB_ETB16 = 0x2000c000 + 0x4000;

ENTRY(ResetISR)

SECTIONS
{
    /* MAIN TEXT SECTION */    
    .text.start : ALIGN(4)
    {
        FILL(0xff)
        KEEP(*(.spi_vectors))

        /* Global Section Table */
        . = ALIGN(4) ;
        __section_table_start = .;
        __data_section_table = .;
        LONG(LOADADDR(.data));
        LONG(    ADDR(.data));
        LONG(  SIZEOF(.data));
        LONG(LOADADDR(.data.core_m0app));
        LONG(    ADDR(.data.core_m0app));
        LONG(  SIZEOF(.data.core_m0app));

        LONG(LOADADDR(.text));
        LONG(    ADDR(.text));
        LONG(  SIZEOF(.text));
        LONG(LOADADDR(.text.core_m0app));
        LONG(    ADDR(.text.core_m0app));
        LONG(  SIZEOF(.text.core_m0app));

        __data_section_table_end = .;
        __bss_section_table = .;
        LONG(    ADDR(.bss));
        LONG(  SIZEOF(.bss));
        LONG(    ADDR(.bss.core_m0app));
        LONG(  SIZEOF(.bss.core_m0app));
        __bss_section_table_end = .;
        __section_table_end = . ;
        /* End of Global Section Table */
        . = ALIGN(4);
        
        *(.reset_isr)
        . = ALIGN(4);
    } > SPIflash

    .text : ALIGN(4)    
    {
        FILL(0xff)

        /* Interupts */
        __vectors_start__ = ABSOLUTE(.) ;
        KEEP(*(.isr_vector))
        *(.after_vectors)
        . = ALIGN(4) ;

        /* Pure text section */
        *(.text*)

        . = ALIGN(4);
        __preinit_array_start = .;
        KEEP (*(.preinit_array))
        __preinit_array_end = .;

        . = ALIGN(4);
        __init_array_start = .;
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array))
        __init_array_end = .;

        KEEP(*(.fini));
        . = ALIGN(4);
        KEEP (*crtbegin.o(.ctors))
        KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
        KEEP (*(SORT(.ctors.*)))
        KEEP (*crtend.o(.ctors))

        /* C++ constructors etc */
        . = ALIGN(4);
        KEEP(*(.init))

        . = ALIGN(4);
        KEEP (*crtbegin.o(.dtors))
        KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
        KEEP (*(SORT(.dtors.*)))
        KEEP (*crtend.o(.dtors))
        /* End C++ */

        . = ALIGN(4);
        *(.rodata .rodata.* .constdata .constdata.*)
        . = ALIGN(4);
    } > RamLoc128 AT> SPIflash

    /*
     * for exception handling/unwind - some Newlib functions (in common
     * with C++ and STDC++) use this. 
     */
    .ARM.extab : ALIGN(4)
    {
    	*(.ARM.extab* .gnu.linkonce.armextab.*)
    } > RamLoc128 AT> SPIflash
    __exidx_start = .;
    
    .ARM.exidx : ALIGN(4)
    {
    	*(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > RamLoc128 AT> SPIflash
    __exidx_end = .;
    
    _etext = .;

    .text.core_m0app : ALIGN(4)
    {
        FILL(0xff)
        __core_m0app_START__ = .; /* start of slave image */
        KEEP(*(.core_m0app))
        __core_m0app_END__ = .; /* end of slave image */
       ASSERT(!(__core_m0app_START__ == __core_m0app_END__), "No slave code for _core_m0app");
       ASSERT( (ABSOLUTE(__core_m0app_START__) == __vectors_start___core_m0app), "M0APP execute address differs from address provided in source image");
       . = ALIGN(4) ;
    } > RamAHB16 AT> SPIflash

    .data.core_m0app : ALIGN(4)
    {
        FILL(0xff)
        KEEP(*(.core_m0app.data))
        . = ALIGN(4) ;
    } > RamAHB16 AT> SPIflash

    .bss.core_m0app : ALIGN(4)
    {
        FILL(0xff)
        KEEP(*(.core_m0app.bss))
        . = ALIGN(4) ;
    } > RamAHB16

    .got : ALIGN(4)    
    {
        *(.got)
        . = ALIGN(4);
    } > RamAHB32 AT> SPIflash

    .got.plt : ALIGN(4)    
    {
        *(.got)
        . = ALIGN(4);
    } > RamAHB32 AT> SPIflash

    /* MAIN DATA SECTION */
    .data : ALIGN(4)
    {
       FILL(0xff)
       _data = . ;
       *(vtable)
       *(.ramfunc*)
       *(.data*)
       . = ALIGN(4) ;
       _edata = . ;
    } > RamAHB32 AT>SPIflash

    /* MAIN BSS SECTION */
    .bss : ALIGN(4)
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4) ;
        _ebss = .;
    } > RamAHB32

    .uninit_RESERVED : ALIGN(4)
    {
        KEEP(*(.bss.$RESERVED*))
        . = ALIGN(4) ;
        _end_uninit_RESERVED = .;
    } > RamAHB32 AT> SPIflash
    
    /* DEFAULT NOINIT SECTION */
    .noinit (NOLOAD): ALIGN(4)
    {
        _noinit = .;
        *(.noinit*) 
         . = ALIGN(4) ;
        _end_noinit = .;
        PROVIDE(end = .);
    } > RamAHB32

    PROVIDE(_pvHeapStart = .);
    PROVIDE(_vStackTop = __top_RamAHB32);
    PROVIDE(_pvHeapLimit = __top_RamAHB32 - 0x600);   /* stack is 0x600 bytes or 1.5k  */
    
    .heap :
    {
        ASSERT (((_pvHeapStart + 0x2000) < _pvHeapLimit), "Heap size is less than 27KB");
    }
}
