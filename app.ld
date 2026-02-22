OUTPUT_ARCH("i386:x86-64")
ENTRY(_start)
SECTIONS
{
	. = 0xFFFF800000000000;
	.text : {
		*(.text)
	}
	.data : {
		*(.data .rodata) QUAD(ADDR(.bss)+SIZEOF(.bss))
	}
	.init_array : {
		__preinit_array_start = .;
		*(.preinit_array)
		__preinit_array_end = .;
		__init_array_start = .;
		*(.init_array .ctors)
		__init_array_end = .;
	}
	.fini_array : {
		__fini_array_start = .;
		*(.fini_array .dtors)
		__fini_array_end = .;
	}
	.bss : {
		__bss_start = .;
		*(.bss)
		__bss_stop = .;
	}
}
