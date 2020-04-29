
match ,{

	err ; fasm 1 assembly not supported

} match -,{
else

	include 'selfhost.inc'

end match
_ equ }

format MachO
public main as '_main'

include '../version.inc'

extrn '_malloc' as libc.malloc
extrn '_realloc' as libc.realloc
extrn '_free' as libc.free
extrn '_fopen' as libc.fopen
extrn '_fclose' as libc.fclose
extrn '_fread' as libc.fread
extrn '_fwrite' as libc.fwrite
extrn '_fseek' as libc.fseek
extrn '_ftell' as libc.ftell
extrn '_time' as libc.time
extrn '_write' as libc.write

extrn '_getenv' as getenv
extrn '_gettimeofday' as gettimeofday
extrn '_exit' as exit

struct timeval
	time_t dd ?
	suseconds_t dd ?
ends

section '__TEXT':'__text' align 16

  main:
	mov	ecx,[esp+4]
	mov	[argc],ecx
	mov	ebx,[esp+8]
	mov	[argv],ebx

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

	ccall	gettimeofday,start_time,0

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
	ccall	gettimeofday,end_time,0
	mov	eax,[end_time.time_t]
	sub	eax,[start_time.time_t]
	mov	ecx,1000000
	mul	ecx
	add	eax,[end_time.suseconds_t]
	adc	edx,0
	sub	eax,[start_time.suseconds_t]
	sbb	edx,0
	add	eax,50000
	mov	ecx,1000000
	div	ecx
	mov	ebx,eax
	mov	eax,edx
	xor	edx,edx
	mov	ecx,100000
	div	ecx
	mov	[tenths_of_second],eax
	xchg	eax,ebx
	or	ebx,eax
	jz	display_output_length
	xor	edx,edx
	call	itoa
	call	display_string
	mov	esi,_message_suffix
	mov	ecx,1
	call	display_string
	mov	eax,[tenths_of_second]
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

	ccall	exit,0

  assembly_failed:

	call	show_errors

	call	assembly_shutdown
	call	system_shutdown

	ccall	exit,2

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

	ccall	exit,3

  display_usage_information:

	mov	esi,_usage
	xor	ecx,ecx
	call	display_string

	call	system_shutdown

	ccall	exit,1

  get_arguments:
	xor	eax,eax
	mov	[initial_commands],eax
	mov	[source_path],eax
	mov	[output_path],eax
	mov	[no_logo],al
	mov	[verbosity_level],eax
	mov	[maximum_number_of_passes],100
	mov	[maximum_number_of_errors],1
	mov	[maximum_depth_of_stack],10000
	mov	ecx,[argc]
	mov	ebx,[argv]
	add	ebx,4
	dec	ecx
	jz	error_in_arguments
    get_argument:
	mov	esi,[ebx]
	mov	al,[esi]
	cmp	al,'-'
	je	get_option
	cmp	[source_path],0
	jne	get_output_file
	mov	[source_path],esi
	jmp	next_argument
    get_output_file:
	cmp	[output_path],0
	jne	error_in_arguments
	mov	[output_path],esi
	jmp	next_argument
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
	cmp	byte [esi],0
	je	next_argument
    error_in_arguments:
	or	al,-1
	ret
    set_verbose_mode:
	cmp	byte [esi],0
	jne	get_verbose_setting
	dec	ecx
	jz	error_in_arguments
	add	ebx,4
	mov	esi,[ebx]
      get_verbose_setting:
	call	get_option_value
	cmp	edx,2
	ja	error_in_arguments
	mov	[verbosity_level],edx
	jmp	next_argument
    set_errors_limit:
	cmp	byte [esi],0
	jne	get_errors_setting
	dec	ecx
	jz	error_in_arguments
	add	ebx,4
	mov	esi,[ebx]
      get_errors_setting:
	call	get_option_value
	test	edx,edx
	jz	error_in_arguments
	mov	[maximum_number_of_errors],edx
	jmp	next_argument
    set_recursion_limit:
	cmp	byte [esi],0
	jne	get_recursion_setting
	dec	ecx
	jz	error_in_arguments
	add	ebx,4
	mov	esi,[ebx]
      get_recursion_setting:
	call	get_option_value
	test	edx,edx
	jz	error_in_arguments
	mov	[maximum_depth_of_stack],edx
	jmp	next_argument
    set_passes_limit:
	cmp	byte [esi],0
	jne	get_passes_setting
	dec	ecx
	jz	error_in_arguments
	add	ebx,4
	mov	esi,[ebx]
      get_passes_setting:
	call	get_option_value
	test	edx,edx
	jz	error_in_arguments
	mov	[maximum_number_of_passes],edx
    next_argument:
	add	ebx,4
	dec	ecx
	jnz	get_argument
	cmp	[source_path],0
	je	error_in_arguments
	xor	al,al
	ret
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
	cmp	byte [esi],0
	jne	measure_initial_command
	dec	ecx
	jz	error_in_arguments
	add	ebx,4
	mov	esi,[ebx]
      measure_initial_command:
	push	ebx ecx edi
	mov	edi,esi
	or	ecx,-1
	xor	al,al
	repne	scasb
	not	ecx
	dec	ecx
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
	rep	movsb
	mov	ax,0Ah
	stosw
	dec	edi
	sub	edi,[initial_commands]
	mov	[initial_commands_length],edi
	pop	edi ecx ebx
	jmp	next_argument
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

section '__TEXT':'__cstring' align 4

  _logo db 'flat assembler  version g.',VERSION,10,0

  _usage db 'Usage: fasmg source [output]',10
	 db 'Optional settings:',10
	 db '    -p limit    Set the maximum allowed number of passes (default 100)',10
	 db '    -e limit    Set the maximum number of displayed errors (default 1)',10
	 db '    -r limit    Set the maximum depth of stack (default 10000)',10
	 db '    -v flag     Enable or disable showing all lines from the stack (default 0)',10
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

  _open_mode db 'r',0
  _create_mode db 'w',0

  include '../tables.inc'
  include '../messages.inc'

section '__DATA':'__data' align 4

  include '../variables.inc'

  source_path dd ?
  output_path dd ?
  maximum_number_of_passes dd ?

  initial_commands dd ?
  initial_commands_length dd ?
  initial_commands_maximum_length dd ?

  argc dd ?
  argv dd ?
  timestamp dq ?

  start_time timeval
  end_time timeval
  tenths_of_second dd ?

  verbosity_level dd ?
  no_logo db ?

  path_buffer rb 1000h
