
define _if _if

define _if..if? _if
define _if..else? _else
define _if..elseif? _elseif
define _if..endif? _endif
define _if..while? _while
define _if..endw? _endw
define _if..repeat? _repeat
define _if..until? _until

calminstruction (cmd) ? a&
	transform cmd,_if
	arrange cmd, cmd a
	assemble cmd
end calminstruction

macro _if cond
	__IF equ :
	local endif
	__ENDIF equ endif
	local else
	__ELSE equ else
	jcondexpr __ELSE,1,cond
end macro

macro _else
	jmp __ENDIF
	match _else, __ELSE
		_else:
	end match
	restore __IF
	__IF equ
end macro

macro _elseif cond
	jmp __ENDIF
	match _else, __ELSE
		_else:
	end match
	restore __ELSE
	local else
	__ELSE equ else
	jcondexpr __ELSE,1,cond
end macro

macro _endif
	match :_else, __IF __ELSE
		_else:
	end match
	match endif, __ENDIF
		endif:
	end match
	restore __ELSE
	restore __ENDIF
	restore __IF
end macro

macro _while cond
	local while
	while:
	__WHILE equ while
	local endw
	__ENDW equ endw
	jcondexpr __ENDW,1,cond
end macro

macro _endw
	jmp __WHILE
	match endw, __ENDW
		endw:
	end match
	restore __ENDW
	restore __WHILE
end macro

macro _repeat
	local repeat
	repeat:
	__REPEAT equ repeat
end macro

macro _until cond
	jcondexpr __REPEAT,1,cond
	restore __REPEAT
end macro

macro newlocal? var
	local new
	redefine var new
end macro

macro JCONDEXPR?: target,mode,cond
	local buffer,current,counter
	local neg,conj
	local f,t
	buffer equ cond
	newlocal f
	newlocal t
	while 1
		match ~x, buffer
			buffer equ x
			neg = (mode) xor 1
		else
			neg = mode
		end match

		match (x, buffer
			counter = 1
			current equ (
			buffer equ x
			while counter > 0
				match p)s, buffer
					match (ps, p
						counter = counter + 1
						current equ current (
						buffer equ ps)s
					else match pp(ps, p
						counter = counter + 1
						current equ current pp(
						buffer equ ps)s
					else
						counter = counter - 1
						current equ current p
						buffer equ )s
					end match
				else
					current equ current buffer
					buffer equ
					break
				end match
			end while
		else
			current equ
		end match

		match a|b, buffer
			match c&d, a
				current equ current c
				buffer equ &d|b
			else
				current equ current a
				buffer equ |b
			end match
		else match c&d, buffer
			match a|b, c
				current equ current a
				buffer equ |b&d
			else
				current equ current c
				buffer equ &d
			end match
		else
			current equ current buffer
			buffer equ
		end match

		match , buffer
			match (c), current
				jcondexpr t,neg,c
			else match c, current
				jcond t,neg,c
			end match
			break
		else
			match |b, buffer
				buffer equ b
				conj = 0
			else match &d, buffer
				buffer equ d
				conj = 1
			else
				err 'invalid expression'
			end match
			if (mode) xor conj
				match (c), current
					jcondexpr f,neg xor 1,c
				else match c, current
					jcond f,neg xor 1,c
				end match
				match t,t
					t:
				end match
				newlocal t

			else
				match (c), current
					jcondexpr t,neg,c
				else match c, current
					jcond t,neg,c
				end match
				match f,f
					f:
				end match
				newlocal f
			end if
		end match
	end while
	match t,t
		label t at target
	end match
	match f,f
		f:
	end match
end macro

macro JCOND? target,neg,cond
	match =signed? v1>==v2, cond
		cmp v1,v2
		if neg
			jl target
		else
			jge target
		end if
	else match =signed? v1<==v2, cond
		cmp v1,v2
		if neg
			jg target
		else
			jle target
		end if
	else match v1>==v2, cond
		cmp v1,v2
		if neg
			jb target
		else
			jae target
		end if
	else match v1<==v2, cond
		cmp v1,v2
		if neg
			ja target
		else
			jbe target
		end if
	else match v1==v2, cond
		cmp v1,v2
		if neg
			jne target
		else
			je target
		end if
	else match v1<>v2, cond
		cmp v1,v2
		if neg
			je target
		else
			jne target
		end if
	else match =signed? v1>v2, cond
		cmp v1,v2
		if neg
			jle target
		else
			jg target
		end if
	else match =signed? v1<v2, cond
		cmp v1,v2
		if neg
			jge target
		else
			jl target
		end if
	else match v1>v2, cond
		cmp v1,v2
		if neg
			jbe target
		else
			ja target
		end if
	else match v1<v2, cond
		cmp v1,v2
		if neg
			jae target
		else
			jb target
		end if
	else match =ZERO=?, cond
		if neg
			jnz target
		else
			jz target
		end if
	else match =CARRY=?, cond
		if neg
			jnc target
		else
			jc target
		end if
	else match =OVERFLOW=?, cond
		if neg
			jno target
		else
			jo target
		end if
	else match =SIGN=?, cond
		if neg
			jns target
		else
			js target
		end if
	else match =PARITY?, cond
		if neg
			jnp target
		else
			jp target
		end if
	else match v, cond
		x86.parse_operand @aux,v
		if @aux.type = 'imm'
			if neg
				if v = 0
					jmp target
				end if
			else
				if v
					jmp target
				end if
			end if
		else if @aux.type = 'reg'
			test v,v
			if neg
				jz target
			else
				jnz target
			end if
		else
			cmp v,0
			if neg
				je target
			else
				jne target
			end if
		end if
	end match
end macro
