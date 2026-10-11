/* GCC Linker Script for GD32F470ZK */
/* Flash: 0x08000000, 3MB (0x300000) */
/* RAM:   0x20000000, 256KB (0x40000) */

ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 3072K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 256K
}

_estack = ORIGIN(RAM) + LENGTH(RAM);
_Min_Heap_Size = 0x200;
_Min_Stack_Size = 0x400;

SECTIONS
{
    .isr_vector : {
        KEEP(*(.isr_vector))
        *(.isr_vector.*)
    } > FLASH

    .text : {
        *(.text*)
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)
        KEEP(*(.init))
        KEEP(*(.fini))
        _etext = .;
    } > FLASH

    .ARM.exidx : {
        *(.ARM.exidx*)
    } > FLASH

    .init_array : {
        *(.init_array.*)
        *(.init_array)
    } > FLASH

    .fini_array : {
        *(.fini_array.*)
        *(.fini_array)
    } > FLASH

    _sidata = LOADADDR(.data);

    .data : {
        _sdata = .;
        *(.data*)
        _edata = .;
    } > RAM AT > FLASH

    .bss : {
        _sbss = .;
        __bss_start__ = _sbss;
        *(.bss*)
        *(COMMON)
        __bss_end__ = .;
        _ebss = .;
    } > RAM

    ._user_heap_stack : {
        . = ALIGN(8);
        PROVIDE(end = .);
        PROVIDE(_end = .);
        . = . + _Min_Heap_Size;
        . = . + _Min_Stack_Size;
        . = ALIGN(8);
    } > RAM

    /DISCARD/ : {
        *(.ARM.exidx*)
        *(.ARM.extab*)
        *(.gnu.linkonce.*)
    }
}
