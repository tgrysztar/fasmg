
LINE_FEED equ 13,10

system_init:
	invoke	GetStdHandle,STD_OUTPUT_HANDLE
	mov	[stdout],eax
	invoke	GetStdHandle,STD_ERROR_HANDLE
	mov	[stderr],eax
	invoke	HeapCreate,0,20000h,0
	mov	[memory],eax
	test	eax,eax
	jz	out_of_memory
	invoke	GetSystemTime,systemtime
	invoke	SystemTimeToFileTime,systemtime,filetime
	mov	ebx,[filetime.dwLowDateTime]
	mov	eax,[filetime.dwHighDateTime]
	sub	ebx,116444736000000000 and 0FFFFFFFFh
	sbb	eax,116444736000000000 shr 32
	xor	edx,edx
	mov	ecx,10000000
	div	ecx
	mov	dword [timestamp+4],eax
	mov	eax,ebx
	div	ecx
	mov	dword [timestamp],eax
	retn

system_shutdown:
	cmp	[memory],0
	je	memory_released
	invoke	HeapDestroy,[memory]
    memory_released:
	invoke	CloseHandle,[stdout]
	invoke	CloseHandle,[stderr]
	retn

malloc:
malloc_fixed:
malloc_growable:
; in: ecx = requested size
; out: eax - allocated block, ecx = allocated size, on error jumps to out_of_memory (does not return)
; preserves: ebx, esi, edi
; note:
;  use of malloc_fixed hints that block will be kept as is until the end of assembly
;  use of malloc_growable hints that block is likely to be resized
	invoke	HeapAlloc,[memory],0,ecx
	test	eax,eax
	jz	out_of_memory
    memory_allocated:
	push	eax
	invoke	HeapSize,[memory],0,eax
	mov	ecx,eax
	pop	eax
	cmp	ecx,-1
	je	out_of_memory
	retn
realloc:
; in: eax - memory block, ecx = requested size
; out: eax - resized block, ecx = allocated size, on error jumps to out_of_memory (does not return)
; preserves: ebx, esi, edi
	invoke	HeapReAlloc,[memory],0,eax,ecx
	test	eax,eax
	jnz	memory_allocated
	jmp	out_of_memory
mfree:
; in: eax - memory block
; out: cf set on error
; preserves: ebx, esi, edi
; note: eax may have value 0 or -1, it should be treated as invalid input then
	test	eax,eax
	jz	interface_error
	cmp	eax,-1
	je	interface_error
	invoke	HeapFree,[memory],0,eax
	test	eax,eax
	jz	interface_error
	clc
	retn
    interface_error:
	stc
	retn

open:
; in: edx - path to file
; out: ebx = file handle, cf set on error
; preserves: esi, edi
	invoke	CreateFile,edx,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0
	cmp	eax,-1
	je	interface_error
	mov	ebx,eax
	clc
	retn
create:
; in: edx - path to file
; out: ebx = file handle, cf set on error
; preserves: esi, edi
	invoke	CreateFile,edx,GENERIC_WRITE,0,0,CREATE_ALWAYS,0,0
	cmp	eax,-1
	je	interface_error
	mov	ebx,eax
	clc
	retn
write:
; in: ebx = file handle, edx - data, ecx = number of bytes
; out: cf set on error
; preserves: ebx, esi, edi
	push	ecx
	invoke	WriteFile,ebx,edx,ecx,systmp,0
	pop	ecx
	test	eax,eax
	jz	interface_error
	cmp	ecx,[systmp]
	jne	interface_error
	clc
	retn
read:
; in: ebx = file handle, edx - buffer, ecx = number of bytes
; out: cf set on error
; preserves: ebx, esi, edi
	push	ecx
	invoke	ReadFile,ebx,edx,ecx,systmp,0
	pop	ecx
	test	eax,eax
	jz	interface_error
	cmp	ecx,[systmp]
	jne	interface_error
	clc
	retn
close:
; in: ebx = file handle
; preserves: ebx, esi, edi
	invoke	CloseHandle,ebx
	retn
lseek:
; in: ebx = file handle, cl = method, edx:eax = offset
; out: edx:eax = new offset from the beginning of file, cf set on error
; preserves: ebx, esi, edi
	movzx	ecx,cl
	mov	[systmp],edx
	invoke	SetFilePointer,ebx,eax,systmp,ecx
	cmp	eax,-1
	jne	lseek_ok
	invoke	GetLastError
	test	eax,eax
	jnz	interface_error
	not	eax
    lseek_ok:
	mov	edx,[systmp]
	clc
	retn

get_timestamp:
; out: edx:eax = timestamp
; preserves: ebx, ecx, esi, edi
	mov	eax,dword [timestamp]
	mov	edx,dword [timestamp+4]
	retn

display_string:
; in:
;  esi - string
;  ecx = string length, zero for ASCIIZ string
; preserves: ebx, esi
	push	ebx
	mov	ebx,[stdout]
	jmp	write_string
display_error_string:
; in:
;  esi - string
;  ecx = string length, zero for ASCIIZ string
; preserves: ebx, esi
	push	ebx
	mov	ebx,[stderr]
   write_string:
	test	ecx,ecx
	jnz	write_string_to_stdout
	xor	al,al
	mov	edi,esi
	or	ecx,-1
	repne	scasb
	neg	ecx
	sub	ecx,2
   write_string_to_stdout:
	mov	edx,esi
   write_portion_to_stdout:
	mov	eax,51200
	cmp	ecx,eax
	jbe	final_write_to_stdout
	sub	ecx,eax
	add	eax,edx
	push	eax ecx
	invoke	WriteFile,ebx,edx,51200,systmp,0
	pop	ecx edx
	jmp	write_portion_to_stdout
   final_write_to_stdout:
	invoke	WriteFile,ebx,edx,ecx,systmp,0
	pop	ebx
	retn

get_environment_variable:
; in:
;  esi - name
;  edi - buffer for value
;  ecx = size of buffer
; out:
;  eax = length of value
; preserves: ebx, esi, edi
	push	ecx
	invoke	GetEnvironmentVariable,esi,edi,ecx
	pop	ecx
	cmp	eax,ecx
	jae	environment_variable_ready
	mov	byte [edi+eax],0
   environment_variable_ready:
	inc	eax
	retn

