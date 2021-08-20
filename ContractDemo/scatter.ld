OUTPUT_ARCH( "riscv" )
ENTRY( _start )
MEMORY
{
	ilm		: ORIGIN = 0x10002000, LENGTH = 24K
	dlm		: ORIGIN = 0x10010000, LENGTH = 8K
	stk		: ORIGIN = 0x10013000, LENGTH = 4K
}
SECTIONS
{
	.text : {
		*(.text.init)
		*(.text, .text.*)
		KEEP(*(.rodata))
		KEEP(*(.rodata.*))
	} >ilm

	.data :
	{
		*(.data)
	} > dlm
	.bss :
	{
		*(.bss)
	} >dlm
	.stack (NOLOAD): {
		. = . + LENGTH(stk);
		PROVIDE(_stack = .);
	} > stk
}