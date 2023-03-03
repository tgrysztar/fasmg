

include 'format/format.inc'

format MachO64 executable

segment '__TEXT' readable executable

	SYSCALL_CLASS_UNIX = 2
	SYSCALL_CLASS_SHIFT = 24
	define SYSCALL_UNIX SYSCALL_CLASS_UNIX shl SYSCALL_CLASS_SHIFT +

	SYS_exit = 1
	SYS_write = 4

	entry $, rdi: 1, rsi: msg

	mov	rdx,msg.length
	mov	rax,SYSCALL_UNIX(SYS_write)
	syscall
	mov	rdi,rax
	mov	rax,SYSCALL_UNIX(SYS_exit)
	syscall

	msg db 'This is a simple 64-bit Mach-O executable using only syscalls.',0Ah
	.length = $ - msg
