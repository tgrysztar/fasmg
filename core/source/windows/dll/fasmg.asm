
match ,{

	include 'win32a.inc'
	include 'localptr.inc'

} match -,{
else

	include 'selfhost.inc'

end match
_ equ }

	format PE large NX DLL
	entry DllEntryPoint

include '../../version.inc'

struct MEMORY_REGION
	address dd ?
	size dd ?
ends

section '.text' code executable

  include 'system.inc'

  include '../../assembler.inc'
  include '../../symbols.inc'
  include '../../expressions.inc'
  include '../../conditions.inc'
  include '../../floats.inc'
  include '../../directives.inc'
  include '../../calm.inc'
  include '../../errors.inc'
  include '../../map.inc'
  include '../../reader.inc'
  include '../../output.inc'
  include '../../console.inc'

  DllEntryPoint:
	mov	eax,1
	retn	12

  fasmg_GetVersion:
	mov	eax,version_string
	retn

  fasmg_Assemble:

	virtual at ebp - LOCAL_VARIABLES_SIZE

		LocalVariables:

			include '../../variables.inc'

			maximum_number_of_passes dd ?

			timestamp dq ?
			systemtime SYSTEMTIME
			filetime FILETIME

			memory dd ?
			bytes_count dd ?
			position_high dd ?

			rb  (LocalVariables - $) and 11b

		LOCAL_VARIABLES_SIZE = $ - LocalVariables

		assert $ - ebp = 0

		previous_frame dd ?
		stored_edi dd ?
		stored_esi dd ?
		stored_ebx dd ?
		return_address dd ?

		FunctionParameters:

			source_string dd ?
			source_path dd ?
			output_region dd ?
			output_path dd ?
			stdout dd ?
			stderr dd ?

		FUNCTION_PARAMETERS_SIZE = $ - FunctionParameters

	end virtual

	push	ebx esi edi
	enter	LOCAL_VARIABLES_SIZE,0

	call	system_init

	mov	[maximum_number_of_passes],100
	mov	[maximum_number_of_errors],1000
	mov	[maximum_depth_of_stack],10000

	xor	al,al
	call	assembly_init

  assemble:
	mov	esi,[source_string]
	mov	edx,[source_path]
	call	assembly_pass
	jc	assembly_done

	mov	eax,[current_pass]
	cmp	eax,[maximum_number_of_passes]
	jb	assemble

	call	show_display_data
	call	assembly_shutdown
	call	system_shutdown
	mov	eax,-2
	leave
	pop	edi esi ebx
	retn	FUNCTION_PARAMETERS_SIZE

  assembly_done:

	call	show_display_data

	cmp	[first_error],0
	jne	assembly_failed

	mov	esi,[output_region]
	test	esi,esi
	jz	output_copied
	call	get_output_length
	test	edx,edx
	jnz	out_of_memory
	mov	[value_length],eax
	xchg	eax,[esi+MEMORY_REGION.size]
	cmp	[esi+MEMORY_REGION.address],0
	je	new_region_for_output
	cmp	eax,[value_length]
	jae	copy_output
	invoke	VirtualAlloc,[esi+MEMORY_REGION.address],[esi+MEMORY_REGION.size],MEM_COMMIT,PAGE_READWRITE
	test	eax,eax
	jnz	copy_output
	invoke	VirtualFree,[esi+MEMORY_REGION.address],0,MEM_RELEASE
   new_region_for_output:
	invoke	VirtualAlloc,0,[esi+MEMORY_REGION.size],MEM_COMMIT,PAGE_READWRITE
	test	eax,eax
	jz	out_of_memory
	mov	[esi+MEMORY_REGION.address],eax
   copy_output:
	mov	edi,[esi+MEMORY_REGION.address]
	xor	eax,eax
	mov	dword [file_offset],eax
	mov	dword [file_offset+4],eax
	call	read_from_output
   output_copied:

	mov	ebx,[source_path]
	mov	edi,[output_path]
	mov	eax,ebx
	or	eax,edi
	jz	output_written
	call	write_output_file
	jc	write_failed
   output_written:

	call	assembly_shutdown
	call	system_shutdown
	xor	eax,eax
	leave
	pop	edi esi ebx
	retn	FUNCTION_PARAMETERS_SIZE

  assembly_failed:
	mov	eax,[first_error]
	xor	ecx,ecx
      count_errors:
	inc	ecx
	mov	eax,[eax+Error.next]
	test	eax,eax
	jnz	count_errors
	push	ecx
	call	show_errors
	call	assembly_shutdown
	call	system_shutdown
	pop	eax
	leave
	pop	edi esi ebx
	retn	FUNCTION_PARAMETERS_SIZE

  write_failed:
	call	assembly_shutdown
	call	system_shutdown
	mov	eax,-3
	leave
	pop	edi esi ebx
	retn	FUNCTION_PARAMETERS_SIZE

  out_of_memory:
	call	assembly_shutdown
	call	system_shutdown
	mov	eax,-1
	leave
	pop	edi esi ebx
	retn	FUNCTION_PARAMETERS_SIZE

section '.rdata' data readable

  include '../../tables.inc'
  include '../../messages.inc'

  version_string db VERSION,0

section '.idata' import data readable

  library kernel32,'KERNEL32.DLL'

  import kernel32,\
	 CloseHandle,'CloseHandle',\
	 CreateFile,'CreateFileA',\
	 ExitProcess,'ExitProcess',\
	 GetEnvironmentVariable,'GetEnvironmentVariableA',\
	 GetSystemTime,'GetSystemTime',\
	 GetTickCount,'GetTickCount',\
	 VirtualAlloc,'VirtualAlloc',\
	 VirtualFree,'VirtualFree',\
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

section '.edata' export data readable

  export 'FASMG.DLL',\
	 fasmg_GetVersion,'fasmg_GetVersion',\
	 fasmg_Assemble,'fasmg_Assemble'

section '.reloc' fixups data readable discardable
	   