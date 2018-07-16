
include 'win64a.inc'

format PE64 console DLL
entry DllEntryPoint

section '.text' code readable executable

proc DllEntryPoint hinstDLL,fdwReason,lpvReserved
	mov	eax,TRUE
	ret
endp

proc WriteMessage uses rbx rsi rdi, message
	mov	rdi,rcx 			; first parameter passed in RCX
	invoke	GetStdHandle,STD_OUTPUT_HANDLE
	mov	rbx,rax
	xor	al,al
	or	rcx,-1
	repne	scasb
	dec	rdi
	mov	r8,-2
	sub	r8,rcx
	sub	rdi,r8
	invoke	WriteFile,rbx,rdi,r8,bytes_count,0
	ret
endp

section '.bss' data readable writeable

  bytes_count dd ?

section '.edata' export data readable

  export 'WRITEMSG.DLL',\
	  WriteMessage,'WriteMessage'

section '.reloc' fixups data readable discardable

section '.idata' import data readable writeable

  library kernel32,'KERNEL32.DLL'

  include 'api/kernel32.inc'
