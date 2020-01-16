
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
