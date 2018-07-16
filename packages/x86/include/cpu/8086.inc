
define x86

element x86.reg
element x86.r8	: x86.reg + 1
element x86.r16 : x86.reg + 2

element al? : x86.r8 + 0
element cl? : x86.r8 + 1
element dl? : x86.r8 + 2
element bl? : x86.r8 + 3
element ah? : x86.r8 + 4
element ch? : x86.r8 + 5
element dh? : x86.r8 + 6
element bh? : x86.r8 + 7

element ax? : x86.r16 + 0
element cx? : x86.r16 + 1
element dx? : x86.r16 + 2
element bx? : x86.r16 + 3
element sp? : x86.r16 + 4
element bp? : x86.r16 + 5
element si? : x86.r16 + 6
element di? : x86.r16 + 7

element x86.sreg

element es? : x86.sreg + 0
element cs? : x86.sreg + 1
element ss? : x86.sreg + 2
element ds? : x86.sreg + 3

define x86.byte? :1
define x86.word? :2
define x86.dword? :4

define @dest
define @src

macro x86.parse_address ns,addr
	match seg:offs, addr
		ns.segment = +seg
		if ns.segment eq 1 elementof ns.segment & 1 metadataof ns.segment relativeto x86.sreg
			ns.segment = 1 metadataof ns.segment - x86.sreg
			ns.segment_prefix = 26h + ns.segment shl 3
		else
			err 'invalid operand'
		end if
		ns.address = +offs
		if ns.size = 0
			ns.size = sizeof (offs)
		end if
	else
		ns.address = +addr
		if ns.size = 0
			ns.size = sizeof (addr)
		end if
	end match
	ns.address_registers = 0
	repeat elementsof ns.address
		if % metadataof ns.address relativeto x86.r16
			ns.address_registers = ns.address_registers + % elementof ns.address * % scaleof ns.address
		end if
	end repeat
	ns.displacement = ns.address - ns.address_registers
	if ns.address_registers relativeto 0
		ns.mod = 0
		ns.rm = 6
		ns.displacement_size = 2
	else
		if ns.address_registers relativeto bx+si
			ns.rm = 0
		else if ns.address_registers relativeto bx+di
			ns.rm = 1
		else if ns.address_registers relativeto bp+si
			ns.rm = 2
		else if ns.address_registers relativeto bp+di
			ns.rm = 3
		else if ns.address_registers relativeto si
			ns.rm = 4
		else if ns.address_registers relativeto di
			ns.rm = 5
		else if ns.address_registers relativeto bp
			ns.rm = 6
		else if ns.address_registers relativeto bx
			ns.rm = 7
		else
			err 'invalid address'
		end if
		ns.displacement_size = 2
		ns.mod = 2
		if ns.displacement relativeto 0
			if ns.displacement = 0 & ns.rm <> 6
				ns.displacement_size = 0
				ns.mod = 0
			else if ns.displacement<80h & ns.displacement>=-80h
				ns.displacement_size = 1
				ns.mod = 1
			else if ns.displacement-10000h>=-80h & ns.displacement<10000h
				ns.displacement = ns.displacement-10000h
				ns.displacement_size = 1
				ns.mod = 1
			end if
		end if
	end if
end macro

macro x86.parse_operand_value ns,op
	ns.segment_prefix = 0
	match [addr], op
		ns.type = 'mem'
		x86.parse_address ns,addr
	else match seg:offs, op
		ns.type = 'far'
		if ns.size & ns.size <> 4
			err 'operand sizes do not match'
		else
			ns.size = 4
		end if
		ns.segment = +seg
		ns.offset = +offs
	else
		ns.type = 'imm'
		ns.imm = +op
		ns.displacement_size = 0
		if ns.imm eq 1 elementof ns.imm
			if 1 metadataof (1 metadataof ns.imm) relativeto x86.reg
				ns.type = 'reg'
				ns.mod = 11b
				ns.rm = 1 metadataof ns.imm - 1 elementof (1 metadataof ns.imm)
				if ns.size & ns.size <> 1 metadataof (1 metadataof ns.imm) - x86.reg
					err 'operand sizes do not match'
				else
					ns.size = 1 metadataof (1 metadataof ns.imm) - x86.reg
				end if
			else if 1 metadataof ns.imm relativeto x86.sreg
				ns.type = 'sreg'
				ns.rm = 1 metadataof ns.imm - x86.sreg
				if ns.size & ns.size <> 2
					err 'operand sizes do not match'
				else
					ns.size = 2
				end if
			end if
		end if
	end match
end macro

macro x86.parse_operand ns,op
	ns.size = 0
	match prefix value, op
		match :sz, x86.prefix
			ns.size = sz
			x86.parse_operand_value ns,value
		else
			x86.parse_operand_value ns,op
		end match
	else
		x86.parse_operand_value ns,op
	end match
end macro

macro x86.store_instruction opcode*,rm_operand*,reg*,imm_size:0,imm
	if rm_operand.segment_prefix
		if rm_operand.rm = 2 | rm_operand.rm = 3 | ( rm_operand.mod > 0 & rm_operand.rm = 6 )
			if rm_operand.segment_prefix <> 36h
				db rm_operand.segment_prefix
			end if
		else if rm_operand.segment_prefix <> 3Eh
			db rm_operand.segment_prefix
		end if
	end if
	db opcode, rm_operand.mod shl 6 + reg shl 3 + rm_operand.rm
	if rm_operand.displacement_size = 1
		db rm_operand.displacement
	else if rm_operand.displacement_size = 2
		dw rm_operand.displacement
	end if
	if imm_size = 1
		db imm
	else if imm_size = 2
		dw imm
	end if
end macro

iterate <instr,basecode>, add,0, or,8, adc,10h, sbb,18h, and,20h, sub,28h, xor,30h, cmp,38h
	macro instr? dest*,src*
		x86.parse_operand @dest,dest
		x86.parse_operand @src,src
		local size
		if @dest.size = 0 & @src.size = 0
			err 'operand size not specified'
		else if @dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
			err 'operand sizes do not match'
		else
			size = @dest.size or @src.size
			if size > 2
				err 'invalid operand size'
			end if
		end if
		if @src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
			if defined size & size = 2
				x86.store_instruction basecode+1,@dest,@src.rm
			else
				x86.store_instruction basecode,@dest,@src.rm
			end if
		else if @src.type = 'mem' & @dest.type = 'reg'
			if defined size & size = 2
				x86.store_instruction basecode+3,@src,@dest.rm
			else
				x86.store_instruction basecode+2,@src,@dest.rm
			end if
		else if @src.type = 'imm' & ( @dest.type = 'reg' | @dest.type = 'mem' )
			if defined size & size = 2
				if @src.imm relativeto 0 & @src.imm<80h & @src.imm>=-80h
					x86.store_instruction 83h,@dest,basecode shr 3,1,@src.imm
				else if @src.imm relativeto 0 & @src.imm-10000h>=-80h & @src.imm<10000h
					@src.imm = @src.imm-10000h
					x86.store_instruction 83h,@dest,basecode shr 3,1,@src.imm
				else if @dest.type = 'reg' & @dest.rm = 0
					db basecode+5
					dw @src.imm
				else
					x86.store_instruction 81h,@dest,basecode shr 3,2,@src.imm
				end if
			else
				if @dest.type = 'reg' & @dest.rm = 0
					db basecode+4
					db @src.imm
				else
					x86.store_instruction 80h,@dest,basecode shr 3,1,@src.imm
				end if
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro
end iterate

iterate <instr,postbyte>, not,2, neg,3, mul,4, imul,5, div,6, idiv,7
	macro instr? src*
		x86.parse_operand @src,src
		if @src.size = 0
			err 'operand size not specified'
		else if @src.size > 2
			err 'invalid operand size'
		end if
		if @src.type = 'mem' | @src.type = 'reg'
			if @src.size = 2
				x86.store_instruction 0F7h,@src,postbyte
			else
				x86.store_instruction 0F6h,@src,postbyte
			end if
		else
			err 'invalid operand'
		end if
	end macro
end iterate

macro mov? dest*,src*
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	local size
	if @dest.size = 0 & @src.size = 0
		err 'operand size not specified'
	else if @dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
		err 'operand sizes do not match'
	else
		size = @dest.size or @src.size
		if size > 2
			err 'invalid operand size'
		end if
	end if
	if @src.type = 'reg' & @dest.type = 'mem' & @src.rm = 0 & @dest.address_registers eq 0
		if @dest.segment_prefix & @dest.segment_prefix <> 3Eh
			db @dest.segment_prefix
		end if
		if defined size & size = 2
			db 0A3h
			dw @dest.address
		else
			db 0A2h
			dw @dest.address
		end if
	else if @src.type = 'mem' & @dest.type = 'reg' & @dest.rm = 0 & @src.address_registers eq 0
		if @src.segment_prefix & @src.segment_prefix <> 3Eh
			db @src.segment_prefix
		end if
		if defined size & size = 2
			db 0A1h
			dw @src.address
		else
			db 0A0h
			dw @src.address
		end if
	else if @src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		if defined size & size = 2
			x86.store_instruction 89h,@dest,@src.rm
		else
			x86.store_instruction 88h,@dest,@src.rm
		end if
	else if @src.type = 'mem' & @dest.type = 'reg'
		if defined size & size = 2
			x86.store_instruction 8Bh,@src,@dest.rm
		else
			x86.store_instruction 8Ah,@src,@dest.rm
		end if
	else if @src.type = 'imm' & @dest.type = 'mem'
		if defined size & size = 2
			x86.store_instruction 0C7h,@dest,0,2,@src.imm
		else
			x86.store_instruction 0C6h,@dest,0,1,@src.imm
		end if
	else if @src.type = 'imm' & @dest.type = 'reg'
		if defined size & size = 2
			db 0B8h + @dest.rm
			dw @src.imm
		else
			db 0B0h + @dest.rm
			db @src.imm
		end if
	else if @src.type = 'sreg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		x86.store_instruction 8Ch,@dest,@src.rm
	else if @dest.type = 'sreg' & @dest.rm <> 1 & ( @src.type = 'reg' | @src.type = 'mem' )
		x86.store_instruction 8Eh,@src,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

macro test? dest*,src*
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	local size
	if @dest.size = 0 & @src.size = 0
		err 'operand size not specified'
	else if @dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
		err 'operand sizes do not match'
	else
		size = @dest.size or @src.size
		if size > 2
			err 'invalid operand size'
		end if
	end if
	if @src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		if defined size & size = 2
			x86.store_instruction 85h,@dest,@src.rm
		else
			x86.store_instruction 84h,@dest,@src.rm
		end if
	else if @src.type = 'mem' & @dest.type = 'reg'
		if defined size & size = 2
			x86.store_instruction 85h,@src,@dest.rm
		else
			x86.store_instruction 84h,@src,@dest.rm
		end if
	else if @src.type = 'imm' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		if defined size & size = 2
			if @dest.type = 'reg' & @dest.rm = 0
				db 0A9h
				dw @src.imm
			else
				x86.store_instruction 0F7h,@dest,0,2,@src.imm
			end if
		else
			if @dest.type = 'reg' & @dest.rm = 0
				db 0A8h
				db @src.imm
			else
				x86.store_instruction 0F6h,@dest,0,1,@src.imm
			end if
		end if
	else
		err 'invalid combination of operands'
	end if
end macro

macro xchg? dest*,src*
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	local size
	if @dest.size = 0 & @src.size = 0
		err 'operand size not specified'
	else if @dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
		err 'operand sizes do not match'
	else
		size = @dest.size or @src.size
		if size > 2
			err 'invalid operand size'
		end if
	end if
	if @src.type = 'reg' & @dest.type = 'reg'
		if defined size & size = 2
			if @src.rm = 0 | @dest.rm = 0
				db 90h + @src.rm + @dest.rm
			else
				x86.store_instruction 87h,@src,@dest.rm
			end if
		else
			x86.store_instruction 86h,@src,@dest.rm
		end if
	else if @src.type = 'reg' & @dest.type = 'mem'
		if defined size & size = 2
			x86.store_instruction 87h,@dest,@src.rm
		else
			x86.store_instruction 86h,@dest,@src.rm
		end if
	else if @src.type = 'mem' & @dest.type = 'reg'
		if defined size & size = 2
			x86.store_instruction 87h,@src,@dest.rm
		else
			x86.store_instruction 86h,@src,@dest.rm
		end if
	else
		err 'invalid combination of operands'
	end if
end macro

iterate instr, inc,dec
	repeat 1 postbyte:%-1
		macro instr? dest*
			x86.parse_operand @dest,dest
			if @dest.size = 0
				err 'operand size not specified'
			else if @dest.size > 2
				err 'invalid operand size'
			end if
			if @dest.type = 'mem'
				if @dest.size = 2
					x86.store_instruction 0FFh,@dest,postbyte
				else
					x86.store_instruction 0FEh,@dest,postbyte
				end if
			else if @dest.type = 'reg'
				if @dest.size = 2
					db 40h + @dest.rm + postbyte shl 3
				else
					x86.store_instruction 0FEh,@dest,postbyte
				end if
			else
				err 'invalid operand'
			end if
		end macro
	end repeat
end iterate

macro push? src*
	x86.parse_operand @src,src
	if @src.size <> 0 & @src.size <> 2
		err 'invalid operand size'
	end if
	if @src.type = 'mem'
		x86.store_instruction 0FFh,@src,110b
	else if @src.type = 'reg'
		db 50h + @src.rm
	else if @src.type = 'sreg'
		db 6 + @src.rm shl 3
	else
		err 'invalid operand'
	end if
end macro

macro pop? dest*
	x86.parse_operand @dest,dest
	if @dest.size <> 0 & @dest.size <> 2
		err 'invalid operand size'
	end if
	if @dest.type = 'mem'
		x86.store_instruction 8Fh,@dest,0
	else if @dest.type = 'reg'
		db 58h + @dest.rm
	else if @dest.type = 'sreg'
		db 7 + @dest.rm shl 3
	else
		err 'invalid operand'
	end if
end macro

macro ret? operand
	match imm, operand
		db 0C2h
		dw imm
	else
		db 0C3h
	end match
end macro

macro retn? operand
	match imm, operand
		db 0C2h
		dw imm
	else
		db 0C3h
	end match
end macro

macro retf? operand
	match imm, operand
		db 0CAh
		dw imm
	else
		db 0CBh
	end match
end macro

macro lea? dest*,src*
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	if @dest.size <> 0 & @dest.size <> 2
		err 'invalid operand size'
	end if
	if @src.type = 'mem' & @dest.type = 'reg'
		x86.store_instruction 8Dh,@src,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

iterate instr, les,lds
	repeat 1 basecode:0C4h+%-1
		macro instr? dest*,src*
			x86.parse_operand @dest,dest
			x86.parse_operand @src,src
			if ( @dest.size <> 0 & @dest.size <> 2 ) | ( @src.size <> 0 & @src.size <> 4 )
				err 'invalid operand size'
			end if
			if @src.type = 'mem' & @dest.type = 'reg'
				x86.store_instruction basecode,@src,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		end macro
	end repeat
end iterate

macro x86.shift_instruction opcode,dest,cnt
	x86.parse_operand @dest,dest
	x86.parse_operand @src,cnt
	if @dest.size = 0
		err 'operand size not specified'
	else if @dest.size > 2
		err 'invalid operand size'
	end if
	if @src.size <> 0 & @src.size <> 1
		err 'invalid operand size'
	end if
	if @src.type = 'reg' & @src.size = 1 & @src.rm = 1 & ( @dest.type = 'reg' | @dest.type = 'mem' )
		if @dest.size = 2
			x86.store_instruction 0D3h,@dest,opcode
		else
			x86.store_instruction 0D2h,@dest,opcode
		end if
	else if @src.type = 'imm' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		if @dest.size = 2
			repeat @src.imm
				x86.store_instruction 0D1h,@dest,opcode
			end repeat
		else
			repeat @src.imm
				x86.store_instruction 0D0h,@dest,opcode
			end repeat
		end if
	else
		err 'invalid combination of operands'
	end if
end macro

macro rol? dest*,cnt*
	x86.shift_instruction 0,dest,cnt
end macro

macro ror? dest*,cnt*
	x86.shift_instruction 1,dest,cnt
end macro

macro rcl? dest*,cnt*
	x86.shift_instruction 2,dest,cnt
end macro

macro rcr? dest*,cnt*
	x86.shift_instruction 3,dest,cnt
end macro

macro shl? dest*,cnt*
	x86.shift_instruction 4,dest,cnt
end macro

macro sal? dest*,cnt*
	x86.shift_instruction 4,dest,cnt
end macro

macro shr? dest*,cnt*
	x86.shift_instruction 5,dest,cnt
end macro

macro sar? dest*,cnt*
	x86.shift_instruction 7,dest,cnt
end macro

macro x86.parse_jump_operand ns,op
	match =far? dest, op
		x86.parse_operand ns,dest
		ns.jump_type = 'far'
	else match =near? dest, op
		x86.parse_operand ns,dest
		ns.jump_type = 'near'
	else match =short? dest, op
		x86.parse_operand ns,dest
		ns.jump_type = 'short'
	else
		x86.parse_operand ns,op
		ns.jump_type = ''
	end match
end macro

macro call? dest*
	x86.parse_jump_operand @dest,dest
	if @dest.type = 'far'
		if @dest.jump_type & @dest.jump_type <> 'far'
			err 'invalid operand'
		end if
		if @dest.size <> 0 & @dest.size <> 4
			err 'invalid operand size'
		end if
		db 9Ah
		dw @dest.offset,@dest.segment
	else if @dest.type = 'mem' | @dest.type = 'reg'
		if @dest.size = 4
			if @dest.jump_type & @dest.jump_type <> 'far'
				err 'invalid operand'
			end if
			x86.store_instruction 0FFh,@dest,11b
		else if @dest.size = 2
			if @dest.jump_type & @dest.jump_type <> 'near'
				err 'invalid operand'
			end if
			x86.store_instruction 0FFh,@dest,10b
		else if @dest.size = 0
			err 'operand size not specified'
		else
			err 'invalid operand size'
		end if
	else if @dest.type = 'imm'
		if @dest.jump_type & @dest.jump_type <> 'near'
			err 'invalid operand'
		end if
		if @dest.imm relativeto 0 & (@dest.imm < 0 | @dest.imm >= 10000h)
			err 'value out of range'
		end if
		db 0E8h
		dw @dest.imm-($+2)
	else
		err 'invalid operand'
	end if
end macro

macro jmp? dest*
	x86.parse_jump_operand @dest,dest
	if @dest.type = 'far'
		if @dest.jump_type & @dest.jump_type <> 'far'
			err 'invalid operand'
		end if
		if @dest.size <> 0 & @dest.size <> 4
			err 'invalid operand size'
		end if
		db 0EAh
		dw @dest.offset,@dest.segment
	else if @dest.type = 'mem' | @dest.type = 'reg'
		if @dest.size = 4
			if @dest.jump_type & @dest.jump_type <> 'far'
				err 'invalid operand'
			end if
			x86.store_instruction 0FFh,@dest,101b
		else if @dest.size = 2
			if @dest.jump_type & @dest.jump_type <> 'near'
				err 'invalid operand'
			end if
			x86.store_instruction 0FFh,@dest,100b
		else if @dest.size = 0
			err 'operand size not specified'
		else
			err 'invalid operand size'
		end if
	else if @dest.type = 'imm'
		if @dest.imm relativeto 0 & (@dest.imm < 0 | @dest.imm >= 10000h)
			err 'value out of range'
		end if
		if @dest.jump_type = 'near'
			db 0E9h
			dw @dest.imm-($+2)
		else if @dest.jump_type = 'short'
			db 0EBh
			if @dest.imm-($+1)<80h & @dest.imm-($+1)>=-80h
				db @dest.imm-($+1)
			else
				err 'relative jump out of range'
				db ?
			end if
		else if ~ @dest.jump_type
			if @dest.imm relativeto $ & @dest.imm-($+2)<80h & @dest.imm-($+2)>=-80h
				db 0EBh
				db @dest.imm-($+1)
			else
				db 0E9h
				dw @dest.imm-($+2)
			end if
		else
			err 'invalid operand'
		end if
	else
		err 'invalid operand'
	end if
end macro

iterate <instr,opcode>, jo,70h, jno,71h, jc,72h, jb,72h, jnae,72h, jnc,73h, jnb,73h, jae,73h, jz,74h, je,74h, jnz,75h, jne,75h, jna,76h, jbe,76h, ja,77h, jnbe,77h, \
			js,78h, jns,79h, jp,7Ah, jpe,7Ah, jnp,7Bh, jpo,7Bh, jl,7Ch, jnge,7Ch, jnl,7Dh, jge,7Dh, jng,7Eh, jle,7Eh, jg,7Fh, jnle,7Fh
	macro instr? dest*
		x86.parse_jump_operand @dest,dest
		if @dest.type = 'imm' & ( @dest.jump_type = 'short' | ~ @dest.jump_type )
			if @dest.imm relativeto 0 & (@dest.imm < 0 | @dest.imm >= 10000h)
				err 'value out of range'
			end if
			if @dest.imm-($+2)<80h & @dest.imm-($+2)>=-80h
				db opcode
				db @dest.imm-($+1)
			else
				if x86.jumps & ~ @dest.jump_type
					local skip
					db opcode xor 1
					db skip-($+1)
					jmp dest
					skip = $
				else
					err 'relative jump out of range'
					db ?
				end if
			end if
		else
			err 'invalid operand'
		end if
	end macro
end iterate

macro jumps?
	x86.jumps = 1
end macro

macro nojumps?
	x86.jumps = 0
end macro

x86.jumps = 0

macro x86.short_jump opcode,dest
	x86.parse_jump_operand @dest,dest
	if @dest.type = 'imm' & ( @dest.jump_type = 'short' | ~ @dest.jump_type )
		if @dest.imm relativeto 0 & (@dest.imm < 0 | @dest.imm >= 10000h)
			err 'value out of range'
		end if
		db opcode
		if @dest.imm-($+1)<80h & @dest.imm-($+1)>=-80h
			db @dest.imm-($+1)
		else
			err 'relative jump out of range'
			db ?
		end if
	else
		err 'invalid operand'
	end if
end macro

macro loopnz? dest*
	x86.short_jump 0E0h,dest
end macro

macro loopne? dest*
	x86.short_jump 0E0h,dest
end macro

macro loopz? dest*
	x86.short_jump 0E1h,dest
end macro

macro loope? dest*
	x86.short_jump 0E1h,dest
end macro

macro loop? dest*
	x86.short_jump 0E2h,dest
end macro

macro jcxz? dest*
	x86.short_jump 0E3h,dest
end macro

macro daa?
	db 27h
end macro

macro das?
	db 2Fh
end macro

macro aaa?
	db 37h
end macro

macro aas?
	db 3Fh
end macro

macro nop?
	db 90h
end macro

macro cbw?
	db 98h
end macro

macro cwd?
	db 99h
end macro

macro int3?
	db 0CCh
end macro

macro int? number*
	db 0CDh,number
end macro

macro into?
	db 0CEh
end macro

macro iret?
	db 0CFh
end macro

macro aam? number:10
	db 0D4h,number
end macro

macro aad? number:10
	db 0D5h,number
end macro

macro salc?
	db 0D6h
end macro

macro repnz? instr&
	db 0F2h
	instr
end macro

macro repne? instr&
	db 0F2h
	instr
end macro

macro rep? instr&
	db 0F3h
	instr
end macro

macro repz? instr&
	db 0F3h
	instr
end macro

macro repe? instr&
	db 0F3h
	instr
end macro

macro hlt?
	db 0F4h
end macro

macro cmc?
	db 0F5h
end macro

macro clc?
	db 0F8h
end macro

macro stc?
	db 0F9h
end macro

macro cli?
	db 0FAh
end macro

macro sti?
	db 0FBh
end macro

macro cld?
	db 0FCh
end macro

macro std?
	db 0FDh
end macro

macro pushf?
	db 9Ch
end macro

macro popf?
	db 9Dh
end macro

macro sahf?
	db 9Eh
end macro

macro lahf?
	db 9Fh
end macro

macro movsb?
	db 0A4h
end macro

macro movsw?
	db 0A5h
end macro

macro cmpsb?
	db 0A6h
end macro

macro cmpsw?
	db 0A7h
end macro

macro stosb?
	db 0AAh
end macro

macro stosw?
	db 0ABh
end macro

macro lodsb?
	db 0ACh
end macro

macro lodsw?
	db 0ADh
end macro

macro scasb?
	db 0AEh
end macro

macro scasw?
	db 0AFh
end macro

macro xlatb?
	db 0D7h
end macro

macro in? dest*,src*
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	if @dest.size = 0
		err 'operand size not specified'
	else if @dest.size > 2
		err 'invalid operand size'
	end if
	if @src.type = 'reg' & @src.size = 2 & @src.rm = 2 & @dest.type = 'reg' & @dest.rm = 0
		if @dest.size = 2
			db 0EDh
		else
			db 0ECh
		end if
	else if @src.type = 'imm' & @dest.type = 'reg' & @dest.rm = 0
		if @dest.size = 2
			db 0E5h,@src.imm
		else
			db 0E4h,@src.imm
		end if
	else
		err 'invalid combination of operands'
	end if
end macro

macro out? dest*,src*
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	if @src.size = 0
		err 'operand size not specified'
	else if @src.size > 2
		err 'invalid operand size'
	end if
	if @dest.type = 'reg' & @dest.size = 2 & @dest.rm = 2 & @src.type = 'reg' & @src.rm = 0
		if @src.size = 2
			db 0EFh
		else
			db 0EEh
		end if
	else if @dest.type = 'imm' & @src.type = 'reg' & @src.rm = 0
		if @src.size = 2
			db 0E7h,@dest.imm
		else
			db 0E6h,@dest.imm
		end if
	else
		err 'invalid combination of operands'
	end if
end macro

macro movs? dest*,src*
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	local size
	if @dest.size = 0 & @src.size = 0
		err 'operand size not specified'
	else if @dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
		err 'operand sizes do not match'
	else
		size = @dest.size or @src.size
		if size > 2
			err 'invalid operand size'
		end if
	end if
	if @src.type = 'mem' & @src.mod = 0 & @src.rm = 4 & @dest.type = 'mem' & @dest.mod = 0 & @dest.rm = 5 & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h )
		if @src.segment_prefix & @src.segment_prefix <> 3Eh
			db @src.segment_prefix
		end if
		if defined size & size = 2
			db 0A5h
		else
			db 0A4h
		end if
	else
		err 'invalid operand'
	end if
end macro

macro cmps? src*,dest*
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	local size
	if @dest.size = 0 & @src.size = 0
		err 'operand size not specified'
	else if @dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
		err 'operand sizes do not match'
	else
		size = @dest.size or @src.size
		if size > 2
			err 'invalid operand size'
		end if
	end if
	if @src.type = 'mem' & @src.mod = 0 & @src.rm = 4 & @dest.type = 'mem' & @dest.mod = 0 & @dest.rm = 5 & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h )
		if @src.segment_prefix & @src.segment_prefix <> 3Eh
			db @src.segment_prefix
		end if
		if defined size & size = 2
			db 0A7h
		else
			db 0A6h
		end if
	else
		err 'invalid operand'
	end if
end macro

macro stos? dest*
	x86.parse_operand @dest,dest
	if @dest.size = 0
		err 'operand size not specified'
	else if @dest.size > 2
		err 'invalid operand size'
	end if
	if @dest.type = 'mem' & @dest.mod = 0 & @dest.rm = 5 & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h )
		if @dest.size = 2
			db 0ABh
		else
			db 0AAh
		end if
	else
		err 'invalid operand'
	end if
end macro

macro lods? src*
	x86.parse_operand @src,src
	if @src.size = 0
		err 'operand size not specified'
	else if @src.size > 2
		err 'invalid operand size'
	end if
	if @src.type = 'mem' & @src.mod = 0 & @src.rm = 4
		if @src.segment_prefix & @src.segment_prefix <> 3Eh
			db @src.segment_prefix
		end if
		if @src.size = 2
			db 0ADh
		else
			db 0ACh
		end if
	else
		err 'invalid operand'
	end if
end macro

macro scas? dest*
	x86.parse_operand @dest,dest
	if @dest.size = 0
		err 'operand size not specified'
	else if @dest.size > 2
		err 'invalid operand size'
	end if
	if @dest.type = 'mem' & @dest.mod = 0 & @dest.rm = 5 & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h )
		if @dest.size = 2
			db 0AFh
		else
			db 0AEh
		end if
	else
		err 'invalid operand'
	end if
end macro

macro xlat? src*
	x86.parse_operand @src,src
	if @src.size > 1
		err 'invalid operand size'
	end if
	if @src.type = 'mem' & @src.mod = 0 & @src.rm = 7
		if @src.segment_prefix & @src.segment_prefix <> 3Eh
			db @src.segment_prefix
		end if
		db 0D7h
	else
		err 'invalid operand'
	end if
end macro
