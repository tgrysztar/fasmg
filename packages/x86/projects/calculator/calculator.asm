
format PE GUI 4.0
entry start

include 'win32ax.inc'


struct MEMORY_REGION
	address dd ?
	size dd ?
ends

EXPRESSION_MAX_LENGTH = 32767

IDR_CALCULATOR = 37

ID_EXPRESSION  = 100
ID_BINARY      = 102
ID_DECIMAL     = 110
ID_HEXADECIMAL = 116
ID_VERSION     = 199


section '.text' code readable executable

  start:

	invoke	VirtualAlloc,0,100000h,MEM_RESERVE,PAGE_READWRITE
	mov	[aout.address],eax
	mov	[aout.size],0
	invoke	VirtualAlloc,0,100000h,MEM_RESERVE,PAGE_READWRITE
	mov	[conv.address],eax
	mov	[conv.size],0

	invoke	GetModuleHandle,0
	invoke	DialogBoxParam,eax,IDR_CALCULATOR,HWND_DESKTOP,CalculatorDialog,0
	invoke	ExitProcess,0

proc CalculatorDialog hwnd,msg,wparam,lparam
	push	ebx esi edi
	cmp	[msg],WM_INITDIALOG
	je	init
	cmp	[msg],WM_COMMAND
	je	command
	cmp	[msg],WM_CLOSE
	je	close
	xor	eax,eax
	jmp	finish
    init:

	invoke	fasmg_GetVersion

	invoke	SetDlgItemText,[hwnd],ID_VERSION,eax

	jmp	processed
    command:
	cmp	[wparam],IDCANCEL
	je	close
	cmp	[wparam],IDOK
	je	processed

	cmp	[wparam],ID_EXPRESSION + EN_CHANGE shl 16
	jne	processed

	invoke	GetDlgItemText,[hwnd],ID_EXPRESSION,expression_buffer,EXPRESSION_MAX_LENGTH

	invoke	fasmg_Assemble,source_string,NULL,aout,NULL,NULL,NULL
	test	eax,eax
	jnz	error

	mov	eax,[aout.size]
	lea	eax,[eax*8+8]
	cmp	eax,[conv.size]
	jbe	convert_output
	mov	[conv.size],eax
	invoke	VirtualAlloc,[conv.address],eax,MEM_COMMIT,PAGE_READWRITE
	test	eax,eax
	jnz	convert_output
	invoke	VirtualFree,[conv.address],0,MEM_RELEASE
	invoke	VirtualAlloc,0,[conv.size],MEM_COMMIT,PAGE_READWRITE
	test	eax,eax
	jz	error
	mov	[conv.address],eax
    convert_output:

	mov	esi,[aout.address]
	mov	edx,[aout.size]
	xor	cl,cl
	mov	edi,[conv.address]
	add	edi,[conv.size]
	sub	edi,2
	mov	word [edi],'b'
      to_bin:
	mov	al,[esi]
	shr	al,cl
	and	al,1
	add	al,'0'
	dec	edi
	mov	[edi],al
	inc	cl
	and	cl,111b
	jnz	to_bin
	inc	esi
	dec	edx
	jnz	to_bin
	test	byte [esi-1],80h
	jz	bin_ok
	dec	edi
	mov	ecx,3
	mov	al,'.'
	std
	rep	stosb
	cld
	inc	edi
      bin_ok:
	invoke	SetDlgItemText,[hwnd],ID_BINARY,edi

	mov	esi,[aout.address]
	mov	ecx,[aout.size]
	mov	edi,[conv.address]
	add	edi,[conv.size]
	sub	edi,2
	mov	word [edi],'h'
      to_hex:
	mov	al,[esi]
	and	al,0Fh
	cmp	al,10
	sbb	al,69h
	das
	dec	edi
	mov	[edi],al
	lodsb
	shr	al,4
	cmp	al,10
	sbb	al,69h
	das
	dec	edi
	mov	[edi],al
	loop	to_hex
	test	byte [esi-1],80h
	jz	hex_ok
	dec	edi
	mov	ecx,3
	mov	al,'.'
	std
	rep	stosb
	cld
	inc	edi
      hex_ok:
	invoke	SetDlgItemText,[hwnd],ID_HEXADECIMAL,edi

	mov	esi,[aout.address]
	mov	ecx,[aout.size]
	mov	edi,[conv.address]
	rep	movsb
	test	byte [esi-1],80h
	jnz	negative
	xor	eax,eax
	stosd
	jmp	to_dec
      negative:
	or	eax,-1
	stosd
	mov	esi,[conv.address]
	mov	ecx,edi
	sub	ecx,esi
	stc
      negate:
	not	byte [esi]
	adc	byte [esi],0
	inc	esi
	loop	negate
      to_dec:
	mov	edi,[conv.address]
	add	edi,[conv.size]
	dec	edi
	and	byte [edi],0
	mov	esi,[conv.address]
	mov	ecx,[aout.size]
	dec	ecx
	and	ecx,not 11b
      obtain_digit:
	xor	edx,edx
      divide_highest_dwords:
	mov	eax,[esi+ecx]
	call	div10
	test	eax,eax
	jnz	more_digits_to_come
	sub	ecx,4
	jnc	divide_highest_dwords
      store_final_digit:
	add	dl,'0'
	dec	edi
	mov	[edi],dl
	mov	esi,[aout.address]
	add	esi,[aout.size]
	test	byte [esi-1],80h
	jz	dec_ok
	dec	edi
	mov	byte [edi],'-'
	jmp	dec_ok
      div10:
	push	ebx ecx
	push	eax
	mov	ebx,eax
	mov	ecx,edx
	shld	edx,eax,2
	sub	ebx,edx
	sbb	ecx,0
	mov	eax,ebx
	mov	ebx,1999999Ah
	mul	ebx
	mov	eax,ecx
	imul	eax,ebx
	add	eax,edx
	pop	edx
	imul	ecx,eax,10
	sub	edx,ecx
	cmp	edx,10
	jb	div10_done
	sub	edx,10
	inc	eax
      div10_done:
	pop	ecx ebx
	retn
      more_digits_to_come:
	mov	ebx,ecx
      divide_remaining_dwords:
	mov	[esi+ebx],eax
	sub	ebx,4
	jc	store_digit
	mov	eax,[esi+ebx]
	call	div10
	jmp	divide_remaining_dwords
      store_digit:
	add	dl,'0'
	dec	edi
	mov	[edi],dl
	jmp	obtain_digit
      dec_ok:
	invoke	SetDlgItemText,[hwnd],ID_DECIMAL,edi

	jmp	processed
    error:
	invoke	SetDlgItemText,[hwnd],ID_BINARY,error_string
	invoke	SetDlgItemText,[hwnd],ID_DECIMAL,error_string
	invoke	SetDlgItemText,[hwnd],ID_HEXADECIMAL,error_string
	jmp	processed
    close:
	invoke	EndDialog,[hwnd],0
    processed:
	mov	eax,1
    finish:
	pop	edi esi ebx
	ret
endp


section '.data' data readable writeable

    error_string:
	db 0

    source_string:
	db 10,'if $ < 0'
	db 10,'emit (bsr (1 or not $) + 1) shr 3 + 1: +$'
	db 10,'else'
	db 10,'emit (bsr (1 or $) + 1) shr 3 + 1: +$'
	db 10,'end if'
	db 10,'$ = '
    expression_buffer db EXPRESSION_MAX_LENGTH dup ?

    aout MEMORY_REGION
    conv MEMORY_REGION


section '.idata' import data readable writeable

  library kernel,'KERNEL32.DLL',\
	  user,'USER32.DLL',\
	  fasmg,'FASMG.DLL'

  import kernel,\
	 GetModuleHandle,'GetModuleHandleA',\
	 VirtualAlloc,'VirtualAlloc',\
	 VirtualFree,'VirtualFree',\
	 ExitProcess,'ExitProcess'

  import user,\
	 DialogBoxParam,'DialogBoxParamA',\
	 GetDlgItemText,'GetDlgItemTextA',\
	 SetDlgItemText,'SetDlgItemTextA',\
	 EndDialog,'EndDialog'

  import fasmg,\
	 fasmg_GetVersion,'fasmg_GetVersion',\
	 fasmg_Assemble,'fasmg_Assemble'


section '.rsrc' resource data readable

  directory RT_DIALOG,dialogs

  resource dialogs,\
	   IDR_CALCULATOR,LANG_ENGLISH+SUBLANG_DEFAULT,calculator_dialog

  dialog calculator_dialog,'fasmg-powered calculator',100,120,380,64,WS_CAPTION+WS_POPUP+WS_SYSMENU+DS_MODALFRAME
    dialogitem 'STATIC','&Expression:',-1,4,8,44,8,WS_VISIBLE+SS_RIGHT
    dialogitem 'EDIT','',ID_EXPRESSION,52,6,320,12,WS_VISIBLE+WS_BORDER+WS_TABSTOP+ES_AUTOHSCROLL
    dialogitem 'STATIC','&Decimal:',-1,4,20,44,8,WS_VISIBLE+SS_RIGHT
    dialogitem 'EDIT','',ID_DECIMAL,52,18,320,12,WS_VISIBLE+WS_BORDER+WS_TABSTOP+ES_AUTOHSCROLL+ES_READONLY
    dialogitem 'STATIC','&Hexadecimal:',-1,4,32,44,8,WS_VISIBLE+SS_RIGHT
    dialogitem 'EDIT','',ID_HEXADECIMAL,52,30,320,12,WS_VISIBLE+WS_BORDER+WS_TABSTOP+ES_AUTOHSCROLL+ES_READONLY
    dialogitem 'STATIC','&Binary:',-1,4,44,44,8,WS_VISIBLE+SS_RIGHT
    dialogitem 'EDIT','',ID_BINARY,52,42,320,12,WS_VISIBLE+WS_BORDER+WS_TABSTOP+ES_AUTOHSCROLL+ES_READONLY
    dialogitem 'STATIC','fasm g.',-1,326,55,30,8,WS_VISIBLE+SS_RIGHT
    dialogitem 'STATIC','',ID_VERSION,356,55,20,8,WS_VISIBLE+SS_LEFT
  enddialog
