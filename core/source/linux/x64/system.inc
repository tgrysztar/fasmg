
LINE_FEED = 0Ah

O_ACCMODE  = 0003o
O_RDONLY   = 0000o
O_WRONLY   = 0001o
O_RDWR	   = 0002o
O_CREAT    = 0100o
O_EXCL	   = 0200o
O_NOCTTY   = 0400o
O_TRUNC    = 1000o
O_APPEND   = 2000o
O_NONBLOCK = 4000o

S_ISUID    = 4000o
S_ISGID    = 2000o
S_ISVTX    = 1000o
S_IRUSR    = 0400o
S_IWUSR    = 0200o
S_IXUSR    = 0100o
S_IRGRP    = 0040o
S_IWGRP    = 0020o
S_IXGRP    = 0010o
S_IROTH    = 0004o
S_IWOTH    = 0002o
S_IXOTH    = 0001o

system_init:
	mov	eax,201 		; sys_time
	mov	edi,timestamp
	syscall
	call	minit
	retn

system_shutdown:
	call	mcheck
	retn

open:
; in: edx - path to file
; out: ebx = file handle, cf set on error
; preserves: esi, edi
	push	rsi rdi
	call	adapt_path
	mov	eax,2			; sys_open
	mov	esi,O_RDONLY
	xor	edx,edx
	syscall
	pop	rdi rsi
	test	eax,eax
	js	interface_error
	mov	ebx,eax
	clc
	ret
     interface_error:
	stc
	ret
   adapt_path:
	xor	ecx,ecx
	mov	edi,path_buffer
      copy_path:
	mov	al,[edx+ecx]
	cmp	al,'\'
	jne	path_char_ok
	mov	al,'/'
      path_char_ok:
	cmp	ecx,1000h
	jae	out_of_memory
	mov	[edi+ecx],al
	inc	ecx
	test	al,al
	jnz	copy_path
	retn
create:
; in: edx - path to file
; out: ebx = file handle, cf set on error
; preserves: esi, edi
	push	rsi rdi
	call	adapt_path
	mov	esi,O_CREAT+O_TRUNC+O_WRONLY
	mov	edx,S_IRUSR+S_IWUSR+S_IRGRP+S_IROTH
	mov	eax,2			; sys_open
	syscall
	pop	rdi rsi
	test	eax,eax
	js	interface_error
	mov	ebx,eax
	clc
	retn
write:
; in: ebx = file handle, edx - data, ecx = number of bytes
; out: cf set on error
; preserves: ebx, esi, edi
	push	rsi rdi
	mov	eax,1			; sys_write
	mov	edi,ebx
	mov	esi,edx
	mov	edx,ecx
	syscall
	pop	rdi rsi
	test	eax,eax
	js	interface_error
	cmp	eax,edx
	jne	interface_error
	clc
	ret
read:
; in: ebx = file handle, edx - buffer, ecx = number of bytes
; out: cf set on error
; preserves: ebx, esi, edi
	push	rsi rdi
	mov	eax,0			; sys_read
	mov	edi,ebx
	mov	esi,edx
	mov	edx,ecx
	syscall
	pop	rdi rsi
	test	eax,eax
	js	interface_error
	cmp	eax,edx
	jne	interface_error
	clc
	ret
close:
; in: ebx = file handle
; preserves: ebx, esi, edi
	push	rdi
	mov	edi,ebx
	mov	eax,3			; sys_close
	syscall
	pop	rdi
	ret
lseek:
; in: ebx = file handle, cl = method, edx:eax = offset
; out: edx:eax = new offset from the beginning of file, cf set on error
; preserves: ebx, esi, edi
	push	rsi rdi
	mov	edi,ebx
	mov	esi,edx
	mov	eax,eax
	shl	rsi,32
	or	rsi,rax
	xor	edx,edx
	mov	dl,cl
	mov	eax,8			; sys_lseek
	syscall
	pop	rdi rsi
	cmp	rax,-1
	je	interface_error
	mov	rdx,rax
	shr	rdx,32
	clc
	ret

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
	push	rbx rsi rbp
	test	ecx,ecx
	jnz	write_string_to_stdout
	xor	al,al
	mov	edi,esi
	or	ecx,-1
	repne	scasb
	neg	ecx
	sub	ecx,2
   write_string_to_stdout:
	mov	eax,1			; sys_write
	mov	edi,1
	mov	edx,ecx
	syscall
	pop	rbp rsi rbx
	retn

display_error_string:
; in:
;  esi - string
;  ecx = string length, zero for ASCIIZ string
; preserves: ebx, esi
	push	rbx rsi rbp
	test	ecx,ecx
	jnz	write_string_to_stderr
	xor	al,al
	mov	edi,esi
	or	ecx,-1
	repne	scasb
	neg	ecx
	sub	ecx,2
   write_string_to_stderr:
	mov	eax,1			; sys_write
	mov	edi,2
	mov	edx,ecx
	syscall
	pop	rbp rsi rbx
	retn

get_environment_variable:
; in:
;  esi - name
;  edi - buffer for value
;  ecx = size of buffer
; out:
;  eax = length of value
; preserves: ebx, esi, edi
	push	rbx rcx
	mov	rdx,[env]
      scan_environment:
	mov	rbx,[rdx]
	test	rbx,rbx
	jz	no_environment_variable
	xor	ecx,ecx
      compare_character:
	mov	al,[rbx+rcx]
	mov	ah,[esi+ecx]
	inc	ecx
	cmp	al,'='
	je	end_of_variable_name
	test	ah,ah
	jz	next_variable
	sub	ah,al
	je	compare_character
	cmp	ah,20h
	jne	next_variable
	cmp	al,'A'
	jb	next_variable
	cmp	al,'Z'
	jna	compare_character
      next_variable:
	add	rdx,8
	jmp	scan_environment
      end_of_variable_name:
	test	ah,ah
	jnz	next_variable
	add	rbx,rcx
	pop	rcx
	xor	eax,eax
      copy_environment_variable:
	mov	dl,[rbx+rax]
	cmp	eax,ecx
	jae	next_environment_variable_character
	mov	[edi+eax],dl
      next_environment_variable_character:
	inc	eax
	test	dl,dl
	jnz	copy_environment_variable
      environment_variable_ok:
	pop	rbx
	ret
      no_environment_variable:
	pop	rcx
	mov	eax,1
	jecxz	environment_variable_ok
	and	byte [edi],0
	pop	rbx
	ret
