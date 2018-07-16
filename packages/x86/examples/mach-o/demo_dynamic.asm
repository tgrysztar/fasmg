
include 'cpu/80386.inc'
use32

MachO.Settings.BaseAddress = 0x1000

include 'format/macho.inc'

interpreter '/usr/lib/dyld'
uses '/usr/lib/libSystem.B.dylib' (1.0.0, 1225.0.0)
import printf,'_printf'
import exit,'_exit'

segment '__TEXT' readable executable

  section '__text' align 16

  entry start

  start:
	and	esp,0FFFFFFF0h
	sub	esp,10h

	mov	dword [esp],msg
	call	printf

	and	dword [esp],0
	call	exit

  section '__cstring' align 4

	msg db 'This is a dynamically linked 32-bit Mach-O executable.',0Ah,0
