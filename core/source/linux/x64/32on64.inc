
macro use32on64
{

	define esp rsp

	define promote.eax rax
	define promote.ebx rbx
	define promote.ecx rcx
	define promote.edx rdx
	define promote.esi rsi
	define promote.edi rdi
	define promote.ebp rbp
	define promote.esp rsp

	macro push args
	\{
		local list,arg,status
		define list
		define arg
		irps sym, args \\{
			define status
			match =dword, sym \\\{
				define status :
			\\\}
			match [any, status arg sym \\\{
				define arg [any
				match [mem], arg \\\\{
					match previous, list \\\\\{ define list previous,[mem] \\\\\}
					match , list \\\\\{ define list [mem] \\\\\}
					define arg
				\\\\}
				define status :
			\\\}
			match [, status arg sym \\\{
				define arg [
				define status :
			\\\}
			match , status \\\{
				match previous, list \\\\{ define list previous,sym \\\\}
				match , list \\\\{ define list sym \\\\}
			\\\}
		\\}
		match ops,list \\{
			irp op, ops \\\{
				if op eqtype eax
					push promote.\\\#op
				else
					mov r8d,op
					push r8
				end if
			\\\}
		\\}
	\}

	macro pop args
	\{
		local list,arg,status
		define list
		define arg
		irps sym, args \\{
			define status
			match =dword, sym \\\{
				define status :
			\\\}
			match [any, status arg sym \\\{
				define arg [any
				match [mem], arg \\\\{
					match previous, list \\\\\{ define list previous,[mem] \\\\\}
					match , list \\\\\{ define list [mem] \\\\\}
					define arg
				\\\\}
				define status :
			\\\}
			match [, status arg sym \\\{
				define arg [
				define status :
			\\\}
			match , status \\\{
				match previous, list \\\\{ define list previous,sym \\\\}
				match , list \\\\{ define list sym \\\\}
			\\\}
		\\}
		match ops,list \\{
			irp op, ops \\\{
				if op eqtype eax
					pop promote.\\\#op
				else
					pop r8
					mov op,r8d
				end if
			\\\}
		\\}
	\}

	irp instr, jmp,call
	\{
		macro instr op
		\\{
			if op eqtype [0]
				mov	r8d,op
				instr	r8
			else if op eqtype 0
				instr op
			else
				instr	promote.\\#op
			end if
		\\}
	\}

	macro jecxz target
	\{
		if target-($+1) < 80h & target-($+1) >= -80h
			jecxz	target
		else
			local	j,k
			jecxz	j
			jmp	k
		    j:	jmp	target
		    k:
		end if
	\}

}
