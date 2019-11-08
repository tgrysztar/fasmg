; Challenger
; Esoteric language inspired by Befunge and x86.
; Designed and implemented by Tomasz Grysztar in 2009.
; This code is public domain.
;
; The main similarity to Befunge is the instruction flow on a two-dimensional
; plane of characters, while a slight semblance of x86 is that this language
; is register-based.
;
; There are no input and output instructions - the character plane is similar
; to memory in x86 system, any portion can be used to present an output,
; analogously to something like a video memory. The intepreter displays
; the contents of the plane in real time.
;
; Each memory cell is a 32-bit signed value, presented as Unicode character.
; The program is loaded from UTF-8 text file and put somewhere on the plane.
;
; There are three pointers operated by the Challenger machine:
; instruction pointer (IP), data pointer (DP) and stack pointer (SP).
; The pointer not only determines the character cell on the plane, but also
; the direction of movement. They all begin in the left upper corner of
; the program, with direction to the right.
;
; Apart from the pointers, the machine has just one register, accumulator (A),
; which is used to perform the arithmetic operations and condition testing.
;
; The basic instruction set:
;  >	Set IP direction to "right"
;  <	Set IP direction to "left"
;  ^	Set IP direction to "up"
;  v	Set IP direction to "down"
;  |	Set IP direction to "down" if A=0, "up" otherwise
;  _	Set IP direction to "right" if A=0, "left" otherwise
;  #	Make IP skip the next cell ("trampoline")
;  .	Move DP one cell forward
;  :	Move DP forward by the number of cells specified in A
;  )	Rotate DP direction clockwise
;  (	Rotate DP direction counterclockwise
;  =	Set DP to be the same as IP
;  \	Exchange A with the cell at DP
;  +	Add the value of the cell at DP to A
;  -	Subtract the value of the cell at DP from A
;  *	Multiply A by the value of the cell at DP
;  /	Divide A by the value of the cell at DP
;  %	Calculate A modulo the value of the cell at DP
;  `	Replace the value in A with its sign flag (so if A<0, set A=1; set A=0 otherwise)
;  "	Load immediate value (the character that follows this instruction) to A
;  0-9	Load corresponding value in range 0-9 into A
;  A-F	Load corresponding value in range 10-15 into A (hexadecimal digits)
;  x	Generate an exception to stop program flow ("trap")
;
; The additional instructions of the Framed Challenger variant:
;  !	Move SP backwards one cell, and write A at this new position ("push" instruction)
;  ?	Read A from cell at SP and move SP forward one cell ("pop" instruction)
;  ,	Move SP one cell forward
;  ;	Move SP forward by the number of cells specified in A
;  ]	Rotate SP direction clockwise
;  [	Rotate SP direction counterclockwise
;  @	Set SP to be the same as IP
;  &	Exchange SP with DP
;
; Both the 0 and 32 (space) are NOP instructions.
; The uninitialized parts of plane are filled with 0 values.
; The initial value of accumulator is 0.
; The interpreter displays the pointers with a tiny arrows: IP is green, DP is red, SP is yellow.
; The instruction pointer wraps when it reaches the end of the initialized part of the plane.

format PE GUI 4.0
entry start

include 'win32wx.inc'	; switch to 'win32ax.inc' for Windows 9x

VERSION_STRING equ '1.01'

CHL_IDLE      = 0
CHL_RUNNING   = 1
CHL_STEPPING  = 2
CHL_EXCEPTION = -1

SEGMENT_SIZE	   = 100
SEGMENTS_PER_BLOCK = 100

struct PLANE_SEGMENT
  left_segment	 dd ?
  top_segment	 dd ?
  right_segment  dd ?
  bottom_segment dd ?
  content	 dd SEGMENT_SIZE dup ?
ends

struct PLANE_POINTER
  segment   dd ?
  offset    dd ?
  x	    dd ?
  y	    dd ?
  direction dd ?
ends

section '.text' code readable executable

start:

	call	initialize_machine

	invoke	GetModuleHandle,0
	mov	[plane_wc.hInstance],eax
	invoke	LoadIcon,0,IDI_APPLICATION
	mov	[plane_wc.hIcon],eax
	invoke	LoadCursor,0,IDC_HAND
	mov	[plane_wc.hCursor],eax
	invoke	GetStockObject,WHITE_BRUSH
	mov	[plane_wc.hbrBackground],eax
	invoke	RegisterClass,plane_wc
	test	eax,eax
	jz	startup_failed
	invoke	CreateWindowEx,0,_plane_class,_plane_title,WS_VISIBLE+WS_OVERLAPPEDWINDOW+WS_HSCROLL+WS_VSCROLL,80,80,620,500,NULL,NULL,[plane_wc.hInstance],NULL
	test	eax,eax
	jz	startup_failed
	mov	[hwnd_plane],eax
	invoke	CreateDialogParam,[plane_wc.hInstance],ID_CONTROLLER,[hwnd_plane],ControllerDialogProc,NULL
	test	eax,eax
	jz	startup_failed
	mov	[hwnd_controller],eax

  msg_loop:
	invoke	PeekMessage,msg,NULL,0,0,PM_NOREMOVE
	or	eax,eax
	jz	no_message
	invoke	GetMessage,msg,NULL,0,0
	cmp	eax,1
	jb	end_loop
	jne	msg_loop
	invoke	IsDialogMessage,[hwnd_controller],msg
	test	eax,eax
	jnz	msg_loop
	invoke	TranslateMessage,msg
	invoke	DispatchMessage,msg
	jmp	msg_loop
  no_message:
	cmp	[status],CHL_STEPPING
	je	machine_step
	cmp	[status],CHL_RUNNING
	jne	nothing_to_process
	cmp	[delay],0
	je	machine_step
	invoke	GetTickCount
	mov	edx,eax
	sub	eax,[clock]
	cmp	eax,[delay]
	jb	nothing_to_process
	mov	[clock],edx
  machine_step:
	call	refresh_at_pointers
	call	interpreter_step
	inc	[steps]
	call	refresh_at_pointers
	btr	[update_flags],0
	jnc	horizontal_range_ok
	invoke	PostMessage,[hwnd_plane],WM_HSCROLL,SB_ENDSCROLL,0
      horizontal_range_ok:
	btr	[update_flags],1
	jnc	vertical_range_ok
	invoke	PostMessage,[hwnd_plane],WM_VSCROLL,SB_ENDSCROLL,0
      vertical_range_ok:
	invoke	SetDlgItemInt,[hwnd_controller],ID_ACCUMULATOR,[accumulator],TRUE
	invoke	SetDlgItemInt,[hwnd_controller],ID_STEPS,[steps],TRUE
	cmp	[status],CHL_STEPPING
	jne	msg_loop
	mov	[status],CHL_IDLE
	invoke	SetDlgItemText,[hwnd_controller],ID_STATUS,""
	jmp	msg_loop
  nothing_to_process:
	invoke	Sleep,1
	jmp	msg_loop

  startup_failed:
	invoke	MessageBox,NULL,_startup_failed,_plane_title,MB_ICONERROR+MB_OK
	invoke	ExitProcess,-1
  out_of_memory:
	invoke	MessageBox,NULL,_out_of_memory,_plane_title,MB_ICONERROR+MB_OK
	invoke	ExitProcess,-2
  end_loop:
	invoke	ExitProcess,[msg.wParam]

interpreter_step:
	mov	esi,[p_instruction.segment]
	mov	eax,[p_instruction.offset]
	mov	eax,[esi+PLANE_SEGMENT.content+eax*4]
	test	eax,eax
	jz	instruction_interpreted
	cmp	eax,' '
	je	instruction_interpreted
	cmp	eax,'<'
	je	ip_left
	cmp	eax,'>'
	je	ip_right
	cmp	eax,'^'
	je	ip_up
	cmp	eax,'v'
	je	ip_down
	cmp	eax,'#'
	je	ip_trampoline
	cmp	eax,'|'
	je	ip_up_or_down
	cmp	eax,'_'
	je	ip_left_or_right
	cmp	eax,'.'
	je	dp_step
	cmp	eax,':'
	je	dp_jump
	cmp	eax,')'
	je	dp_rotate_clockwise
	cmp	eax,'('
	je	dp_rotate_counterclockwise
	cmp	eax,'='
	je	set_dp
	cmp	eax,'\'
	je	xchg_a
	cmp	eax,'+'
	je	add_a
	cmp	eax,'-'
	je	sub_a
	cmp	eax,'*'
	je	mul_a
	cmp	eax,'/'
	je	div_a
	cmp	eax,'%'
	je	mod_a
	cmp	eax,'`'
	je	sign_of_a
	cmp	eax,'"'
	je	load_immediate
	cmp	eax,'x'
	je	exception_1

	cmp	eax,'!'
	je	push_a
	cmp	eax,'?'
	je	pop_a
	cmp	eax,','
	je	sp_step
	cmp	eax,';'
	je	sp_jump
	cmp	eax,']'
	je	sp_rotate_clockwise
	cmp	eax,'['
	je	sp_rotate_counterclockwise
	cmp	eax,'@'
	je	set_sp
	cmp	eax,'&'
	je	xchg_sp_with_dp

	sub	eax,'0'
	jc	exception_6
	cmp	eax,10
	jb	value_ok
	sub	eax,'A'-'0'
	jc	exception_6
	cmp	eax,6
	ja	exception_6
	add	eax,10
    value_ok:
	mov	[accumulator],eax
	jmp	instruction_interpreted
    ip_left:
	mov	[p_instruction.direction],0
	jmp	instruction_interpreted
    ip_up:
	mov	[p_instruction.direction],1
	jmp	instruction_interpreted
    ip_left_or_right:
	cmp	[accumulator],0
	jne	ip_left
    ip_right:
	mov	[p_instruction.direction],2
	jmp	instruction_interpreted
    ip_up_or_down:
	cmp	[accumulator],0
	jne	ip_up
    ip_down:
	mov	[p_instruction.direction],3
	jmp	instruction_interpreted
    ip_trampoline:
	mov	ebx,p_instruction
	mov	ecx,1
	call	move_pointer
	jmp	instruction_interpreted
    dp_step:
	mov	ecx,1
      move_dp:
	mov	ebx,p_data
	call	move_pointer
	jmp	instruction_interpreted
    dp_jump:
	mov	ecx,[accumulator]
	jmp	move_dp
    dp_rotate_clockwise:
	mov	eax,[p_data.direction]
	inc	al
	and	al,3
	mov	[p_data.direction],eax
	jmp	instruction_interpreted
    dp_rotate_counterclockwise:
	mov	eax,[p_data.direction]
	dec	al
	and	al,3
	mov	[p_data.direction],eax
	jmp	instruction_interpreted
    set_dp:
	mov	esi,p_instruction
	mov	edi,p_data
	mov	ecx,sizeof.PLANE_POINTER/4
	rep	movsd
	jmp	instruction_interpreted
    xchg_a:
	mov	esi,[p_data.segment]
	mov	ecx,[p_data.offset]
	mov	eax,[esi+PLANE_SEGMENT.content+ecx*4]
	xchg	eax,[accumulator]
	mov	[esi+PLANE_SEGMENT.content+ecx*4],eax
	jmp	instruction_interpreted
    add_a:
	mov	esi,[p_data.segment]
	mov	ecx,[p_data.offset]
	mov	eax,[esi+PLANE_SEGMENT.content+ecx*4]
	add	[accumulator],eax
	jmp	instruction_interpreted
    sub_a:
	mov	esi,[p_data.segment]
	mov	ecx,[p_data.offset]
	mov	eax,[esi+PLANE_SEGMENT.content+ecx*4]
	sub	[accumulator],eax
	jmp	instruction_interpreted
    mul_a:
	mov	esi,[p_data.segment]
	mov	ecx,[p_data.offset]
	mov	eax,[esi+PLANE_SEGMENT.content+ecx*4]
	imul	[accumulator]
	mov	[accumulator],eax
	jmp	instruction_interpreted
    div_a:
	mov	esi,[p_data.segment]
	mov	ecx,[p_data.offset]
	mov	ecx,[esi+PLANE_SEGMENT.content+ecx*4]
	test	ecx,ecx
	jz	exception_0
	mov	eax,[accumulator]
	cdq
	idiv	ecx
	mov	[accumulator],eax
	jmp	instruction_interpreted
    mod_a:
	mov	esi,[p_data.segment]
	mov	ecx,[p_data.offset]
	mov	ecx,[esi+PLANE_SEGMENT.content+ecx*4]
	test	ecx,ecx
	jz	exception_0
	mov	eax,[accumulator]
	cdq
	idiv	ecx
	mov	[accumulator],edx
	jmp	instruction_interpreted
    sign_of_a:
	shr	[accumulator],31
	jmp	instruction_interpreted
    sp_step:
	mov	ecx,1
      move_sp:
	mov	ebx,p_stack
	call	move_pointer
	jmp	instruction_interpreted
    sp_jump:
	mov	ecx,[accumulator]
	jmp	move_sp
    sp_rotate_clockwise:
	mov	eax,[p_stack.direction]
	inc	al
	and	al,3
	mov	[p_stack.direction],eax
	jmp	instruction_interpreted
    sp_rotate_counterclockwise:
	mov	eax,[p_stack.direction]
	dec	al
	and	al,3
	mov	[p_stack.direction],eax
	jmp	instruction_interpreted
    set_sp:
	mov	esi,p_instruction
	mov	edi,p_stack
	mov	ecx,sizeof.PLANE_POINTER/4
	rep	movsd
	jmp	instruction_interpreted
    xchg_sp_with_dp:
	mov	esi,p_stack
	mov	edi,p_loader
	mov	ecx,sizeof.PLANE_POINTER/4
	rep	movsd
	mov	esi,p_data
	mov	edi,p_stack
	mov	ecx,sizeof.PLANE_POINTER/4
	rep	movsd
	mov	esi,p_loader
	mov	edi,p_data
	mov	ecx,sizeof.PLANE_POINTER/4
	rep	movsd
	jmp	instruction_interpreted
    push_a:
	mov	ecx,-1
	mov	ebx,p_stack
	call	move_pointer
	mov	esi,[ebx+PLANE_POINTER.segment]
	mov	ecx,[ebx+PLANE_POINTER.offset]
	mov	eax,[accumulator]
	mov	[esi+PLANE_SEGMENT.content+ecx*4],eax
	jmp	instruction_interpreted
    pop_a:
	mov	ebx,p_stack
	mov	esi,[ebx+PLANE_POINTER.segment]
	mov	ecx,[ebx+PLANE_POINTER.offset]
	mov	eax,[esi+PLANE_SEGMENT.content+ecx*4]
	mov	[accumulator],eax
	mov	ecx,1
	call	move_pointer
	jmp	instruction_interpreted
    load_immediate:
	mov	ebx,p_instruction
	mov	ecx,1
	call	move_pointer
	mov	esi,[p_instruction.segment]
	mov	ecx,[p_instruction.offset]
	mov	eax,[esi+PLANE_SEGMENT.content+ecx*4]
	mov	[accumulator],eax
    instruction_interpreted:
	mov	ebx,p_instruction
	mov	ecx,1
	mov	eax,[ebx+PLANE_POINTER.direction]
	cmp	eax,1
	jb	check_for_left_border
	je	check_for_top_border
	cmp	eax,3
	je	check_for_bottom_border
      check_for_right_border:
	mov	eax,[p_instruction.x]
	cmp	eax,[max_x]
	jl	advance_instruction_pointer
	mov	ecx,[min_x]
	sub	ecx,[max_x]
	jmp	advance_instruction_pointer
      check_for_left_border:
	mov	eax,[p_instruction.x]
	cmp	eax,[min_x]
	jg	advance_instruction_pointer
	mov	ecx,[min_x]
	sub	ecx,[max_x]
	jmp	advance_instruction_pointer
      check_for_bottom_border:
	mov	eax,[p_instruction.y]
	cmp	eax,[max_y]
	jl	advance_instruction_pointer
	mov	ecx,[min_y]
	sub	ecx,[max_y]
	jmp	advance_instruction_pointer
      check_for_top_border:
	mov	eax,[p_instruction.y]
	cmp	eax,[min_y]
	jg	advance_instruction_pointer
	mov	ecx,[min_y]
	sub	ecx,[max_y]
      advance_instruction_pointer:
	call	move_pointer
	ret

exception_0:
	invoke	SetDlgItemText,[hwnd_controller],ID_STATUS,_exception_0
	jmp	set_exception_status
exception_1:
	invoke	SetDlgItemText,[hwnd_controller],ID_STATUS,_exception_1
	jmp	set_exception_status
exception_6:
	invoke	SetDlgItemText,[hwnd_controller],ID_STATUS,_exception_6
      set_exception_status:
	mov	[status],CHL_EXCEPTION
	invoke	GetDlgItem,[hwnd_controller],ID_RUN
	invoke	EnableWindow,eax,FALSE
	invoke	GetDlgItem,[hwnd_controller],ID_STEP
	invoke	EnableWindow,eax,FALSE
	ret

initialize_machine:
	xor	eax,eax
	mov	[memory_bottom],eax
	mov	[memory_top],eax
	call	new_block
	xor	eax,eax
	mov	[window_segment],edx
	mov	[window_offset],eax
	mov	[window_x],eax
	mov	[window_y],eax
	mov	ecx,2
	mov	[p_instruction.segment],edx
	mov	[p_instruction.offset],eax
	mov	[p_instruction.x],eax
	mov	[p_instruction.y],eax
	mov	[p_instruction.direction],ecx
	mov	[p_data.segment],edx
	mov	[p_data.offset],eax
	mov	[p_data.x],eax
	mov	[p_data.y],eax
	mov	[p_data.direction],ecx
	mov	[p_stack.segment],edx
	mov	[p_stack.offset],eax
	mov	[p_stack.x],eax
	mov	[p_stack.y],eax
	mov	[p_stack.direction],ecx
	mov	[p_loader.segment],edx
	mov	[p_loader.offset],eax
	mov	[p_loader.x],eax
	mov	[p_loader.y],eax
	mov	[p_loader.direction],ecx
	mov	[accumulator],eax
	mov	[steps],eax
	mov	[min_x],eax
	mov	[min_y],eax
	mov	[max_x],eax
	mov	[max_y],eax
	mov	[status],CHL_IDLE
	ret

free_memory:
	mov	eax,[memory_bottom]
	test	eax,eax
	jz	memory_free
	mov	edx,[eax]
	mov	[memory_bottom],edx
	invoke	VirtualFree,eax,0,MEM_RELEASE
	jmp	free_memory
    memory_free:
	ret

new_segment:
; returns: edx - allocated segment
; preserves: ebx, esi
	mov	edx,[memory_cursor]
	lea	eax,[edx+sizeof.PLANE_SEGMENT]
	cmp	eax,[memory_top]
	jae	new_block
	mov	[memory_cursor],eax
	mov	edi,edx
	mov	ecx,6+SEGMENT_SIZE
	xor	eax,eax
	rep	stosd
	ret
    new_block:
	mov	[memory_top],4+sizeof.PLANE_SEGMENT*SEGMENTS_PER_BLOCK
	invoke	VirtualAlloc,NULL,[memory_top],MEM_COMMIT,PAGE_READWRITE
	test	eax,eax
	jz	out_of_memory
	add	[memory_top],eax
	lea	edx,[eax+4]
	mov	[memory_cursor],edx
	xchg	eax,[memory_bottom]
	mov	[edx-4],eax
	jmp	new_segment

move_pointer:
; ebx - plane pointer structure
; ecx = amount of cells to move by
; preserves: ebx
	mov	al,byte [ebx+PLANE_POINTER.direction]
	test	ecx,ecx
	jz	pointer_movement_done
	jns	pointer_direction_ok
	neg	ecx
	xor	al,2
    pointer_direction_ok:
	cmp	al,1
	jb	go_left
	je	go_up
	cmp	al,3
	je	go_down
    go_right:
	mov	eax,[ebx+PLANE_POINTER.x]
	add	eax,ecx
	mov	[ebx+PLANE_POINTER.x],eax
	cmp	[max_x],eax
	jge	move_pointer_right
	mov	[max_x],eax
	bts	[update_flags],0
    move_pointer_right:
	mov	eax,[ebx+PLANE_POINTER.offset]
	add	eax,ecx
	cmp	eax,SEGMENT_SIZE
	jae	next_segment_right
	mov	[ebx+PLANE_POINTER.offset],eax
    pointer_movement_done:
	ret
    next_segment_right:
	mov	ecx,eax
	sub	ecx,SEGMENT_SIZE
	xor	eax,eax
	mov	[ebx+PLANE_POINTER.offset],eax
      go_segment_right:
	mov	esi,[ebx+PLANE_POINTER.segment]
	mov	esi,[esi+PLANE_SEGMENT.right_segment]
	test	esi,esi
	jz	new_segments_right
	mov	[ebx+PLANE_POINTER.segment],esi
	jmp	move_pointer_right
    new_segments_right:
	push	ecx
	mov	esi,[ebx+PLANE_POINTER.segment]
	call	find_bottom_segment
	push	ebx
	xor	ebx,ebx
      append_segments_right:
	call	new_segment
	test	ebx,ebx
	jz	right_bottom_connection_ok
	mov	[ebx+PLANE_SEGMENT.top_segment],edx
	mov	[edx+PLANE_SEGMENT.bottom_segment],ebx
      right_bottom_connection_ok:
	mov	[edx+PLANE_SEGMENT.left_segment],esi
	mov	[esi+PLANE_SEGMENT.right_segment],edx
	mov	ebx,edx
	mov	esi,[esi+PLANE_SEGMENT.top_segment]
	test	esi,esi
	jnz	append_segments_right
	pop	ebx
	pop	ecx
	jmp	go_segment_right
      find_bottom_segment:
	mov	eax,[esi+PLANE_SEGMENT.bottom_segment]
	test	eax,eax
	jz	bottom_segment_found
	mov	esi,eax
	jmp	find_bottom_segment
      bottom_segment_found:
	ret
    go_left:
	mov	eax,[ebx+PLANE_POINTER.x]
	sub	eax,ecx
	mov	[ebx+PLANE_POINTER.x],eax
	cmp	[min_x],eax
	jle	move_pointer_left
	mov	[min_x],eax
	bts	[update_flags],0
    move_pointer_left:
	mov	eax,[ebx+PLANE_POINTER.offset]
	sub	eax,ecx
	jc	next_segment_left
	mov	[ebx+PLANE_POINTER.offset],eax
	ret
    next_segment_left:
	mov	ecx,eax
	not	ecx
	mov	eax,SEGMENT_SIZE-1
	mov	[ebx+PLANE_POINTER.offset],eax
      go_segment_left:
	mov	esi,[ebx+PLANE_POINTER.segment]
	mov	esi,[esi+PLANE_SEGMENT.left_segment]
	test	esi,esi
	jz	new_segments_left
	mov	[ebx+PLANE_POINTER.segment],esi
	jmp	move_pointer_left
    new_segments_left:
	push	ecx
	mov	esi,[ebx+PLANE_POINTER.segment]
	call	find_bottom_segment
	push	ebx
	xor	ebx,ebx
      append_segments_left:
	call	new_segment
	test	ebx,ebx
	jz	left_bottom_connection_ok
	mov	[ebx+PLANE_SEGMENT.top_segment],edx
	mov	[edx+PLANE_SEGMENT.bottom_segment],ebx
      left_bottom_connection_ok:
	mov	[edx+PLANE_SEGMENT.right_segment],esi
	mov	[esi+PLANE_SEGMENT.left_segment],edx
	mov	ebx,edx
	mov	esi,[esi+PLANE_SEGMENT.top_segment]
	test	esi,esi
	jnz	append_segments_left
	pop	ebx
	pop	ecx
	jmp	go_segment_left
    go_up:
	mov	eax,[ebx+PLANE_POINTER.y]
	sub	eax,ecx
	mov	[ebx+PLANE_POINTER.y],eax
	cmp	[min_y],eax
	jle	move_pointer_up
	mov	[min_y],eax
	bts	[update_flags],1
    move_pointer_up:
	mov	esi,[ebx+PLANE_POINTER.segment]
      go_segment_up:
	mov	eax,[esi+PLANE_SEGMENT.top_segment]
	test	eax,eax
	jz	new_segments_up
	mov	esi,eax
	loop	go_segment_up
	mov	[ebx+PLANE_POINTER.segment],esi
	ret
    new_segments_up:
	push	esi
	push	ecx
	push	ebx
	call	find_leftmost_segment
	xor	ebx,ebx
      append_segments_up:
	call	new_segment
	test	ebx,ebx
	jz	top_left_connection_ok
	mov	[ebx+PLANE_SEGMENT.right_segment],edx
	mov	[edx+PLANE_SEGMENT.left_segment],ebx
      top_left_connection_ok:
	mov	[edx+PLANE_SEGMENT.bottom_segment],esi
	mov	[esi+PLANE_SEGMENT.top_segment],edx
	mov	ebx,edx
	mov	esi,[esi+PLANE_SEGMENT.right_segment]
	test	esi,esi
	jnz	append_segments_up
	pop	ebx
	pop	ecx
	pop	esi
	mov	esi,[esi+PLANE_SEGMENT.top_segment]
	loop	new_segments_up
	mov	[ebx+PLANE_POINTER.segment],esi
	ret
      find_leftmost_segment:
	mov	eax,[esi+PLANE_SEGMENT.left_segment]
	test	eax,eax
	jz	leftmost_segment_found
	mov	esi,eax
	jmp	find_leftmost_segment
      leftmost_segment_found:
	ret
    go_down:
	mov	eax,[ebx+PLANE_POINTER.y]
	add	eax,ecx
	mov	[ebx+PLANE_POINTER.y],eax
	cmp	[max_y],eax
	jge	move_pointer_down
	mov	[max_y],eax
	bts	[update_flags],1
    move_pointer_down:
	mov	esi,[ebx+PLANE_POINTER.segment]
      go_segment_down:
	mov	eax,[esi+PLANE_SEGMENT.bottom_segment]
	test	eax,eax
	jz	new_segments_down
	mov	esi,eax
	loop	go_segment_down
	mov	[ebx+PLANE_POINTER.segment],esi
	ret
    new_segments_down:
	push	esi
	push	ecx
	push	ebx
	call	find_leftmost_segment
	xor	ebx,ebx
      append_segments_down:
	call	new_segment
	test	ebx,ebx
	jz	bottom_left_connection_ok
	mov	[ebx+PLANE_SEGMENT.right_segment],edx
	mov	[edx+PLANE_SEGMENT.left_segment],ebx
      bottom_left_connection_ok:
	mov	[edx+PLANE_SEGMENT.top_segment],esi
	mov	[esi+PLANE_SEGMENT.bottom_segment],edx
	mov	ebx,edx
	mov	esi,[esi+PLANE_SEGMENT.right_segment]
	test	esi,esi
	jnz	append_segments_down
	pop	ebx
	pop	ecx
	pop	esi
	mov	esi,[esi+PLANE_SEGMENT.bottom_segment]
	loop	new_segments_down
	mov	[ebx+PLANE_POINTER.segment],esi
	ret

scroll_horizontal:
; ecx = number of cells to scroll by
; preserves: ebx, esi, edi
	neg	ecx
	jz	scroll_done
	jns	scroll_left
	neg	ecx
    scroll_right:
	mov	eax,[window_x]
	add	eax,ecx
	add	eax,[window_width]
	sub	eax,2
	sub	eax,[max_x]
	jle	scroll_right_range_ok
	sub	ecx,eax
      scroll_right_range_ok:
	add	[window_x],ecx
      move_window_right:
	add	[window_offset],ecx
	cmp	[window_offset],SEGMENT_SIZE
	jb	scroll_done
	xor	ecx,ecx
	xchg	ecx,[window_offset]
	sub	ecx,SEGMENT_SIZE
	mov	eax,[window_segment]
	mov	eax,[eax+PLANE_SEGMENT.right_segment]
	mov	[window_segment],eax
	jmp	move_window_right
    scroll_left:
	mov	eax,[window_x]
	sub	eax,ecx
	cmp	eax,[min_x]
	jge	scroll_left_range_ok
	mov	ecx,[window_x]
	sub	ecx,[min_x]
      scroll_left_range_ok:
	sub	[window_x],ecx
      move_window_left:
	sub	[window_offset],ecx
	jnc	scroll_done
	mov	ecx,SEGMENT_SIZE
	xchg	ecx,[window_offset]
	neg	ecx
	mov	eax,[window_segment]
	mov	eax,[eax+PLANE_SEGMENT.left_segment]
	mov	[window_segment],eax
	jmp	move_window_left
      scroll_done:
	ret

scroll_vertical:
; ecx = number of cells to scroll by
; preserves: ebx, esi, edi
	neg	ecx
	jz	scroll_done
	jns	scroll_up
	neg	ecx
    scroll_down:
	mov	edx,[window_y]
	push	esi
	mov	esi,[window_segment]
	mov	eax,[max_y]
	add	eax,2
	sub	eax,[window_height]
      move_window_down:
	cmp	edx,eax
	jge	bottom_reached
	inc	edx
	mov	esi,[esi+PLANE_SEGMENT.bottom_segment]
	loop	move_window_down
      bottom_reached:
	mov	[window_segment],esi
	mov	[window_y],edx
	pop	esi
	ret
    scroll_up:
	mov	edx,[window_y]
	push	esi
	mov	esi,[window_segment]
      move_window_up:
	cmp	edx,[min_y]
	jle	top_reached
	dec	edx
	mov	esi,[esi+PLANE_SEGMENT.top_segment]
	loop	move_window_up
      top_reached:
	mov	[window_segment],esi
	mov	[window_y],edx
	pop	esi
	ret

refresh_at_pointers:
	mov	ebx,p_instruction
	call	force_cell_update
	mov	ebx,p_data
	call	force_cell_update
	mov	ebx,p_stack
	call	force_cell_update
	ret
    force_cell_update:
	mov	eax,[ebx+PLANE_POINTER.x]
	sub	eax,[window_x]
	imul	eax,[cell_width]
	mov	[rect.left],eax
	add	eax,[cell_width]
	mov	[rect.right],eax
	mov	eax,[ebx+PLANE_POINTER.y]
	sub	eax,[window_y]
	imul	eax,[cell_height]
	mov	[rect.top],eax
	add	eax,[cell_height]
	mov	[rect.bottom],eax
	invoke	InvalidateRect,[hwnd_plane],rect,FALSE
	ret

proc PlaneWindowProc uses ebx esi edi, hwnd,wmsg,wparam,lparam
  locals
    sc SCROLLINFO
    size SIZE
    ps PAINTSTRUCT
    char TCHAR
    pt1 POINT
    pt2 POINT
    pt3 POINT
  endl
	mov	eax,[wmsg]
	cmp	eax,WM_DESTROY
	je	wmdestroy
	cmp	eax,WM_CREATE
	je	wmcreate
	cmp	eax,WM_SIZE
	je	wmsize
	cmp	eax,WM_HSCROLL
	je	wmhscroll
	cmp	eax,WM_VSCROLL
	je	wmvscroll
	cmp	eax,WM_PAINT
	je	wmpaint
	cmp	eax,WM_LBUTTONDOWN
	je	wmlbuttondown
  defwndproc:
	invoke	DefWindowProc,[hwnd],[wmsg],[wparam],[lparam]
	jmp	finish
  wmcreate:
	invoke	CreateFont,20,16,0,0,0,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_RASTER_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FIXED_PITCH+FF_DONTCARE,'Courier New'
	or	eax,eax
	jz	failed
	mov	[hfont],eax
	invoke	GetDC,[hwnd]
	mov	ebx,eax
	invoke	SelectObject,ebx,[hfont]
	mov	[char],20h
	lea	eax,[size]
	invoke	GetTextExtentPoint32,ebx,addr char,1,addr size
	mov	eax,[size.cy]
	cmp	eax,20
	jae	cell_height_ok
	mov	eax,20
    cell_height_ok:
	mov	[cell_height],eax
	mov	eax,[size.cx]
	cmp	eax,20
	jae	cell_width_ok
	mov	eax,20
    cell_width_ok:
	mov	[cell_width],eax
	invoke	ReleaseDC,[hwnd],ebx
	invoke	CreateSolidBrush,00EE00h
	mov	[hbrush_instruction],eax
	invoke	CreateSolidBrush,0000EEh
	mov	[hbrush_data],eax
	invoke	CreateSolidBrush,00EEEEh
	mov	[hbrush_stack],eax
	xor	eax,eax
	mov	[window_buffer],eax
	xor	eax,eax
	jmp	finish
  wmsize:
	invoke	GetClientRect,[hwnd],rect
	mov	eax,[rect.right]
	sub	eax,[rect.left]
	cdq
	div	[cell_width]
	add	edx,-1
	adc	eax,0
	mov	[window_width],eax
	mov	eax,[rect.bottom]
	sub	eax,[rect.top]
	cdq
	div	[cell_height]
	add	edx,-1
	adc	eax,0
	mov	[window_height],eax
	mov	eax,[window_width]
	mul	[window_height]
	shl	eax,1
	jz	buffer_allocated
	mov	ebx,eax
	mov	eax,[window_buffer]
	test	eax,eax
	jz	buffer_released
	invoke	VirtualFree,eax,0,MEM_RELEASE
    buffer_released:
	invoke	VirtualAlloc,0,ebx,MEM_COMMIT,PAGE_READWRITE
	test	eax,eax
	jz	out_of_memory
    buffer_allocated:
	mov	[window_buffer],eax
	invoke	InvalidateRect,[hwnd],rect,FALSE
    update_scrollbars:
	mov	eax,[max_x]
	sub	eax,[min_x]
	add	eax,2
	cmp	eax,[window_width]
	jg	horizontal_position_ok
	mov	ecx,[window_x]
	sub	ecx,[min_x]
	neg	ecx
	jz	horizontal_position_ok
	call	scroll_horizontal
	invoke	GetClientRect,[hwnd],rect
	invoke	InvalidateRect,[hwnd],rect,FALSE
    horizontal_position_ok:
	mov	eax,[max_y]
	sub	eax,[min_y]
	add	eax,2
	cmp	eax,[window_height]
	jg	vertical_position_ok
	mov	ecx,[window_y]
	sub	ecx,[min_y]
	neg	ecx
	jz	vertical_position_ok
	call	scroll_vertical
	invoke	GetClientRect,[hwnd],rect
	invoke	InvalidateRect,[hwnd],rect,FALSE
    vertical_position_ok:
	mov	[sc.cbSize],sizeof.SCROLLINFO
	mov	[sc.fMask],SIF_DISABLENOSCROLL+SIF_RANGE+SIF_PAGE+SIF_POS
	mov	eax,[min_y]
	mov	[sc.nMin],eax
	mov	eax,[max_y]
	inc	eax
	mov	[sc.nMax],eax
	mov	eax,[window_height]
	mov	[sc.nPage],eax
	mov	eax,[window_y]
	mov	[sc.nPos],eax
	invoke	SetScrollInfo,[hwnd],SB_VERT,addr sc,TRUE
	mov	eax,[min_x]
	mov	[sc.nMin],eax
	mov	eax,[max_x]
	inc	eax
	mov	[sc.nMax],eax
	mov	eax,[window_width]
	mov	[sc.nPage],eax
	mov	eax,[window_x]
	mov	[sc.nPos],eax
	invoke	SetScrollInfo,[hwnd],SB_HORZ,addr sc,TRUE
	jmp	finish
  wmhscroll:
	movzx	eax,word [wparam]
	cmp	eax,SB_LINEUP
	je	hscroll_left
	cmp	eax,SB_LINEDOWN
	je	hscroll_right
	cmp	eax,SB_PAGEUP
	je	hscroll_wleft
	cmp	eax,SB_PAGEDOWN
	je	hscroll_wright
	cmp	eax,SB_THUMBTRACK
	je	hscroll_pos
	jmp	update_scrollbars
    hscroll_left:
	mov	ecx,-1
	call	scroll_horizontal
	jmp	scrolling_done
    hscroll_right:
	mov	ecx,1
	call	scroll_horizontal
	jmp	scrolling_done
    hscroll_wleft:
	mov	ecx,[window_width]
	neg	ecx
	call	scroll_horizontal
	jmp	scrolling_done
    hscroll_wright:
	mov	ecx,[window_width]
	call	scroll_horizontal
	jmp	scrolling_done
    hscroll_pos:
	mov	[sc.cbSize],sizeof.SCROLLINFO
	mov	[sc.fMask],SIF_ALL
	invoke	GetScrollInfo,[hwnd],SB_HORZ,addr sc
	mov	ecx,[sc.nTrackPos]
	sub	ecx,[window_x]
	call	scroll_horizontal
	jmp	scrolling_done
  wmvscroll:
	movzx	eax,word [wparam]
	cmp	eax,SB_LINEUP
	je	vscroll_left
	cmp	eax,SB_LINEDOWN
	je	vscroll_right
	cmp	eax,SB_PAGEUP
	je	vscroll_wleft
	cmp	eax,SB_PAGEDOWN
	je	vscroll_wright
	cmp	eax,SB_THUMBTRACK
	je	vscroll_pos
	jmp	update_scrollbars
    vscroll_left:
	mov	ecx,-1
	call	scroll_vertical
	jmp	scrolling_done
    vscroll_right:
	mov	ecx,1
	call	scroll_vertical
	jmp	scrolling_done
    vscroll_wleft:
	mov	ecx,[window_height]
	neg	ecx
	call	scroll_vertical
	jmp	scrolling_done
    vscroll_wright:
	mov	ecx,[window_height]
	call	scroll_vertical
	jmp	scrolling_done
    vscroll_pos:
	mov	[sc.cbSize],sizeof.SCROLLINFO
	mov	[sc.fMask],SIF_ALL
	invoke	GetScrollInfo,[hwnd],SB_VERT,addr sc
	mov	ecx,[sc.nTrackPos]
	sub	ecx,[window_y]
	call	scroll_vertical
    scrolling_done:
	invoke	GetClientRect,[hwnd],rect
	invoke	InvalidateRect,[hwnd],rect,FALSE
	jmp	update_scrollbars
  wmpaint:
	mov	edi,[window_buffer]
	mov	esi,[window_segment]
	mov	ecx,[window_height]
      copy_row:
	push	esi
	push	ecx
	mov	edx,[window_width]
	mov	eax,[window_offset]
      copy_from_segment:
	lea	esi,[esi+PLANE_SEGMENT.content+eax*4]
	mov	ecx,SEGMENT_SIZE
	sub	ecx,eax
	cmp	ecx,edx
	jbe	amount_to_copy_ok
	mov	ecx,edx
      amount_to_copy_ok:
	sub	edx,ecx
      copy_characters:
	lodsd
	test	eax,eax
	jz	blank_character
	test	eax,0FFFF0000h
	jz	store_character
      blank_character:
	mov	ax,20h
      store_character:
	stosw
	loop	copy_characters
	test	edx,edx
	jz	copy_next_row
	sub	esi,sizeof.PLANE_SEGMENT
	xor	eax,eax
	mov	esi,[esi+PLANE_SEGMENT.right_segment]
	test	esi,esi
	jnz	copy_from_segment
	mov	ecx,edx
	mov	ax,20h
	rep	stosw
      copy_next_row:
	pop	ecx
	pop	esi
	dec	ecx
	jz	window_ready
	mov	esi,[esi+PLANE_SEGMENT.bottom_segment]
	test	esi,esi
	jnz	copy_row
	imul	ecx,[window_width]
	mov	al,20h
	rep	stosw
      window_ready:
	invoke	CreateRectRgn,0,0,0,0
	mov	edi,eax
	invoke	GetUpdateRgn,[hwnd],edi,FALSE
	invoke	BeginPaint,[hwnd],addr ps
	mov	ebx,eax
	invoke	SelectObject,ebx,[hfont]
	mov	esi,[window_buffer]
	mov	[rect.top],0
	mov	eax,[cell_width]
	mov	[rect.right],eax
	mov	eax,[cell_height]
	mov	[rect.bottom],eax
	mov	ecx,[window_height]
    draw_window:
	push	ecx
	mov	[rect.left],0
	mov	eax,[cell_width]
	imul	eax,[window_width]
	mov	[rect.right],eax
	invoke	RectInRegion,edi,rect
	test	eax,eax
	jz	skip_row
	mov	eax,[cell_width]
	mov	[rect.right],eax
	mov	ecx,[window_width]
    draw_row:
	push	ecx
	invoke	RectInRegion,edi,rect
	test	eax,eax
	jz	draw_next_character
	invoke	FillRect,ebx,rect,[plane_wc.hbrBackground]
	invoke	TextOut,ebx,[rect.left],[rect.top],esi,1
    draw_next_character:
	add	esi,2
	mov	eax,[cell_width]
	add	[rect.left],eax
	add	[rect.right],eax
	pop	ecx
	loop	draw_row
	jmp	draw_next_row
    skip_row:
	mov	eax,[window_width]
	lea	esi,[esi+eax*2]
    draw_next_row:
	mov	eax,[cell_height]
	add	[rect.top],eax
	add	[rect.bottom],eax
	pop	ecx
	dec	ecx
	jnz	draw_window
	invoke	DeleteObject,edi
	invoke	SelectObject,ebx,[hbrush_data]
	mov	[pt1.x],1
	mov	[pt3.x],1
	mov	[pt2.x],5
	mov	eax,[cell_height]
	shr	eax,1
	mov	[pt3.y],eax
	sub	eax,4
	mov	[pt2.y],eax
	sub	eax,4
	mov	[pt1.y],eax
	mov	esi,p_data
	call	draw_pointer_arrow
	invoke	SelectObject,ebx,[hbrush_stack]
	mov	[pt1.x],1
	mov	[pt3.x],1
	mov	[pt2.x],5
	mov	eax,[cell_height]
	shr	eax,1
	mov	[pt1.y],eax
	add	eax,4
	mov	[pt2.y],eax
	add	eax,4
	mov	[pt3.y],eax
	mov	esi,p_stack
	call	draw_pointer_arrow
	invoke	SelectObject,ebx,[hbrush_instruction]
	mov	[pt1.x],1
	mov	[pt3.x],1
	mov	[pt2.x],5
	mov	eax,[cell_height]
	shr	eax,1
	mov	[pt2.y],eax
	sub	eax,4
	mov	[pt1.y],eax
	add	eax,8
	mov	[pt3.y],eax
	mov	esi,p_instruction
	call	draw_pointer_arrow
	invoke	EndPaint,[hwnd],addr ps
	invoke	ReleaseDC,[hwnd],ebx
	jmp	finish
    draw_pointer_arrow:
	mov	eax,[esi+PLANE_POINTER.direction]
	call	rotate_arrow
	mov	eax,[esi+PLANE_POINTER.x]
	sub	eax,[window_x]
	imul	eax,[cell_width]
	add	[pt1.x],eax
	add	[pt2.x],eax
	add	[pt3.x],eax
	mov	eax,[esi+PLANE_POINTER.y]
	sub	eax,[window_y]
	imul	eax,[cell_height]
	add	[pt1.y],eax
	add	[pt2.y],eax
	add	[pt3.y],eax
	invoke	Polygon,ebx,addr pt1,3
	retn
    rotate_arrow:
	lea	edi,[pt1]
	call	rotate_point
	lea	edi,[pt2]
	call	rotate_point
	lea	edi,[pt3]
	call	rotate_point
	retn
    rotate_point:
	cmp	al,2
	je	point_ok
	ja	clockwise
	test	al,al
	jz	flip
	mov	edx,[edi+POINT.y]
	xchg	edx,[edi+POINT.x]
	neg	edx
	add	edx,[cell_height]
	mov	[edi+POINT.y],edx
	jmp	point_ok
      clockwise:
	mov	edx,[edi+POINT.x]
	xchg	edx,[edi+POINT.y]
	neg	edx
	add	edx,[cell_width]
	mov	[edi+POINT.x],edx
	jmp	point_ok
      flip:
	neg	[edi+POINT.x]
	neg	[edi+POINT.y]
	mov	edx,[cell_width]
	add	[edi+POINT.x],edx
	mov	edx,[cell_height]
	add	[edi+POINT.y],edx
      point_ok:
	retn
  wmlbuttondown:
	mov	eax,[window_segment]
	mov	[p_loader.segment],eax
	mov	eax,[window_offset]
	mov	[p_loader.offset],eax
	mov	eax,[window_x]
	mov	[p_loader.x],eax
	mov	eax,[window_y]
	mov	[p_loader.y],eax
	mov	[p_loader.direction],2
	mov	ebx,p_loader
	movsx	eax,word [lparam]
	cdq
	idiv	[cell_width]
	mov	ecx,eax
	call	move_pointer
	mov	[p_loader.direction],3
	movsx	eax,word [lparam+2]
	cdq
	idiv	[cell_height]
	mov	ecx,eax
	call	move_pointer
	invoke	IsDlgButtonChecked,[hwnd_controller],ID_POKE
	cmp	eax,BST_CHECKED
	je	poke
	mov	esi,[p_loader.segment]
	mov	ecx,[p_loader.offset]
	mov	eax,[esi+PLANE_SEGMENT.content+ecx*4]
	invoke	SetDlgItemInt,[hwnd_controller],ID_VALUE,eax,TRUE
	jmp	finish
     poke:
	invoke	GetDlgItemInt,[hwnd_controller],ID_VALUE,bytes_read,TRUE
	mov	esi,[p_loader.segment]
	mov	ecx,[p_loader.offset]
	mov	[esi+PLANE_SEGMENT.content+ecx*4],eax
	mov	eax,[p_loader.x]
	sub	eax,[window_x]
	imul	eax,[cell_width]
	mov	[rect.left],eax
	add	eax,[cell_width]
	mov	[rect.right],eax
	mov	eax,[p_loader.y]
	sub	eax,[window_y]
	imul	eax,[cell_height]
	mov	[rect.top],eax
	add	eax,[cell_height]
	mov	[rect.bottom],eax
	invoke	InvalidateRect,[hwnd],rect,FALSE
	jmp	finish
  failed:
	or	eax,-1
	jmp	finish
  wmdestroy:
	invoke	VirtualFree,[window_buffer],0,MEM_RELEASE
	invoke	DeleteObject,[hfont]
	invoke	DeleteObject,[hbrush_instruction]
	invoke	DeleteObject,[hbrush_data]
	invoke	DeleteObject,[hbrush_stack]
	invoke	PostQuitMessage,0
	xor	eax,eax
  finish:
	ret
endp

proc ControllerDialogProc uses ebx esi edi, hwnd,msg,wparam,lparam
  locals
    ofn OPENFILENAME
  endl
	mov	eax,[msg]
	cmp	eax,WM_INITDIALOG
	je	wminitdialog
	cmp	eax,WM_COMMAND
	je	wmcommand
	cmp	eax,WM_CLOSE
	je	wmclose
	xor	eax,eax
	jmp	return
  wminitdialog:
	invoke	SetDlgItemText,[hwnd],ID_STATUS,_waiting
	invoke	SetDlgItemInt,[hwnd],ID_ACCUMULATOR,[accumulator],TRUE
	invoke	SetDlgItemInt,[hwnd],ID_STEPS,[steps],TRUE
	invoke	CheckDlgButton,[hwnd],ID_8HZ,BST_CHECKED
	invoke	CheckDlgButton,[hwnd],ID_PEEK,BST_CHECKED
	btr	[update_flags],8
	jmp	@8hz
  wmcommand:
	mov	eax,[wparam]
	cmp	eax,BN_CLICKED shl 16 + IDCANCEL
	je	wmclose
	cmp	eax,BN_CLICKED shl 16 + ID_RESET
	je	reset
	cmp	eax,BN_CLICKED shl 16 + ID_LOAD
	je	?load
	cmp	eax,BN_CLICKED shl 16 + ID_STEP
	je	step
	cmp	eax,BN_CLICKED shl 16 + ID_RUN
	je	run
	cmp	eax,EN_CHANGE shl 16 + ID_VALUE
	je	value_change
	cmp	eax,EN_CHANGE shl 16 + ID_CHARACTER
	je	character_change
	shr	eax,16
	cmp	eax,BN_CLICKED
	jne	processed
	invoke	IsDlgButtonChecked,[hwnd],ID_8HZ
	cmp	eax,BST_CHECKED
	je	@8hz
	invoke	IsDlgButtonChecked,[hwnd],ID_50HZ
	cmp	eax,BST_CHECKED
	je	@50hz
	mov	[delay],0
	jmp	processed
     @8hz:
	mov	[delay],1000/8
	jmp	processed
     @50hz:
	mov	[delay],1000/50
	jmp	processed
     value_change:
	invoke	GetDlgItemInt,[hwnd],ID_VALUE,bytes_read,TRUE
	mov	edi,string_buffer
	test	eax,0FFFF0000h
	jnz	char_ok
	stosw
     char_ok:
	xor	ax,ax
	stosw
	bts	[update_flags],8
	invoke	SetDlgItemText,[hwnd],ID_CHARACTER,string_buffer
	btr	[update_flags],8
	jmp	processed
     character_change:
	bt	[update_flags],8
	jc	processed
	invoke	GetDlgItem,[hwnd],ID_CHARACTER
	mov	ebx,eax
	invoke	SendMessage,ebx,WM_GETTEXT,100h,string_buffer
	invoke	SendMessage,ebx,EM_GETSEL,NULL,NULL
	and	eax,0FFFFh
	jz	pos_ok
	dec	eax
      pos_ok:
	movzx	eax,[string_buffer+eax*2]
	invoke	SetDlgItemInt,[hwnd],ID_VALUE,eax,TRUE
	jmp	processed
     run:
	cmp	[status],CHL_RUNNING
	je	stop
	cmp	[status],CHL_IDLE
	jne	processed
	mov	[status],CHL_RUNNING
	invoke	SetDlgItemText,[hwnd],ID_STATUS,_running
	invoke	SetDlgItemText,[hwnd],ID_RUN,_stop
	invoke	GetDlgItem,[hwnd],ID_STEP
	invoke	EnableWindow,eax,FALSE
	jmp	processed
    stop:
	mov	[status],CHL_IDLE
	invoke	SetDlgItemText,[hwnd],ID_STATUS,_waiting
	invoke	SetDlgItemText,[hwnd],ID_RUN,_run
	invoke	GetDlgItem,[hwnd],ID_STEP
	invoke	EnableWindow,eax,TRUE
	jmp	processed
    step:
	cmp	[status],CHL_IDLE
	jne	processed
	mov	[status],CHL_STEPPING
	jmp	processed
    reset:
	call	free_memory
	call	initialize_machine
	jmp	machine_reinitialized
    ?load:
	mov	[ofn.lStructSize],sizeof.OPENFILENAME
	mov	eax,[hwnd]
	mov	[ofn.hwndOwner],eax
	mov	eax,[plane_wc.hInstance]
	mov	[ofn.hInstance],eax
	mov	[ofn.lpstrCustomFilter],NULL
	mov	[ofn.nFilterIndex],1
	mov	[ofn.nMaxFile],1000h
	mov	[ofn.lpstrFileTitle],NULL
	mov	[ofn.nMaxFileTitle],0
	mov	[ofn.lpstrInitialDir],NULL
	mov	[ofn.lpstrDefExt],NULL
	mov	[ofn.lpstrFile],path_buffer
	mov	[ofn.lpstrFilter],chl_filter
	mov	[ofn.Flags],OFN_EXPLORER+OFN_FILEMUSTEXIST
	mov	[ofn.lpstrTitle],NULL
	invoke	GetOpenFileName,addr ofn
	test	eax,eax
	jz	processed
	call	free_memory
	call	initialize_machine
	invoke	CreateFile,path_buffer,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0
	cmp	eax,-1
	je	program_initialized
	mov	ebx,eax
	invoke	GetFileSize,ebx,NULL
	mov	esi,eax
	lea	eax,[eax*3]
	invoke	VirtualAlloc,0,eax,MEM_COMMIT,PAGE_READWRITE
	test	eax,eax
	jz	out_of_memory
	mov	edi,eax
	invoke	ReadFile,ebx,edi,esi,bytes_read,0
	invoke	CloseHandle,ebx
	mov	ecx,esi
	add	esi,edi
	lea	eax,[ecx*2]
	invoke	MultiByteToWideChar,CP_UTF8,0,edi,ecx,esi,eax
	test	eax,eax
	jnz	file_converted
	mov	ecx,esi
	sub	ecx,edi
	lea	eax,[ecx*2]
	invoke	MultiByteToWideChar,CP_ACP,0,edi,ecx,esi,eax
      file_converted:
	push	MEM_RELEASE
	push	0
	push	edi	; for VirtualFree
	mov	ebx,p_loader
      initialize_program:
	lodsw
	test	ax,ax
	jz	program_initialized
	cmp	ax,13
	je	cr
	cmp	ax,10
	je	lf
	mov	edi,[ebx+PLANE_POINTER.segment]
	mov	edx,[ebx+PLANE_POINTER.offset]
	lea	edi,[edi+PLANE_SEGMENT.content+edx*4]
	stosw
	push	esi
	mov	ecx,1
	call	move_pointer
	pop	esi
	jmp	initialize_program
      cr:
	cmp	byte [esi],10
	jne	next_program_row
	lodsw
	jmp	next_program_row
      lf:
	cmp	byte [esi],13
	jne	next_program_row
	lodsw
      next_program_row:
	push	esi
	mov	[ebx+PLANE_POINTER.direction],3
	mov	ecx,1
	call	move_pointer
	mov	[ebx+PLANE_POINTER.direction],2
	mov	ecx,[ebx+PLANE_POINTER.x]
	neg	ecx
	call	move_pointer
	pop	esi
	jmp	initialize_program
      program_initialized:
	invoke	VirtualFree
      machine_reinitialized:
	mov	[status],CHL_IDLE
	invoke	GetClientRect,[hwnd_plane],rect
	invoke	InvalidateRect,[hwnd_plane],rect,FALSE
	invoke	PostMessage,[hwnd_plane],WM_HSCROLL,SB_ENDSCROLL,0
	invoke	PostMessage,[hwnd_plane],WM_VSCROLL,SB_ENDSCROLL,0
	invoke	SetDlgItemText,[hwnd],ID_STATUS,_waiting
	invoke	SetDlgItemInt,[hwnd],ID_ACCUMULATOR,[accumulator],TRUE
	invoke	SetDlgItemInt,[hwnd],ID_STEPS,[steps],TRUE
	invoke	SetDlgItemText,[hwnd],ID_RUN,_run
	invoke	GetDlgItem,[hwnd],ID_RUN
	invoke	EnableWindow,eax,TRUE
	invoke	GetDlgItem,[hwnd],ID_STEP
	invoke	EnableWindow,eax,TRUE
	jmp	processed

  wmclose:
	invoke	EndDialog,[hwnd],0
	invoke	DestroyWindow,[hwnd_plane]
  processed:
	mov	eax,1
  return:
	ret
endp

section '.data' data readable writeable

  _plane_class TCHAR 'CHALLENGER',0
  _plane_title TCHAR 'Challenger ',VERSION_STRING,0

  _startup_failed TCHAR 'Startup failed.',0
  _out_of_memory TCHAR 'Out of memory!',0

  _running TCHAR 'Running.',0
  _waiting TCHAR 'Waiting.',0
  _exception_0 TCHAR 'Exception 0: division by zero.',0
  _exception_1 TCHAR 'Exception 1: trap.',0
  _exception_6 TCHAR 'Exception 6: unrecognized instruction.',0

  _run TCHAR '&Run',0
  _stop TCHAR 'S&top',0

  plane_wc WNDCLASS 0,PlaneWindowProc,0,0,NULL,NULL,NULL,0,NULL,_plane_class

  chl_filter TCHAR 'Challenger files',0,'*.CHL',0,\
		   'All files',0,'*.*',0,0

  msg MSG
  rect RECT

  hwnd_plane dd ?
  hwnd_controller dd ?
  hfont dd ?
  cell_width dd ?
  cell_height dd ?
  hbrush_instruction dd ?
  hbrush_data dd ?
  hbrush_stack dd ?
  clock dd ?
  delay dd ?
  status dd ?
  bytes_read dd ?

  memory_bottom dd ?
  memory_top dd ?
  memory_cursor dd ?
  update_flags dd ?

  window_buffer dd ?
  window_width dd ?
  window_height dd ?

  window_segment dd ?
  window_offset dd ?
  window_x dd ?
  window_y dd ?

  min_x dd ?
  min_y dd ?
  max_x dd ?
  max_y dd ?

  p_instruction PLANE_POINTER
  p_data PLANE_POINTER
  p_stack PLANE_POINTER
  p_loader PLANE_POINTER

  accumulator dd ?
  steps dd ?

  string_buffer TCHAR 100h dup ?
  path_buffer TCHAR 1000h dup ?

section '.idata' import data readable writeable

  library kernel32,'KERNEL32.DLL',\
	  user32,'USER32.DLL',\
	  gdi32,'GDI32.DLL',\
	  comdlg32,'COMDLG32.DLL'

  include 'api\kernel32.inc'
  include 'api\user32.inc'
  include 'api\gdi32.inc'
  include 'api\comdlg32.inc'

section '.rsrc' resource data readable

  ID_CONTROLLER = 1

  ID_STATUS	 = 100
  ID_ACCUMULATOR = 101
  ID_STEPS	 = 102
  ID_8HZ	 = 110
  ID_50HZ	 = 111
  ID_MAXIMUM	 = 112
  ID_RUN	 = 120
  ID_STEP	 = 121
  ID_LOAD	 = 122
  ID_RESET	 = 123
  ID_PEEK	 = 130
  ID_POKE	 = 131
  ID_VALUE	 = 132
  ID_CHARACTER	 = 133

  directory RT_DIALOG,dialogs

  resource dialogs,\
	   ID_CONTROLLER,LANG_ENGLISH+SUBLANG_DEFAULT,controller

  dialog controller,'Challenger controller',280,20,180,162,WS_VISIBLE+WS_CAPTION+WS_POPUP+WS_SYSMENU+DS_MODALFRAME

    dialogitem 'BUTTON','Status',-1,8,8,164,26,WS_VISIBLE+BS_GROUPBOX+WS_GROUP
    dialogitem 'STATIC','',ID_STATUS,16,19,140,8,WS_VISIBLE

    dialogitem 'BUTTON','Execution speed',-1,8,38,76,56,WS_VISIBLE+BS_GROUPBOX+WS_GROUP
    dialogitem 'BUTTON','8 Hz',ID_8HZ,16,50,60,13,WS_VISIBLE+BS_AUTORADIOBUTTON
    dialogitem 'BUTTON','50 Hz',ID_50HZ,16,64,60,13,WS_VISIBLE+BS_AUTORADIOBUTTON
    dialogitem 'BUTTON','Maximum',ID_MAXIMUM,16,78,60,13,WS_VISIBLE+BS_AUTORADIOBUTTON

    dialogitem 'BUTTON','Accumulator',-1,96,38,76,26,WS_VISIBLE+BS_GROUPBOX+WS_GROUP
    dialogitem 'STATIC','',ID_ACCUMULATOR,104,50,64,8,WS_VISIBLE

    dialogitem 'BUTTON','Steps executed',-1,96,68,76,26,WS_VISIBLE+BS_GROUPBOX+WS_GROUP
    dialogitem 'STATIC','',ID_STEPS,104,80,64,8,WS_VISIBLE

    dialogitem 'BUTTON','&Run',ID_RUN,8,100,38,15,WS_VISIBLE+WS_TABSTOP+BS_DEFPUSHBUTTON
    dialogitem 'BUTTON','&Step',ID_STEP,50,100,38,15,WS_VISIBLE+WS_TABSTOP
    dialogitem 'BUTTON','Rese&t',ID_RESET,92,100,38,15,WS_VISIBLE+WS_TABSTOP
    dialogitem 'BUTTON','&Load...',ID_LOAD,134,100,38,15,WS_VISIBLE+WS_TABSTOP

    dialogitem 'BUTTON','Manipulation',-1,8,120,164,32,WS_VISIBLE+BS_GROUPBOX+WS_GROUP
    dialogitem 'BUTTON','&Peek',ID_PEEK,16,132,40,13,WS_VISIBLE+BS_AUTORADIOBUTTON
    dialogitem 'BUTTON','P&oke',ID_POKE,58,132,40,13,WS_VISIBLE+BS_AUTORADIOBUTTON
    dialogitem 'EDIT','',ID_VALUE,100,132,46,12,WS_VISIBLE+WS_BORDER+WS_TABSTOP
    dialogitem 'EDIT','',ID_CHARACTER,152,132,12,12,WS_VISIBLE+WS_BORDER+ES_AUTOHSCROLL+WS_TABSTOP

  enddialog
