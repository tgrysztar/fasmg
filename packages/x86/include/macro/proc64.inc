define fastcall? fastcall

fastcall.r1 equ rcx
fastcall.rd1 equ ecx
fastcall.rw1 equ cx
fastcall.rb1 equ cl
fastcall.rf1 equ xmm0

fastcall.r2 equ rdx
fastcall.rd2 equ edx
fastcall.rw2 equ dx
fastcall.rb2 equ dl
fastcall.rf2 equ xmm1

fastcall.r3 equ r8
fastcall.rd3 equ r8d
fastcall.rw3 equ r8w
fastcall.rb3 equ r8b
fastcall.rf3 equ xmm2

fastcall.r4 equ r9
fastcall.rd4 equ r9d
fastcall.rw4 equ r9w
fastcall.rb4 equ r9b
fastcall.rf4 equ xmm3

fastcall?.frame = -1

macro frame?
	local size
	define fastcall?.frame_size size
	fastcall?.frame =: 0
	sub rsp,size
end macro

macro end?.frame?
	match size, fastcall?.frame_size
		size := fastcall?.frame
		add rsp,size
	end match
	restore fastcall?.frame,fastcall?.frame_size
end macro

macro endf?
	end frame
end macro

macro fastcall?.inline_string var
	local data,continue
	jmp continue
	if sizeof.TCHAR = 2
		align 2,90h
	end if
	match value, var
		data TCHAR value,0
	end match
	redefine var data
	continue:
end macro

macro fastcall?: proc*,args&
	local offset,framesize,type,value
	if framesize & fastcall?.frame < 0
		sub rsp,framesize
	end if
	offset = 0
	iterate arg, args
		match =float? val, arg
			type = 'f'
			redefine value val
			SSE.parse_operand @src,val
		else match =addr? val, arg
			type = 'a'
			redefine value val
			x86.parse_operand @src,[val]
		else match =invoke? func, arg
			frame
			invoke func
			end frame
			type = 0
			redefine value rax
			x86.parse_operand @src,rax
		else match =fastcall? func, arg
			frame
			fastcall func
			end frame
			type = 0
			redefine value rax
			x86.parse_operand @src,rax
		else match first=,rest, arg
			type = 's'
			redefine value arg
		else
			type = 0
			redefine value arg
			SSE.parse_operand @src,arg
			if @src.type = 'imm' & @src.size = 0
				if value eqtype ''
					type = 's'
				end if
			end if
		end match
		if type = 's'
			fastcall.inline_string value
			type = 'a'
		end if
		if % < 5
			if type = 'f'
				if @src.size = 8 | ~ @src.size | @src.type = 'mmreg'
					if @src.type = 'imm'
						mov rax,value
						movq fastcall.rf#%,rax
					else
						movq fastcall.rf#%,value
					end if
				else if @src.size = 4
					if @src.type = 'imm'
						mov eax,value
						movd fastcall.rf#%,eax
					else
						movd fastcall.rf#%,value
					end if
				else
					err 'invalid argument ',`arg
				end if
			else
				if type = 'a'
					lea fastcall.r#%,[value]
				else
					if @src.size = 8 | ~ @src.size
						redefine target fastcall.r#%
						if @src.type <> 'reg' | ~ @src.imm eq fastcall.r#%
							mov fastcall.r#%,value
						end if
					else if @src.size = 4
						redefine target fastcall.rd#%
						if @src.type <> 'reg' | ~ @src.imm eq fastcall.rd#%
							mov fastcall.rd#%,value
						end if
					else if @src.size = 2
						redefine target fastcall.r#%
						if @src.type <> 'reg' | ~ @src.imm eq fastcall.rw#%
							mov fastcall.rw#%,value
						end if
					else if @src.size = 1
						redefine target fastcall.rb#%
						if @src.type <> 'reg' | ~ @src.imm eq fastcall.rb#%
						end if
					else
						err 'invalid argument ',`arg
					end if
				end if
			end if
		else
			if @src.type = 'reg'
				mov [rsp+offset],value
			else if @src.type = 'mem'
				if type = 'a'
					lea rax,[value]
					mov [rsp+offset],rax
				else
					if @src.size = 8 | ~ @src.size
						mov rax,value
						mov [rsp+offset],rax
					else if @src.size = 4
						mov eax,value
						mov [rsp+offset],eax
					else if @src.size = 2
						mov ax,value
						mov [rsp+offset],ax
					else if @src.size = 1
						mov al,value
						mov [rsp+offset],al
					else
						err 'invalid argument ',`arg
					end if
				end if
			else if @src.type = 'imm'
				if @src.size = 8 | ~ @src.size
					if (value) relativeto 0 & (value) - 1 shl 64 >= -80000000h & (value) < 1 shl 64
						mov rax,(value) - 1 shl 64
						mov [rsp+offset],rax
					else if (value) relativeto 0 & ( (value) >= 80000000h | (value) < -80000000h )
						mov rax,value
						mov [rsp+offset],rax
					else
						mov qword [rsp+offset],value
					end if
				else if @src.size = 4
					mov dword [rsp+offset],value
				else if @src.size = 2
					mov word [rsp+offset],value
				else if @src.size = 1
					mov byte [rsp+offset],value
				else
					err 'invalid argument ',`arg
				end if
			else if type = 'f' & @src.type = 'mmreg' & @src.size = 16
				movq [rsp+offset],value
			else
				err 'invalid argument ',`arg
			end if
		end if
		offset = offset + 8
	end iterate
	pcountcheck proc,offset/8
	if offset < 20h
		offset = 20h
	end if
	framesize = offset + offset and 8
	call proc
	if framesize & fastcall?.frame < 0
		add rsp,framesize
	else if fastcall?.frame >= 0 & framesize > fastcall?.frame
		fastcall?.frame = framesize
	end if
end macro

macro invoke?: proc*,args&
	fastcall [proc],args 
end macro 

macro cinvoke?: proc*,args&
	fastcall [proc],args 
end macro

macro pcountcheck? proc*,args*
end macro

define pcountsuffix %

prologue@proc equ prologuedef

macro prologuedef procname,flag,parmbytes,localbytes,reglist
	local loc,fill
	loc = (localbytes+15) and (not 15)
	parmbase@proc equ rbp+16
	localbase@proc equ rbp-loc
	push rbp
	mov rbp,rsp
	if loc+fill
		sub rsp,loc+fill
	end if
	match any, reglist
		iterate reg, reglist
			push reg
			if % = %%
				fill := 8*(% and 1)
			end if
		end iterate
	else
		fill := 0
	end match
end macro

epilogue@proc equ epiloguedef

macro epiloguedef procname,flag,parmbytes,localbytes,reglist
	iterate reg, reglist
		indx %%-%+1
		pop reg
	end iterate
	leave
	retn
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
							?arg dq ?
						end match
					end iterate
				end match
				parmbytes := $-(parmbase@proc)
				match p, pcountsuffix
					name#p = parmbytes/8
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
				else if length < 8
					load value:dword from area:pointer
					mov dword [pointer],value
					pointer = pointer + 4
					length = length - 4
				else
					load value:qword from area:pointer
					if  value < 80000000h | value >= 1 shl 64 - 80000000h
						mov qword [pointer],value
						pointer = pointer + 8
						length = length - 8
					else
						mov dword [pointer],value and 0FFFFFFFFh
						pointer = pointer + 4
						length = length - 4
					end if
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
						?varname rq count
					else match varname type, arg
						?varname type
					else
						?arg dq ?
					end match
				end iterate
			endl
		end macro

end macro
