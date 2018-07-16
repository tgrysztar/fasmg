
include 'cpu/x64.inc'
use64

MachO.Settings.ProcessorType equ CPU_TYPE_X86_64
MachO.Settings.BaseAddress = 0x1000000

include 'format/macho.inc'

interpreter '/usr/lib/dyld'
uses '/usr/lib/libSystem.B.dylib' (1.0.0, 1226.10.1)
import printf,'_printf'
import exit,'_exit'

segment '__TEXT' readable executable

  section '__text' align 16

    entry start

    start:

	and	rsp, 0xFFFFFFFFFFFFFFF0
	sub	rsp, 0x10

	lea	rdi,[msg]
	call	printf

	xor	rdi,rdi
	call	exit

  section '__cstring' align 4

	msg db 'This is a dynamically linked 64-bit Mach-O executable.',0Ah,0



