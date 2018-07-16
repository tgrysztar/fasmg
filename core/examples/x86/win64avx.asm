
include 'format/format.inc'

format PE64 NX GUI 5.0
entry start

include 'ext/avx.inc'

section '.data' data readable writeable

  _title db 'AVX playground',0
  _error db 'AVX instructions are not supported.',0

  x dq 3.14159265389

  vector_output:
    repeat 16, i:0
	db 'ymm',`i,': %f,%f,%f,%f',13,10
    end repeat
    db 0

  buffer db 1000h dup ?

section '.text' code readable executable

  start:

	mov	eax,1
	cpuid
	and	ecx,18000000h
	cmp	ecx,18000000h
	jne	no_AVX
	xor	ecx,ecx
	xgetbv
	and	eax,110b
	cmp	eax,110b
	jne	no_AVX

	vbroadcastsd	ymm0, [x]
	vsqrtpd 	ymm1, ymm0

	vsubpd		ymm2, ymm0, ymm1
	vsubpd		ymm3, ymm1, ymm2

	vaddpd		xmm4, xmm2, xmm3
	vaddpd		ymm5, ymm4, ymm0

	vperm2f128	ymm6, ymm4, ymm5, 03h
	vshufpd 	ymm7, ymm6, ymm5, 10010011b

	vroundpd	ymm8, ymm7, 0011b
	vroundpd	ymm9, ymm7, 0

	sub	rsp,418h

    repeat 16, i:0
	vmovups [rsp+10h+i*32],ymm#i
    end repeat

	mov	r8,[rsp+10h]
	mov	r9,[rsp+18h]
	lea	rdx,[vector_output]
	lea	rcx,[buffer]
	call	[sprintf]

	xor	ecx,ecx
	lea	rdx,[buffer]
	lea	r8,[_title]
	xor	r9d,r9d
	call	[MessageBoxA]

	xor	ecx,ecx
	call	[ExitProcess]

  no_AVX:

	sub	rsp,28h

	xor	ecx,ecx
	lea	rdx,[_error]
	lea	r8,[_title]
	mov	r9d,10h
	call	[MessageBoxA]

	mov	ecx,1
	call	[ExitProcess]

section '.idata' import data readable writeable

  dd 0,0,0,RVA kernel_name,RVA kernel_table
  dd 0,0,0,RVA user_name,RVA user_table
  dd 0,0,0,RVA msvcrt_name,RVA msvcrt_table
  dd 0,0,0,0,0

  kernel_table:
    ExitProcess dq RVA _ExitProcess
    dq 0
  user_table:
    MessageBoxA dq RVA _MessageBoxA
    dq 0
  msvcrt_table:
    sprintf dq RVA _sprintf
    dq 0

  kernel_name db 'KERNEL32.DLL',0
  user_name db 'USER32.DLL',0
  msvcrt_name db 'MSVCRT.DLL',0

  _ExitProcess dw 0
    db 'ExitProcess',0
  _MessageBoxA dw 0
    db 'MessageBoxA',0
  _sprintf dw 0
    db 'sprintf',0
