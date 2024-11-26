
format binary as 'exe'

PE.Settings.Stub = 'nul'
PE.Settings.Magic = 0x20B
PE.Settings.Machine = IMAGE_FILE_MACHINE_ARM64
PE.Settings.ImageBase = 0x140000000
PE.Settings.Characteristics = IMAGE_FILE_EXECUTABLE_IMAGE + IMAGE_FILE_LARGE_ADDRESS_AWARE
PE.Settings.DllCharacteristics = IMAGE_DLLCHARACTERISTICS_NX_COMPAT + IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE
PE.Settings.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_GUI
include '../x86/include/format/pe.inc'

include 'iset/aarch64.inc'

define xIP0? x16	; Windows-specific aliases
define xIP1? x17

section '.text' code readable executable

	entry $

		mov	x0,0
		adr	x1,_message
		adr	x2,_caption
		mov	x3,0
		bl	MessageBoxA

		bl	ExitProcess

	MessageBoxA:
		adr	xip0,imp__MessageBoxA
		ldr	xip0,[xip0]
	    ;	 adrp	 xip0,imp__MessageBoxA
	    ;	 ldr	 xip0,[xip0,(imp__MessageBoxA-PE.IMAGE_BASE) and 0FFFh]
		br	xip0

	ExitProcess:
		adr	xip0,imp__ExitProcess
		ldr	xip0,[xip0]
	    ;	 adrp	 xip0,imp__ExitProcess
	    ;	 ldr	 xip0,[xip0,(imp__ExitProcess-PE.IMAGE_BASE) and 0FFFh]
		br	xip0

section '.data' data readable writeable

  _caption db 'Windows on ARM64',0
  _message db 'Hello, world of assembly!',0

section '.idata' import data readable writeable

  dd 0,0,0,RVA kernel_name,RVA kernel_table
  dd 0,0,0,RVA user_name,RVA user_table
  dd 0,0,0,0,0

  kernel_table:
    imp__ExitProcess dq RVA _ExitProcess
    dq 0
  user_table:
    imp__MessageBoxA dq RVA _MessageBoxA
    dq 0

  kernel_name db 'KERNEL32.DLL',0
  user_name db 'USER32.DLL',0

  _ExitProcess dw 0
    db 'ExitProcess',0
  _MessageBoxA dw 0
    db 'MessageBoxA',0

section '.reloc' fixups data readable discardable
