
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

; ASM
; generates code to assemble given line of text as-is
; (any symbols in this text keep the context of the instruction's namespace)
calminstruction calminstruction?.asm? line&
	local	name, i
	initsym name, name.0
	match	name.i, name
	compute i, i+1
	arrange name, name.i
	publish name:, line
	arrange line, =assemble name
	assemble line
end calminstruction

; UNIQUE
; generates a new unique identifier at the definition time of CALM instruction;
; the identifier is stored in variable specified by name,
; and the same name is used as a prefix for generated identifier
calminstruction calminstruction?.unique? name
	local i, buffer
	init i
	compute i, i+1
	arrange buffer, name#i
	publish name, buffer
end calminstruction

; XCALL
; extends the CALL command with the ability to define some arguments as literal values;
; arguments enclosed with () are treated as numeric,
; ones enclosed with <> are treated as a text of symbolic value
calminstruction calminstruction?.xcall? instruction*, arguments&
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
	match	<v?>, v
	jyes	symbolic
    append:
	arrange instruction, instruction=,v
	jump	convert
    numeric:
	compute v, v
    symbolic:
	local	name
	asm	unique name
	publish name, v
	arrange instruction, instruction=,name
	jump	convert
    ready:
	assemble instruction
end calminstruction
