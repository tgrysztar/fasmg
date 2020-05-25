
define stdcall? stdcall

macro stdcall?.push_string value&
	local continue
	if sizeof.TCHAR = 2
		local alignment
		virtual at $+5
			align 2
			alignment = $-$$
		end virtual
		db alignment dup 90h
	end if
	call continue
	TCHAR value,0
	continue:
end macro

macro stdcall?: proc*,args&
	local count
	count = 0
	iterate arg, args
		indx 1+%%-%
		match =addr? val, arg
			if val relativeto 0 | val relativeto $
				push dword val
			else
				lea edx,[val]
				push edx
			end if
		else match =double? [var], arg
			push dword [var+4]
			push dword [var]
			count = count + 1
		else match =double? val, arg
			local low,high
			virtual at 0
				dq val
				load low:dword from 0
				load high:dword from 4
			end virtual
			push dword high
			push dword low
			count = count + 1
		else match =invoke? func, arg
			invoke func
			push eax
		else match =cinvoke? func, arg
			cinvoke func
			push eax
		else match =stdcall? func, arg
			stdcall func
			push eax
		else match =ccall? func, arg
			ccall func
			push eax
		else match first=,rest, arg
			stdcall?.push_string arg
		else match size [var], arg
			if size = 4
				push arg
			else
				if size = 1
					mov al, arg
				else if size = 2
					mov ax, arg
				else
					mov eax, arg
				end if
				push eax
			end if
		else match [var], arg
			push dword arg
		else
			if arg eqtype ''
				stdcall?.push_string arg
			else
				push dword arg
			end if
		end match
		count = count + 1
	end iterate
	pcountcheck proc,count
	call proc
end macro

macro ccall?: proc*,args&
	local count
	count = 0
	iterate arg, args
		indx 1+%%-%
		match =addr? val, arg
			if val relativeto 0 | val relativeto $
				push dword val
			else
				lea edx,[val]
				push edx
			end if
		else match =double? [var], arg
			push dword [var+4]
			push dword [var]
			count = count + 1
		else match =double? val, arg
			local low,high
			virtual at 0
				dq val
				load low:dword from 0
				load high:dword from 4
			end virtual
			push dword high
			push dword low
			count = count + 1
		else match =invoke? func, arg
			invoke func
			push eax
		else match =cinvoke? func, arg
			cinvoke func
			push eax
		else match =stdcall? func, arg
			stdcall func
			push eax
		else match =ccall? func, arg
			ccall func
			push eax
		else match first=,rest, arg
			stdcall?.push_string arg
		else match size [var], arg
			if size = 4
				push arg
			else
				if size = 1
					mov al, arg
				else if size = 2
					mov ax, arg
				else
					mov eax, arg
				end if
				push eax
			end if
		else match [var], arg
			push dword arg
		else
			if arg eqtype ''
				stdcall?.push_string arg
			else
				push dword arg
			end if
		end match
		count = count + 1
	end iterate
	pcountcheck proc,count
	call proc
	if count
		add esp,count*4
	end if
end macro

macro invoke?: proc*,args&
	stdcall [proc],args
end macro

macro cinvoke?: proc*,args&
	ccall [proc],args
end macro

macro pcountcheck? proc*,args*
end macro

define pcountsuffix %

prologue@proc equ prologuedef

macro prologuedef procname,flag,parmbytes,localbytes,reglist
	local loc
	loc = (localbytes+3) and (not 3)
	parmbase@proc equ ebp+8
	localbase@proc equ ebp-loc
	if parmbytes | localbytes
		push ebp
		mov ebp,esp
		if localbytes
			sub esp,loc
		end if
	end if
	iterate reg, reglist
		push reg
	end iterate
end macro

epilogue@proc equ epiloguedef

macro epiloguedef procname,flag,parmbytes,localbytes,reglist
	iterate reg, reglist
		indx %%-%+1
		pop reg
	end iterate
	if parmbytes | localbytes
		leave
	end if
	if flag and 10000b
		retn
	else
		retn parmbytes
	end if
end macro

close@proc equ

macro proc? statement&

	local _local,params,flag,regs,parmbytes,localbytes,current,tmp,initlocal

	macro endp?!

			localbytes = current

			purge ret?,locals?,endl?,proclocal?

			match close:reglist, close@proc,<regs>
				close name,flag,parmbytes,localbytes,reglist
			end match

		end match
		end namespace
		end if
		end match

		purge endp?

	end macro

	match name declaration, statement :
	if used name
	name:
	namespace name
	outscope match local?, proclocal

		match =stdcall? args :, declaration
			define params args
			flag = 11b
		else match =stdcall? :, declaration
			define params
			flag = 11b
		else match =c? args :, declaration
			define params args
			flag = 10001b
		else match =c? :, declaration
			define params
			flag = 10001b
		else match args :, declaration
			define params args
			flag = 0
		else
			define params
			flag = 0
		end match
		define regs
		match =uses? list, params
			define params list
			while 1
				match =, tail, params
					define params tail
					break
				else match reg tail, params&
					match more&, tail
						define params more
					else
						define params
					end match
					if % = 1
						regs equ reg
					else
						regs equ regs,reg
					end if
				else
					break
				end match
			end while
		else match =, tail, params
			define params tail
		end match

		match prologue:reglist, prologue@proc:<regs>
			prologue name,flags,parmbytes,localbytes,reglist
		end match

		virtual at parmbase@proc
			namespace name
				match args, params
					iterate arg, args
						match argname:type, arg
							label argname:type
							rb type
						else
							?arg dd ?
						end match
					end iterate
				end match
				parmbytes := $-(parmbase@proc)
				match p, pcountsuffix
					name#p = parmbytes/4
				end match
			end namespace
		end virtual

		macro ret? operand
			match any, operand
				retn operand
			else
				match epilogue:reglist, epilogue@proc:<regs>
					epilogue name,flag,parmbytes,localbytes,reglist
				end match
			end match
		end macro

		current = 0

		macro initlocal
			local area,pointer,length,value
			area::
			pointer = localbase@proc+current
			length = $@ - (localbase@proc) - current
			current = $ - (localbase@proc)
			end virtual
			while length > 0
				if length < 2
					load value:byte from area:pointer
					mov byte [pointer],value
					pointer = pointer + 1
					length = length - 1
				else if length < 4
					load value:word from area:pointer
					mov word [pointer],value
					pointer = pointer + 2
					length = length - 2
				else
					load value:dword from area:pointer
					mov dword [pointer],value
					pointer = pointer + 4
					length = length - 4
				end if
			end while
			virtual at localbase@proc+current
		end macro

		macro locals?
			virtual at localbase@proc+current
			iterate dword, dword,qword
				macro dword? value
					if value relativeto 0
						emit dword: value
					else
						initlocal
						local pointer
						pointer := $
						end virtual
						mov dword [pointer],value
						virtual at pointer+4
						current = $ - (localbase@proc)
					end if
				end macro
			end iterate
			macro ? line&
				line
				if $ > $@
					initlocal
				end if
			end macro
		end macro

		macro endl?
			purge ?, dword?,qword?
			initlocal
			end virtual
		end macro

		macro proclocal? args&
			locals
				iterate arg, args
					match varname[count]:type, arg
						?varname dbx type:count dup ?
					else match varname:type, arg
						?varname dbx type, ?
					else match varname[count], arg
						?varname rd count
					else match varname type, arg
						?varname type
					else
						?arg dd ?
					end match
				end iterate
			endl
		end macro

end macro
