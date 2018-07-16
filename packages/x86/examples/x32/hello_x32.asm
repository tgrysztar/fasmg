
format ELFx32 executable ELFOSABI_LINUX

segment readable executable

entry start

start:

	mov	   edx,msg_size
	lea	   esi,[msg]
	mov	   edi,1		; STDOUT
	mov	   eax,1		; sys_write
	syscall

	xor	   edi,edi
	mov	   eax,60		 ; sys_exit
	syscall

segment readable writeable

msg db 'Hello x32 ABI!',0xA
msg_size = $-msg
