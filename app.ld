OUTPUT_ARCH("i386:x86-64")
ENTRY(_start)
PHDRS
{
	text PT_LOAD FLAGS(5); /* R + X */
	data PT_LOAD FLAGS(6); /* R + W */
}

SECTIONS
{
	. = 0xFFFF800000000000;
	.text : {
		*(.text)
	} :text
	.rodata : {
		*(.rodata .rodata.*)
	} :text
	.data : {
		*(.data .data.*)
		QUAD(ADDR(.bss)+SIZEOF(.bss))
	} :data
	.init_array : {
		__preinit_array_start = .;
		*(.preinit_array)
		__preinit_array_end = .;
		__init_array_start = .;
		*(.init_array .ctors)
		__init_array_end = .;
	} :data
	.fini_array : {
		__fini_array_start = .;
		*(.fini_array .dtors)
		__fini_array_end = .;
	} :data
	.bss (NOLOAD) : {
		__bss_start = .;
		*(.bss .bss.* COMMON)
		__bss_stop = .;
	} :data
}
