
include 'format/format.inc'

format ELF executable ELFOSABI_LINUX
entry start

include 'import32.inc'

interpreter '/lib/ld-linux.so.2'
needed 'libc.so.6'
import printf,exit

segment readable executable

start:
	sub	esp,12
	push	msg
	call	[printf]
	add	esp,16

	call	[exit]

segment readable writeable

msg db 'Hello world!',0xA,0
