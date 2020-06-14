
match ,{

	include 'macro/struct.inc'

} match -,{
else

	include 'selfhost.inc'

end match
_ equ }

include '../version.inc'

BUFFER_SIZE = 4000h
STACK_SIZE = 4000h

	format	MZ
	heap	0
	stack	stack_segment:stack_top-stack_bottom
	entry	loader:startup

segment loader use16

  startup:

	mov	ax,1687h
	int	2Fh
	or	ax,ax			; DPMI installed?
	jnz	short no_dpmi
	test	bl,1			; 32-bit programs supported?
	jz	short no_dpmi
	mov	word [cs:mode_switch],di
	mov	word [cs:mode_switch+2],es
	mov	bx,si			; allocate memory for DPMI data
	mov	ah,48h
	int	21h
	jnc	switch_to_protected_mode
  init_failed:
	call	startup_error
	db	'DPMI initialization failed.',0Dh,0Ah,0
  no_dpmi:
	call	startup_error
	db	'32-bit DPMI services are not available.',0Dh,0Ah,0
  startup_error:
	pop	si
	push	cs
	pop	ds
      show_message:
	lodsb
	test	al,al
	jz	message_shown
	mov	dl,al
	mov	ah,2
	int	21h
	jmp	show_message
      message_shown:
	mov	ax,4CFFh
	int	21h
  switch_to_protected_mode:
	mov	es,ax
	mov	ds,[ds:2Ch]
	mov	ax,1
	call	far [cs:mode_switch]	; switch to protected mode
	jc	init_failed
	mov	cx,1
	xor	ax,ax
	int	31h			; allocate descriptor for code
	jc	init_failed
	mov	si,ax
	xor	ax,ax
	int	31h			; allocate descriptor for data
	jc	init_failed
	mov	di,ax
	mov	dx,cs
	lar	cx,dx
	shr	cx,8
	or	cx,0C000h
	mov	bx,si
	mov	ax,9
	int	31h			; set code descriptor access rights
	jc	init_failed
	mov	dx,ds
	lar	cx,dx
	shr	cx,8
	or	cx,0C000h
	mov	bx,di
	int	31h			; set data descriptor access rights
	jc	init_failed
	mov	ecx,main
	shl	ecx,4
	mov	dx,cx
	shr	ecx,16
	mov	ax,7
	int	31h			; set data descriptor base address
	jc	init_failed
	mov	bx,si
	int	31h			; set code descriptor base address
	jc	init_failed
	mov	cx,0FFFFh
	mov	dx,0FFFFh
	mov	ax,8			; set segment limit to 4 GB
	int	31h
	jc	init_failed
	mov	bx,di
	int	31h
	jc	init_failed
	mov	ax,ds
	mov	ds,di
	mov	[main_selector],di
	mov	[psp_selector],es
	mov	gs,ax			; environment selector in GS
	cli
	mov	ss,di
	mov	esp,stack_top
	sti
	mov	es,di
	mov	cx,1
	xor	ax,ax
	int	31h			; allocate descriptor for BIOS data segment
	jc	init_failed
	mov	bx,ax
	mov	ax,gs
	lar	cx,ax
	shr	cx,8
	mov	ax,9
	int	31h			; set descriptor access rights
	jc	init_failed
	xor	cx,cx
	mov	dx,400h
	mov	ax,7
	int	31h			; set base address of BIOS data segment
	jc	init_failed
	xor	cx,cx
	mov	dx,0FFh
	mov	ax,8
	int	31h			; set limit of BIOS data segment
	jc	init_failed
	mov	fs,bx			; BIOS data selector in FS
	push	si
	push	start
	retf

  mode_switch dd ?

segment main use32

  start:

	call	system_init

	call	get_arguments
	mov	bl,al
	cmp	[no_logo],0
	jne	logo_ok
	mov	esi,_logo
	xor	ecx,ecx
	call	display_string
      logo_ok:
	test	bl,bl
	jnz	display_usage_information

	xor	al,al
	mov	ecx,[verbosity_level]
	jecxz	init
	or	al,TRACE_ERROR_STACK
	dec	ecx
	jz	init
	or	al,TRACE_DISPLAY
  init:

	call	assembly_init

	mov	eax,[fs:6Ch]
	mov	[timer],eax

  assemble:
	mov	esi,[initial_commands]
	mov	edx,[source_path]
	call	assembly_pass
	jc	assembly_done

	mov	eax,[current_pass]
	cmp	eax,[maximum_number_of_passes]
	jb	assemble

	call	show_display_data

	mov	esi,_error_prefix
	xor	ecx,ecx
	call	display_error_string
	mov	esi,_code_cannot_be_generated
	xor	ecx,ecx
	call	display_error_string
	mov	esi,_message_suffix
	xor	ecx,ecx
	call	display_error_string

	jmp	assembly_failed

  assembly_done:

	call	show_display_data

	cmp	[first_error],0
	jne	assembly_failed

	cmp	[no_logo],0
	jne	summary_done
	mov	eax,[current_pass]
	xor	edx,edx
	call	itoa
	call	display_string
	mov	esi,_passes
	cmp	[current_pass],1
	jne	display_passes_suffix
	mov	esi,_pass
      display_passes_suffix:
	xor	ecx,ecx
	call	display_string
	mov	eax,[fs:6Ch]
	sub	eax,[timer]
	mov	ecx,36000
	mul	ecx
	shrd	eax,edx,16
	shr	edx,16
	mov	ecx,10
	div	ecx
	mov	[timer],edx
	or	edx,eax
	jz	display_output_length
	xor	edx,edx
	call	itoa
	call	display_string
	mov	esi,_message_suffix
	mov	ecx,1
	call	display_string
	mov	eax,[timer]
	xor	edx,edx
	call	itoa
	call	display_string
	mov	esi,_seconds
	xor	ecx,ecx
	call	display_string
      display_output_length:
	call	get_output_length
	push	eax edx
	call	itoa
	call	display_string
	pop	edx eax
	mov	esi,_bytes
	cmp	eax,1
	jne	display_bytes_suffix
	test	edx,edx
	jnz	display_bytes_suffix
	mov	esi,_byte
      display_bytes_suffix:
	xor	ecx,ecx
	call	display_string
	mov	esi,_new_line
	xor	ecx,ecx
	call	display_string
      summary_done:

	mov	ebx,[source_path]
	mov	edi,[output_path]
	call	write_output_file
	jc	write_failed

	call	assembly_shutdown
	call	system_shutdown

	mov	ax,4C00h
	int	21h

  assembly_failed:

	call	show_errors

	call	assembly_shutdown
	call	system_shutdown

	mov	ax,4C02h
	int	21h

  write_failed:
	mov	ebx,_write_failed
	jmp	fatal_error

  out_of_memory:
	mov	ebx,_out_of_memory
	jmp	fatal_error

  fatal_error:

	mov	esi,_error_prefix
	xor	ecx,ecx
	call	display_error_string
	mov	esi,ebx
	xor	ecx,ecx
	call	display_error_string
	mov	esi,_message_suffix
	xor	ecx,ecx
	call	display_error_string

	call	assembly_shutdown
	call	system_shutdown

	mov	ax,4C03h
	int	21h

  display_usage_information:

	mov	esi,_usage
	xor	ecx,ecx
	call	display_string

	call	system_shutdown

	mov	ax,4C01h
	int	21h

  get_arguments:
	push	ds
	mov	ds,[psp_selector]
	mov	esi,81h
	mov	edi,command_line
	mov	ecx,7Fh
     move_command_line:
	lodsb
	cmp	al,0Dh
	je	command_line_moved
	stosb
	loop	move_command_line
     command_line_moved:
	pop	ds
	xor	eax,eax
	stosb
	mov	[initial_commands],eax
	mov	[source_path],eax
	mov	[output_path],eax
	mov	[no_logo],al
	mov	[verbosity_level],eax
	mov	[maximum_number_of_passes],100
	mov	[maximum_number_of_errors],1
	mov	[maximum_depth_of_stack],10000
	mov	[maximum_depth_of_stack],10000
	mov	esi,command_line
	mov	edi,parameters
    get_argument:
	xor	ah,ah
      read_character:
	lodsb
	test	al,al
	jz	no_more_arguments
	cmp	al,22h
	je	switch_quote
	cmp	ax,20h
	je	end_argument
	stosb
	jmp	read_character
      end_argument:
	xor	al,al
	stosb
    find_next_argument:
	mov	al,[esi]
	test	al,al
	jz	no_more_arguments
	cmp	al,20h
	jne	next_argument_found
	inc	esi
	jmp	find_next_argument
    switch_quote:
	xor	ah,1
	jmp	read_character
    next_argument_found:
	cmp	al,'-'
	je	get_option
	cmp	al,'/'
	je	get_option
	cmp	[source_path],0
	je	get_source_path
	cmp	[output_path],0
	je	get_output_path
    error_in_arguments:
	or	al,-1
	retn
    get_source_path:
	mov	[source_path],edi
	jmp	get_argument
    get_output_path:
	mov	[output_path],edi
	jmp	get_argument
    no_more_arguments:
	cmp	[source_path],0
	je	error_in_arguments
	xor	al,al
	stosb
	retn
    get_option:
	inc	esi
	lodsb
	cmp	al,'e'
	je	set_errors_limit
	cmp	al,'E'
	je	set_errors_limit
	cmp	al,'i'
	je	insert_initial_command
	cmp	al,'I'
	je	insert_initial_command
	cmp	al,'p'
	je	set_passes_limit
	cmp	al,'P'
	je	set_passes_limit
	cmp	al,'r'
	je	set_recursion_limit
	cmp	al,'R'
	je	set_recursion_limit
	cmp	al,'v'
	je	set_verbose_mode
	cmp	al,'V'
	je	set_verbose_mode
	cmp	al,'n'
	je	set_no_logo
	cmp	al,'N'
	jne	error_in_arguments
    set_no_logo:
	or	[no_logo],-1
	mov	al,[esi]
	cmp	al,20h
	je	find_next_argument
	test	al,al
	jnz	error_in_arguments
	jmp	find_next_argument
    set_verbose_mode:
	call	get_option_value
	jc	error_in_arguments
	cmp	edx,2
	ja	error_in_arguments
	mov	[verbosity_level],edx
	jmp	find_next_argument
    set_errors_limit:
	call	get_option_value
	jc	error_in_arguments
	test	edx,edx
	jz	error_in_arguments
	mov	[maximum_number_of_errors],edx
	jmp	find_next_argument
    set_recursion_limit:
	call	get_option_value
	jc	error_in_arguments
	test	edx,edx
	jz	error_in_arguments
	mov	[maximum_depth_of_stack],edx
	jmp	find_next_argument
    set_passes_limit:
	call	get_option_value
	jc	error_in_arguments
	test	edx,edx
	jz	error_in_arguments
	mov	[maximum_number_of_passes],edx
	jmp	find_next_argument
    get_option_value:
	xor	eax,eax
	mov	edx,eax
      find_option_value:
	cmp	byte [esi],20h
	jne	get_option_digit
	inc	esi
	jmp	find_option_value
      get_option_digit:
	lodsb
	cmp	al,20h
	je	option_value_ok
	test	al,al
	jz	option_value_ok
	sub	al,30h
	jc	invalid_option_value
	cmp	al,9
	ja	invalid_option_value
	imul	edx,10
	jo	invalid_option_value
	add	edx,eax
	jc	invalid_option_value
	jmp	get_option_digit
      option_value_ok:
	dec	esi
	clc
	ret
      invalid_option_value:
	stc
	ret
    insert_initial_command:
	push	edi
      find_command_segment:
	cmp	byte [esi],20h
	jne	command_segment_found
	inc	esi
	jmp	find_command_segment
      command_segment_found:
	xor	ah,ah
	cmp	byte [esi],22h
	jne	measure_command_segment
	inc	esi
	inc	ah
      measure_command_segment:
	mov	ebx,esi
      scan_command_segment:
	mov	ecx,esi
	mov	al,[esi]
	test	al,al
	jz	command_segment_measured
	cmp	ax,20h
	je	command_segment_measured
	cmp	ax,22h
	je	command_segment_measured
	inc	esi
	cmp	al,22h
	jne	scan_command_segment
      command_segment_measured:
	sub	ecx,ebx
	mov	edi,[initial_commands]
	lea	eax,[ecx+2]
	test	edi,edi
	jz	allocate_initial_commands_buffer
	mov	edx,[initial_commands_length]
	add	edi,edx
	add	eax,edx
	cmp	eax,[initial_commands_maximum_length]
	ja	grow_initial_commands_buffer
      copy_initial_command:
	xchg	esi,ebx
	rep	movsb
	mov	esi,ebx
	sub	edi,[initial_commands]
	mov	[initial_commands_length],edi
	mov	al,[esi]
	test	al,al
	jz	initial_command_ready
	cmp	al,20h
	jne	command_segment_found
      initial_command_ready:
	mov	edi,[initial_commands]
	add	edi,[initial_commands_length]
	mov	ax,0Ah
	stosw
	inc	[initial_commands_length]
	pop	edi
	jmp	find_next_argument
      allocate_initial_commands_buffer:
	push	ecx
	mov	ecx,eax
	call	malloc
	mov	[initial_commands],eax
	mov	[initial_commands_maximum_length],ecx
	mov	edi,eax
	pop	ecx
	jmp	copy_initial_command
      grow_initial_commands_buffer:
	push	ecx
	mov	ecx,eax
	mov	eax,[initial_commands]
	call	realloc
	mov	[initial_commands],eax
	mov	[initial_commands_maximum_length],ecx
	mov	edi,eax
	add	edi,[initial_commands_length]
	pop	ecx
	jmp	copy_initial_command

  include 'system.inc'

  include '../assembler.inc'
  include '../symbols.inc'
  include '../expressions.inc'
  include '../conditions.inc'
  include '../floats.inc'
  include '../directives.inc'
  include '../calm.inc'
  include '../errors.inc'
  include '../map.inc'
  include '../reader.inc'
  include '../output.inc'
  include '../console.inc'

  _logo db 'flat assembler  version g.',VERSION,13,10,0

  _usage db 'Usage: fasmg source [output]',13,10
	 db 'Optional settings:',13,10
	 db '    -e limit    Set the maximum number of displayed errors (default 1)',13,10
	 db '    -p limit    Set the maximum allowed number of passes (default 100)',13,10
	 db '    -r limit    Set the maximum depth of the stack (default 10000)',13,10
	 db '    -v flag     Enable or disable showing all lines from the stack (default 0)',13,10
	 db '    -i command  Insert instruction at the beginning of source',13,10
	 db '    -n          Do not show logo nor summary',13,10
	 db 0

  _pass db ' pass, ',0
  _passes db ' passes, ',0
  _dot db '.'
  _seconds db ' seconds, ',0
  _byte db ' byte.',0
  _bytes db ' bytes.',0

  _write_failed db 'failed to write the output file',0
  _out_of_memory db 'not enough memory to complete the assembly',0
  _code_cannot_be_generated db 'could not generate code within the allowed number of passes',0

  include '../tables.inc'
  include '../messages.inc'

  align 4

  include '../variables.inc'

  psp_selector dw ?
  main_selector dw ?

  malloc_freelist dd ?

  source_path dd ?
  output_path dd ?
  maximum_number_of_passes dd ?

  initial_commands dd ?
  initial_commands_length dd ?
  initial_commands_maximum_length dd ?

  timestamp dq ?

  timer dd ?
  verbosity_level dd ?
  no_logo db ?

  command_line db 80h dup ?
  parameters db 80h dup ?

segment buffer_segment

  buffer = (buffer_segment-main) shl 4

  db BUFFER_SIZE dup ?

segment stack_segment

  stack_bottom = (stack_segment-main) shl 4

  db STACK_SIZE dup ?

  stack_top = stack_bottom + $
