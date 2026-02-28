ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 4M
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 4M
    RAM2  (rwx) : ORIGIN = 0x21000000, LENGTH = 2M
}

/* Keep stack at the RAM top to avoid overlapping the large localbase test .bss region. */
_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text*)
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame)
        KEEP(*(.init))
        KEEP(*(.fini))
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    .ARM.extab :
    {
        . = ALIGN(4);
        *(.ARM.extab* .gnu.linkonce.armextab.*)
        . = ALIGN(4);
    } > FLASH

    .ARM.exidx :
    {
        . = ALIGN(4);
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
        . = ALIGN(4);
    } > FLASH

    _sidata = LOADADDR(.data);

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH

    .bss (NOLOAD) :
    {
        . = ALIGN(8);
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(8);
        _ebss = .;
    } > RAM

    .freertosHeap (NOLOAD) :
    {
        . = ALIGN(8);
        __freertosHeapStart = .;
        KEEP(*(.freertosHeap))
        KEEP(*(.freertosHeap*))
        . = ALIGN(8);
        __freertosHeapEnd = .;
    } > RAM2

    .heap (NOLOAD) :
    {
        . = ALIGN(8);
        _end = .;
        end = .;
        __end__ = .;
        __HeapBase = .;
        __heap_start__ = .;
        . = ORIGIN(RAM) + LENGTH(RAM);
        __HeapLimit = .;
        __heap_end__ = .;
    } > RAM

    ASSERT(__freertosHeapEnd <= (ORIGIN(RAM2) + LENGTH(RAM2)), "freertosHeap overflows RAM2")

    /DISCARD/ :
    {
        *(.note*)
    }
}
