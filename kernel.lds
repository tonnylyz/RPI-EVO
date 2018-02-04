SECTIONS
{
    . = 0x00080000;
    _init : {
        boot/start.o
        boot/vm.o
    }
    _init_end = ABSOLUTE(.);
    .text 0xFFFFFF8000000000 + _init_end : AT(_init_end) {
        init/main.o
        *(.text*)
    }
    
    .rodata : {
        *(.rodata*)
    }
    .data : {
        *(.data*)
    }
    .bss : {
        *(.bss*)
    }
}
