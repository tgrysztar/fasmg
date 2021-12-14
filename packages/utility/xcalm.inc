
; Helper commands for writing CALM instructions

; INIT
; this command can be used to give an initial numeric value to local variable
; at the time when the CALM instruction is defined
calminstruction calminstruction?.init? var*, val:0
	compute val, val
	publish var, val
end calminstruction

; INITSYM
; this command can be used to give an initial symbolic value to local variable
; at the time when the CALM instruction is defined
; (any symbols in the value also keep the context of that instruction's namespace)
calminstruction calminstruction?.initsym? var*, val&
	publish var, val
end calminstruction

; UNIQUE
; generates a new unique identifier and stores it in given variable
; (the identifier uses the name of said variable as a prefix)
calminstruction calminstruction?.unique? name
	local counter, buffer
	init counter
	compute counter, counter + 1
	arrange buffer, name#counter
	publish name, buffer
end calminstruction

; ASM
; generates code to assemble given line of text as-is
; (any symbols in this text keep the context of the instruction's namespace)
calminstruction calminstruction?.asm? line&
	local tmp, ln, buffer
	initsym tmp, unique ln
	assemble tmp
	publish ln, line
	arrange buffer, =assemble ln
	assemble buffer
end calminstruction

; CALL
; extends the built-in command with the ability to define some arguments as literal values;
; arguments enclosed with () are treated as numeric,
; ones enclosed with <> are treated as a text of symbolic value.
calminstruction calminstruction?.call? instruction*, arguments&
	arrange instruction, =call instruction
    convert:
	match	, arguments
	jyes	ready
	local	v
	match	v=,arguments, arguments, <>
	jyes	recognize
	arrange v, arguments
	arrange arguments,
    recognize:
	match	(v), v
	jyes	numeric
	match	<v>, v
	jyes	symbolic
    append:
	arrange instruction, instruction=,v
	jump	convert
    numeric:
	compute v, v
    symbolic:
	local name
	asm unique name
	publish name, v
	arrange instruction, instruction=,name
	jump	convert
    ready:
	assemble instruction
end calminstruction
