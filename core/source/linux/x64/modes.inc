
macro x86.encode_address_32_64 ns
	x86.encode_address_32_64 ns
	if ns.address_registers eq esp & x86.mode = 64
		ns.mode = 64
		ns.displacement = ns.displacement shl 1
	end if
end macro

iterate <reg32,reg64>, eax,rax, ebx,rbx, ecx,rcx, edx,rdx, esi,rsi, edi,rdi, esp,rsp, ebp,rbp
	promote.reg32? equ :reg64
end iterate

iterate instr, jmp,call
	macro instr? arg
		match [mem], arg
			mov	r8d,[mem]
			instr	r8
		else match :reg64, promote.arg
			instr	reg64
		else
			instr	arg
		end match
	end macro
end iterate

purge push?,pop?

macro push? arg
	match :reg64, promote.arg
		push	reg64
	else
		mov	r8d,arg
		push	r8
	end match
end macro

macro pop? arg
	match :reg64, promote.arg
		pop	reg64
	else
		pop	r8
		mov	arg,r8d
	end match
end macro

iterate instr, push,pop
	macro instr? op
		local sequence
		sequence equ op --
		while 1
			match --, sequence
				break
			else match car= cdr, sequence
				redefine sequence cdr
				match :sz, x86.car
					match --, sequence
						instr car
						break
					else match head= tail, sequence
						redefine sequence tail
						instr car head
					end match
				else
					instr car
				end match
			end match
		end while
	end macro
end iterate

macro jecxz? target
	if target-($+1) < 80h & target-($+1) >= -80h
		jecxz	target
	else
		local	j,k
		jecxz	j
		jmp	k
	    j:	jmp	target
	    k:
	end if
end macro
