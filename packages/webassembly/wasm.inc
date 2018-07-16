
include 'wasmcode.inc'

format binary as 'wasm'

define wasm

wasm.section.type	= 1
wasm.section.import	= 2
wasm.section.function	= 3
wasm.section.table	= 4
wasm.section.memory	= 5
wasm.section.global	= 6
wasm.section.export	= 7
wasm.section.start	= 8
wasm.section.element	= 9
wasm.section.code	= 10
wasm.section.data	= 11

wasm.type.i32	  = -0x01 and 7Fh
wasm.type.i64	  = -0x02 and 7Fh
wasm.type.f32	  = -0x03 and 7Fh
wasm.type.f64	  = -0x04 and 7Fh
wasm.type.anyfunc = -0x10 and 7Fh
wasm.type.func	  = -0x20 and 7Fh
wasm.type.empty   = -0x40 and 7Fh

wasm.kind.function = 0
wasm.kind.table    = 1
wasm.kind.memory   = 2
wasm.kind.global   = 3

macro section? id*
	local payload_data,payload_len

	if defined wasm.section.id
		varuint7 wasm.section.id
	else
		varuint7 0
	end if

	varuint32 payload_len

	if ~ defined wasm.section.id
		varuint32 lengthof `id
		db `id
	end if

	payload_data:

	macro end?.section?
		payload_len = $ - payload_data
		purge end?.section?
	end macro
end macro

struc functype result,parameters&
	local signature_buffer,count
	signature_buffer equ func_#result
	iterate parameter, parameters
		match signature, signature_buffer
			signature_buffer reequ signature#_#parameter
		end match
	end iterate
	match funcsig, signature_buffer
		if defined wasm.signature.funcsig
			. = wasm.signature.funcsig
		else
			restore wasm.signature.funcsig
			wasm.signature.funcsig = wasm.type_index
			repeat 1, index: wasm.type_index
				wasm.type_#index = wasm.type.func
				match , result
					wasm.type_#index.result_count := 0
				else
					wasm.type_#index.result_count := 1
					wasm.type_#index.result_type := result
				end match
				wasm.type_#index.parameter_types equ parameters
				. = wasm.type_index
				wasm.type_index = wasm.type_index + 1
			end repeat
		end if
	end match
end struc

macro function declaration*&
	repeat 1, index: wasm.function_index
		wasm.function_#index.signature equ
		match function(parameters), declaration
			wasm.function_#index equ function
			iterate parameter, parameters
				match parameter_name:parameter_type, parameter
					wasm.function_#index.body.parameter_name := %-1
					wasm.function_#index.signature reequ wasm.function_#index.signature,parameter_type
				else match parameter_type parameter_name, parameter
					wasm.function_#index.body.parameter_name := %-1
					wasm.function_#index.signature reequ wasm.function_#index.signature,parameter_type
				else
					err 'parameter "',`parameter,'" is missing a type'
				end match
				if % = %%
					wasm.function_#index.parameters_count := %%
				end if
			end iterate
		else match function(), declaration
			wasm.function_#index equ function
			wasm.function_#index.parameters_count := 0
		else
			err '"',`declaration,'" is not a valid function declaration'
		end match
		match function_name:result_type, wasm.function_#index
			function_name := index
			wasm.function_#index.name := `function_name
			wasm.function_#index.signature reequ result_type wasm.function_#index.signature
		else match result_type function_name, wasm.function_#index
			function_name := index
			wasm.function_#index.name := `function_name
			wasm.function_#index.signature reequ result_type wasm.function_#index.signature
		else
			function := index
			wasm.function_#index.name := `function
		end match
		match signature, wasm.function_#index.signature
			wasm.function_#index.type_index functype signature
		end match
		varuint32 wasm.function_#index.end - wasm.function_#index.body
		wasm.function_#index.body:
		match list, wasm.function_#index.locals
			local type,next_type,count
			count = 0
			iterate local_type, list
				if % = 1
					varuint32 %%		; number of local entries
				end if
				count = count + 1
				type = wasm.type.local_type
				if % < %%
					indx %+1
					next_type = wasm.type.local_type
				end if
				if % = %% | next_type <> type
					varuint32 count 	; number of variables of this type
					varint7 type
					count = 0
				end if
			end iterate
		else
			varuint32 0
		end match
		wasm.function_#index.local_index = wasm.function_#index.parameters_count
		wasm.function_#index.local_list equ
		namespace wasm.function_#index.body
		macro var variables&
			iterate variable, variables
				match variable_name:variable_type, variable
					variable_name := wasm.function_#index.local_index
					match list, wasm.function_#index.local_list
						wasm.function_#index.local_list reequ list,variable_type
					else
						wasm.function_#index.local_list reequ variable_type
					end match
				else match variable_type variable_name, variable
					variable_name := wasm.function_#index.local_index
					match list, wasm.function_#index.local_list
						wasm.function_#index.local_list reequ list,variable_type
					else
						wasm.function_#index.local_list reequ variable_type
					end match
				else
					err 'variable "',`variable,'" is missing a type'
				end match
				wasm.function_#index.local_index = wasm.function_#index.local_index + 1
			end iterate
		end macro
	end repeat

end macro

macro end?.function
	end namespace
	db 0x0b
	repeat 1, index: wasm.function_index
		wasm.function_#index.end:
		wasm.function_#index.locals equ wasm.function_#index.local_list
	end repeat
	wasm.function_index = wasm.function_index + 1
end macro


uint32 0x6d736100
uint32 0x1	; WebAssembly version

section type
	varuint32 wasm.type_count				; count of type entries
	repeat wasm.type_count, index:0
		varint7 wasm.type_#index
		match list, wasm.type_#index.parameter_types
			iterate parameter_type, list
				if % = 1
					varuint32 %%		; number of parameters
				end if
				varint7 wasm.type.parameter_type
			end iterate
		else
			varuint32 0
		end match
		varuint1 wasm.type_#index.result_count		; number of results
		repeat wasm.type_#index.result_count
			varint7 wasm.type_#index.result_type	; type of result
		end repeat
	end repeat
end section

section function
	varuint32 wasm.function_count				; count of functions
	repeat wasm.function_count, index:0
		varuint32 wasm.function_#index.type_index	; index into type section
	end repeat
end section

section export
	varuint32 wasm.function_count				; count of exported functions
	repeat wasm.function_count, index:0
		varuint32 lengthof wasm.function_#index.name
		db wasm.function_#index.name
		db wasm.kind.function
		varuint32 %-1					; index of function
	end repeat
end section

section code
	varuint32 wasm.function_count				; count of functions

wasm.type_index = 0
wasm.function_index = 0

postpone
	end section
	wasm.type_count := wasm.type_index
	wasm.function_count := wasm.function_index
end postpone
