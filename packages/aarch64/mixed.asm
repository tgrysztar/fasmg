
; Switch to select code to be executed (both are present in the executable):
PE.Settings.Machine = IMAGE_FILE_MACHINE_AMD64
;PE.Settings.Machine = IMAGE_FILE_MACHINE_ARM64

format binary as 'exe'

PE.Settings.Stub = 'nul'
PE.Settings.Magic = 0x20B
PE.Settings.ImageBase = 0x140000000
PE.Settings.Characteristics = IMAGE_FILE_EXECUTABLE_IMAGE + IMAGE_FILE_LARGE_ADDRESS_AWARE
PE.Settings.DllCharacteristics = IMAGE_DLLCHARACTERISTICS_NX_COMPAT + IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE
PE.Settings.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_GUI
include '../x86/include/format/pe.inc'

include '../x86/include/win64a.inc'

; Define global anchors for the namespaces:
define X86
define ARM64

; Put instruction sets into isolated namespaces:
namespace X86
	include '../x86-2/x86-2.inc'
	use everything, 64
end namespace

namespace ARM64
	include 'iset/aarch64.inc'
	define xIP0? x16
	define xIP1? x17
end namespace

if PE.Settings.Machine = IMAGE_FILE_MACHINE_ARM64
	entry ARM64.Main
else
	entry X86.Main
end if

section '.text' code readable executable

    namespace X86
	Main:
		sub	rsp,8*5

		mov	r9d,0
		lea	r8,[Caption]
		lea	rdx,[Message]
		mov	rcx,0
		call	[MessageBoxA]

		mov	ecx,eax
		call	[ExitProcess]
    end namespace

    namespace ARM64
		align	4
	Main:
		mov	x0,0
		adr	x1,Message
		adr	x2,Caption
		bl	goto_MessageBoxA

		bl	goto_ExitProcess

	goto_MessageBoxA:
		adrp	xip0,MessageBoxA
		ldr	xip0,[xip0,(RVA MessageBoxA) and 0FFFh]
		br	xip0

	goto_ExitProcess:
		adrp	xip0,ExitProcess
		ldr	xip0,[xip0,(RVA ExitProcess) and 0FFFh]
		br	xip0
    end namespace

section '.data' data readable writeable

     Caption db 'Mixed codes',0

     namespace X86
	 Message db 'Hello from x86-64!',0
     end namespace

     namespace ARM64
	 Message db 'Hello from aarch64!',0
     end namespace

section '.idata' import data readable writeable

     library kernel32,'KERNEL32.DLL',\
	     user32,'USER32.DLL'

     include '../x86/include/api/kernel32.inc'
     include '../x86/include/api/user32.inc'

section '.reloc' fixups data readable discardable
