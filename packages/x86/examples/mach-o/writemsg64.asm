
include 'cpu/x64.inc'
use64

MachO.Settings.FileType equ MH_OBJECT
MachO.Settings.ProcessorType equ CPU_TYPE_X86_64
MachO.Settings.ProcessorSubtype equ CPU_SUBTYPE_X86_64_ALL
include 'format/macho.inc'

section '__TEXT':'__text'

public writemsg
public exit

	SYSCALL_CLASS_UNIX = 2
	SYSCALL_CLASS_SHIFT = 24
	define SYSCALL_CONSTRUCT_UNIX SYSCALL_CLASS_UNIX shl SYSCALL_CLASS_SHIFT +

	SYS_exit = 1
	SYS_write = 4

writemsg:
	mov	rdi,rsi
	or	rcx,-1
	xor	al,al
	repne	scasb
	neg	rcx
	sub	rcx,2
	mov	rdx,rcx
	mov	rdi,1
	mov	rax,SYSCALL_CONSTRUCT_UNIX(SYS_write)
	syscall
	retn

exit:
	mov	rdi,rax
	mov	rax,SYSCALL_CONSTRUCT_UNIX(SYS_exit)
	syscall
