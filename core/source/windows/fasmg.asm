
match ,{

	include 'win32a.inc'

} match -,{
else

	include 'selfhost.inc'

end match
_ equ }


	format	PE large NX console 4.0
	entry	start

include '../version.inc'

section '.text' code executable

  include 'system.inc'

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
	movzx	ecx,[verbosity_level]
	jecxz	init
	or	al,TRACE_ERROR_STACK
	dec	ecx
	jz	init
	or	al,TRACE_DISPLAY
  init:
	call	assembly_init

	invoke	GetTickCount
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
	invoke	GetTickCount
	sub	eax,[timer]
	xor	edx,edx
	add	eax,50
	mov	ecx,1000
	div	ecx
	mov	ebx,eax
	mov	eax,edx
	xor	edx,edx
	mov	ecx,100
	div	ecx
	mov	[timer],eax
	xchg	eax,ebx
	or	ebx,eax
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

	invoke	ExitProcess,0

  assembly_failed:

	call	show_errors

	call	assembly_shutdown
	call	system_shutdown

	invoke	ExitProcess,2

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

	invoke	ExitProcess,3

  display_usage_information:

	mov	esi,_usage
	xor	ecx,ecx
	call	display_string

	call	system_shutdown

	invoke	ExitProcess,1

  get_arguments:
	xor	eax,eax
	mov	[initial_commands],eax
	mov	[source_path],eax
	mov	[output_path],eax
	mov	[no_logo],al
	mov	[verbosity_level],al
	mov	[maximum_number_of_passes],100
	mov	[maximum_number_of_errors],1
	mov	[maximum_depth_of_stack],10000
	invoke	GetCommandLine
	mov	esi,eax
	mov	edi,eax
	or	ecx,-1
	xor	al,al
	repne	scasb
	sub	edi,esi
	mov	ecx,edi
	call	malloc
	mov	edi,eax
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
	mov	[verbosity_level],dl
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

  include '../symbols.inc'
  include '../assembler.inc'
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

section '.bss' readable writeable

  include '../variables.inc'

  source_path dd ?
  output_path dd ?
  maximum_number_of_passes dd ?

  initial_commands dd ?
  initial_commands_length dd ?
  initial_commands_maximum_length dd ?

  stdout dd ?
  stderr dd ?
  memory dd ?
  timestamp dq ?
  systemtime SYSTEMTIME
  filetime FILETIME
  systmp dd ?

  timer dd ?
  verbosity_level db ?
  no_logo db ?

section '.rdata' data readable

data import

	library kernel32,'KERNEL32.DLL'

	import kernel32,\
	       CloseHandle,'CloseHandle',\
	       CreateFile,'CreateFileA',\
	       ExitProcess,'ExitProcess',\
	       GetCommandLine,'GetCommandLineA',\
	       GetEnvironmentVariable,'GetEnvironmentVariableA',\
	       GetStdHandle,'GetStdHandle',\
	       GetSystemTime,'GetSystemTime',\
	       GetTickCount,'GetTickCount',\
	       HeapAlloc,'HeapAlloc',\
	       HeapCreate,'HeapCreate',\
	       HeapDestroy,'HeapDestroy',\
	       HeapFree,'HeapFree',\
	       HeapReAlloc,'HeapReAlloc',\
	       HeapSize,'HeapSize',\
	       ReadFile,'ReadFile',\
	       SetFilePointer,'SetFilePointer',\
	       SystemTimeToFileTime,'SystemTimeToFileTime',\
	       WriteFile,'WriteFile',\
	       GetLastError,'GetLastError'

end data

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
