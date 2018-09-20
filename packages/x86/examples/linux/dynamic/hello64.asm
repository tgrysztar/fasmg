
include 'format/format.inc'

format ELF64 executable ELFOSABI_LINUX
entry start

include 'import64.inc'

interpreter '/lib64/ld-linux-x86-64.so.2'
needed 'libc.so.6'
import printf,exit

segment readable executable

start:

	lea	rdi,[msg]
	xor	eax,eax
	call	[printf]

	call	[exit]

segment readable writeable

msg db 'Hello world!',0xA,0
