
define x86

element x86.reg
element x86.r8	: x86.reg + 1
element x86.r16 : x86.reg + 2
element x86.r32 : x86.reg + 4
element x86.r64 : x86.reg + 8

element al? : x86.r8 + 0
element cl? : x86.r8 + 1
element dl? : x86.r8 + 2
element bl? : x86.r8 + 3

element spl? : x86.r8 + 4
element bpl? : x86.r8 + 5
element sil? : x86.r8 + 6
element dil? : x86.r8 + 7

element ah? : x86.r8 - 4
element ch? : x86.r8 - 5
element dh? : x86.r8 - 6
element bh? : x86.r8 - 7

repeat 8, i:8
	element r#i#b? : x86.r8 + i
	element r#i#l? : x86.r8 + i
end repeat

element ax? : x86.r16 + 0
element cx? : x86.r16 + 1
element dx? : x86.r16 + 2
element bx? : x86.r16 + 3
element sp? : x86.r16 + 4
element bp? : x86.r16 + 5
element si? : x86.r16 + 6
element di? : x86.r16 + 7

repeat 8, i:8
	element r#i#w? : x86.r16 + i
end repeat

element eax? : x86.r32 + 0
element ecx? : x86.r32 + 1
element edx? : x86.r32 + 2
element ebx? : x86.r32 + 3
element esp? : x86.r32 + 4
element ebp? : x86.r32 + 5
element esi? : x86.r32 + 6
element edi? : x86.r32 + 7

repeat 8, i:8
	element r#i#d? : x86.r32 + i
end repeat

element rax? : x86.r64 + 0
element rcx? : x86.r64 + 1
element rdx? : x86.r64 + 2
element rbx? : x86.r64 + 3
element rsp? : x86.r64 + 4
element rbp? : x86.r64 + 5
element rsi? : x86.r64 + 6
element rdi? : x86.r64 + 7

repeat 8, i:8
	element r#i? : x86.r64 + i
end repeat

element x86.ip

element eip? : x86.ip + 4
element rip? : x86.ip + 8

element x86.sreg

element es? : x86.sreg + 0
element cs? : x86.sreg + 1
element ss? : x86.sreg + 2
element ds? : x86.sreg + 3
element fs? : x86.sreg + 4
element gs? : x86.sreg + 5

element x86.creg

element x86.crx : x86.creg + 0
element x86.drx : x86.creg + 1

repeat 16, i:0
	element cr#i? : x86.crx + i
	element dr#i? : x86.drx + i
end repeat

define x86.byte? :1
define x86.word? :2
define x86.dword? :4
define x86.pword? :6
define x86.fword? :6
define x86.qword? :8
define x86.tword? :10
define x86.tbyte? :10
define x86.dqword? :16

x86.mode = 16

macro use16?
	x86.mode = 16
end macro

macro use32?
	x86.mode = 32
end macro

macro use64?
	x86.mode = 64
end macro

x86.REX_REQUIRED = 100h
x86.REX_FORBIDDEN = 200h

define @dest
define @src
define @aux

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

macro x86.parse_operand_value ns,op
	ns.segment_prefix = 0
	ns.prefix = 0
	ns.opcode_prefix = 0
	ns.rex_prefix = 0
	match [addr], op
		ns.type = 'mem'
		match seg:offs, addr
			x86.parse_segment_prefix ns,seg
			x86.parse_address ns,offs
		else
			x86.parse_address ns,addr
		end match
	else match =ptr? addr, op
		ns.type = 'mem'
		match seg:offs, addr
			x86.parse_segment_prefix ns,seg
			x86.parse_address ns,offs
		else
			x86.parse_address ns,addr
		end match
	else match seg:offs, op
		ns.type = 'far'
		if ns.size & ns.size <> 4 & ns.size <> 6 & ns.size <> 10
			err 'operand sizes do not match'
		end if
		ns.segment = +seg
		ns.offset = +offs
	else
		ns.type = 'imm'
		ns.imm = +op
		if defined op
			ns.unresolved = 0
		else
			ns.unresolved = 1
		end if
		ns.displacement_size = 0
		if ns.imm eq 1 elementof ns.imm
			if 1 metadataof (1 metadataof ns.imm) relativeto x86.reg
				ns.type = 'reg'
				ns.mode = x86.mode
				ns.mod = 11b
				ns.rm = 1 metadataof ns.imm - 1 elementof (1 metadataof ns.imm)
				if ns.size & ns.size <> 1 metadataof (1 metadataof ns.imm) - x86.reg
					err 'operand sizes do not match'
				else
					ns.size = 1 metadataof (1 metadataof ns.imm) - x86.reg
					if ns.rm < 0
						ns.rm = x86.REX_FORBIDDEN - ns.rm
					else if ns.size = 1 & ns.rm >= 4 & ns.rm < 8
						ns.rm = x86.REX_REQUIRED + ns.rm
					end if
				end if
			else if 1 metadataof ns.imm relativeto x86.sreg
				ns.type = 'sreg'
				ns.rm = 1 metadataof ns.imm - x86.sreg
				if ns.size <> 0 & ns.size <> 2 & ns.size <> 4
					err 'invalid operand size'
				end if
			end if
		end if
	end match
end macro

macro x86.parse_segment_prefix ns,seg
	ns.segment = +seg
	if ns.segment eq 1 elementof ns.segment & 1 metadataof ns.segment relativeto x86.sreg
		ns.segment = 1 metadataof ns.segment - x86.sreg
		if ns.segment < 4
			ns.segment_prefix = 26h + ns.segment shl 3
		else
			ns.segment_prefix = 64h + ns.segment-4
		end if
	else
		err 'invalid operand'
	end if
end macro

macro x86.parse_address ns,addr
	ns.mode = 0
	match :sz value, x86.addr
		if sz = 2 | sz = 4 | sz = 8
			ns.mode = sz shl 3
		else
			err 'invalid address size'
		end if
		ns.address = +value
		if ns.size = 0
			ns.size = sizeof (value)
		end if
	else
		ns.address = +addr
		if ns.size = 0
			ns.size = sizeof (addr)
		end if
	end match
	ns.address_registers = 0
	repeat elementsof ns.address
		if % metadataof ns.address relativeto x86.r16 | % metadataof ns.address relativeto x86.r32 | % metadataof ns.address relativeto x86.r64 | % metadataof ns.address relativeto x86.ip
			ns.address_registers = ns.address_registers + % elementof ns.address * % scaleof ns.address
		end if
	end repeat
	ns.displacement = ns.address - ns.address_registers
	ns.auto_relative = 0
	if ns.address_registers eq 0
		x86.encode_direct_address ns
	else
		if ns.mode & ns.mode <> 0 scaleof (1 metadataof (1 metadataof ns.address_registers)) shl 3 & ~ 1 metadataof ns.address_registers relativeto x86.ip
			err 'invalid address'
		end if
		if 1 metadataof ns.address_registers relativeto x86.r64 | 1 metadataof ns.address_registers relativeto x86.r32
			x86.encode_address_32_64 ns
		else if 1 metadataof ns.address_registers relativeto x86.r16
			x86.encode_address_16 ns
		else if ns.address_registers eq rip | ns.address_registers eq eip
			ns.mode = 0 scaleof (1 metadataof ns.address_registers) shl 3
			ns.mod = 0
			ns.rm = 5
			ns.displacement_size = 4
		else
			err 'invalid address'
		end if
	end if
end macro

macro x86.encode_direct_address ns
	ns.mod = 0
	if x86.mode = 64
		ns.displacement_size = 4
		if ns.mode = 0
			ns.mode = 64
			ns.rm = 5
			ns.auto_relative = 1
		else if ns.mode > 16
			ns.rm = 4
			ns.base = 5
			ns.index = 4
			ns.scale = 1
			if ns.mode = 64
				ns.displacement_size = 8
			else if ns.displacement relativeto 0
				if ns.displacement >= 100000000h | ns.displacement < -100000000h
					err 'address out of range'
				end if
				ns.displacement = ns.displacement and 0FFFFFFFFh
			end if
			if ~ ns.displacement relativeto 0 | ns.displacement < 80000000h | ns.displacement >= 100000000h
				ns.mode = 64
			else
				ns.mode = 32
			end if
		else
			err 'invalid size of address value'
		end if
	else
		if ns.mode = 0
			ns.mode = x86.mode
			if ns.mode = 16 & ns.displacement relativeto 0 & ns.displacement >= 10000h
				ns.mode = 32
			end if
		end if
		if ns.mode = 16
			ns.rm = 6
			ns.displacement_size = 2
		else
			ns.rm = 5
			ns.displacement_size = 4
		end if
	end if
end macro

macro x86.encode_address_16 ns
	ns.mode = 16
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
end macro

macro x86.encode_address_32_64 ns
	if 1 metadataof ns.address_registers relativeto x86.r64
		ns.mode = 64
		ns.address_registers_type = x86.r64
	else
		ns.mode = 32
		ns.address_registers_type = x86.r32
	end if
	ns.index_only = 0
	if 2 scaleof ns.address_registers = 0
		ns.scale = 1 scaleof ns.address_registers
		ns.base = 1 metadataof ns.address_registers - ns.address_registers_type
		if ns.scale = 1
			if ns.base and 111b = 4
				ns.rm = 4
				ns.index = 4
			else
				ns.rm = ns.base
				ns.index = -1
			end if
		else if ns.base <> 4 & ns.scale = 2
			ns.rm = 4
			ns.index = ns.base
			ns.scale = 1
		else if ns.base <> 4 & (ns.scale = 4 | ns.scale = 8)
			ns.rm = 4
			ns.index = ns.base
			ns.base = 5
			ns.index_only = 1
		else if ns.base <> 4 & (ns.scale = 3 | ns.scale = 5 | ns.scale = 9)
			ns.rm = 4
			ns.index = ns.base
			ns.scale = ns.scale - 1
		else
			err 'invalid address'
		end if
	else if 3 scaleof ns.address_registers = 0 & 2 metadataof ns.address_registers relativeto ns.address_registers_type
		ns.rm = 4
		if 1 scaleof ns.address_registers = 1
			ns.base = 1 metadataof ns.address_registers - ns.address_registers_type
			ns.index = 2 metadataof ns.address_registers - ns.address_registers_type
			ns.scale = 2 scaleof ns.address_registers
		else if 2 scaleof ns.address_registers = 1
			ns.base = 2 metadataof ns.address_registers - ns.address_registers_type
			ns.index = 1 metadataof ns.address_registers - ns.address_registers_type
			ns.scale = 1 scaleof ns.address_registers
		else
			err 'invalid address'
		end if
		if ns.index = 4
			if ns.scale = 1
				ns.index = ns.base
				ns.base = 4
			else
				err 'invalid address'
			end if
		else if (x86.mode <> 64 & ns.segment_prefix = 36h) & ns.index = 5 & ns.scale = 1
			ns.index = ns.base
			ns.base = 5
		else if (x86.mode = 64 | ns.segment_prefix = 3Eh) & ns.base = 5 & ns.scale = 1
			ns.base = ns.index
			ns.index = 5
		else if ns.scale > 2 & ns.scale <> 4 & ns.scale <> 8
			err 'invalid address'
		end if
	else
		err 'invalid address'
	end if
	ns.displacement_size = 4
	ns.mod = 2
	if ns.index_only
		ns.mod = 0
	else if ns.displacement relativeto 0
		if ns.displacement = 0 & ns.rm and 111b <> 5 & (ns.rm <> 4 | ns.base and 111b <> 5)
			ns.displacement_size = 0
			ns.mod = 0
		else if ns.displacement < 80h & ns.displacement >= -80h
			ns.displacement_size = 1
			ns.mod = 1
		else if ns.displacement - 1 shl ns.mode >= -80h & ns.displacement < 1 shl ns.mode
			ns.displacement = ns.displacement - 1 shl ns.mode
			ns.displacement_size = 1
			ns.mod = 1
		else if (x86.mode = 64 | ns.segment_prefix = 3Eh) & ns.base = 5 & ns.index = 5 & ns.scale = 1
			ns.scale = 2
			ns.mod = 0
		end if
	end if
end macro

macro x86.select_operand_prefix rm_operand*,size*
	if (size = 2 & x86.mode <> 16) | (size = 4 & x86.mode = 16)
		rm_operand.prefix = 66h
	else if size = 8
		rm_operand.prefix = 48h
	else if size <> 0 & size <> 2 & size <> 4 & size <> 8
		err 'invalid operand size'
	end if
end macro

macro x86.store_operand_prefix size*,reg:0
	x86.rex_prefix = 0
	if (size = 2 & x86.mode <> 16) | (size = 4 & x86.mode = 16)
		db 66h
	else if size = 8
		x86.rex_prefix = 48h
	else if size <> 0 & size <> 2 & size <> 4 & size <> 8
		err 'invalid operand size'
	end if
	if reg and 1000b
		x86.rex_prefix = x86.rex_prefix or 41h
	else if reg and x86.REX_REQUIRED
		x86.rex_prefix = x86.rex_prefix or 40h
	end if
	if x86.rex_prefix
		if x86.mode < 64
			err 'instruction requires long mode'
		else if reg and x86.REX_FORBIDDEN
			err 'disallowed combination of registers'
		end if
		db x86.rex_prefix
	end if
end macro

macro x86.store_instruction opcode*,rm_operand*,reg*,imm_size:0,imm
	if rm_operand.segment_prefix
		if x86.mode = 64
			if rm_operand.segment_prefix >= 64h
				db rm_operand.segment_prefix
			end if
		else if rm_operand.mode = 16 & ( rm_operand.rm = 2 | rm_operand.rm = 3 | ( rm_operand.mod > 0 & rm_operand.rm = 6 ) )
			if rm_operand.segment_prefix <> 36h
				db rm_operand.segment_prefix
			end if
		else if rm_operand.mode = 32 & ( ( rm_operand.mod > 0 & rm_operand.rm = 5 ) | ( rm_operand.rm = 4 & rm_operand.base = 4 ) | ( rm_operand.mod > 0 & rm_operand.rm = 4 & rm_operand.base = 5 ) )
			if rm_operand.segment_prefix <> 36h
				db rm_operand.segment_prefix
			end if
		else if rm_operand.segment_prefix <> 3Eh
			db rm_operand.segment_prefix
		end if
	end if
	if rm_operand.mod <> 11b & rm_operand.mode <> x86.mode
		if rm_operand.mode = 64 | (rm_operand.mode = 16 & x86.mode = 64)
			err 'illegal addressing mode'
		end if
		db 67h
	end if
	if (reg or rm_operand.rm) and x86.REX_REQUIRED
		rm_operand.rex_prefix = rm_operand.rex_prefix or 40h
	end if
	if rm_operand.rm and 1000b | (rm_operand.mod <> 11b & rm_operand.mode > 16 & rm_operand.rm = 4 & rm_operand.base and 1000b)
		rm_operand.rex_prefix = rm_operand.rex_prefix or 41h
	end if
	if rm_operand.mod <> 11b & rm_operand.mode > 16 & rm_operand.rm = 4 & rm_operand.index and 1000b
		rm_operand.rex_prefix = rm_operand.rex_prefix or 42h
	end if
	if reg and 1000b
		rm_operand.rex_prefix = rm_operand.rex_prefix or 44h
	end if
	if rm_operand.prefix
		if rm_operand.prefix = 48h
			rm_operand.rex_prefix = rm_operand.rex_prefix or 48h
		else
			db rm_operand.prefix
		end if
	end if
	if rm_operand.opcode_prefix
		db rm_operand.opcode_prefix
	end if
	if rm_operand.rex_prefix
		if x86.mode < 64
			err 'instruction requires long mode'
		else if (reg or rm_operand.rm) and x86.REX_FORBIDDEN
			err 'disallowed combination of registers'
		end if
		db rm_operand.rex_prefix
	end if
	db opcode, rm_operand.mod shl 6 + (reg and 111b) shl 3 + rm_operand.rm and 111b
	if rm_operand.mod <> 11b & rm_operand.rm = 4 & rm_operand.mode <> 16
		db (bsf rm_operand.scale) shl 6 + (rm_operand.index and 111b) shl 3 + (rm_operand.base and 111b)
	end if
	if rm_operand.displacement_size = 1
		db rm_operand.displacement
	else if rm_operand.displacement_size = 2
		dw rm_operand.displacement
	else if rm_operand.displacement_size = 4 | rm_operand.displacement_size = 8
		if rm_operand.auto_relative
			if imm_size < 8
				rm_operand.displacement = rm_operand.displacement - ($ + 4 + imm_size)
			else
				rm_operand.displacement = rm_operand.displacement - ($ + 4 + 4)
			end if
		end if
		if rm_operand.mode = 64 & rm_operand.displacement relativeto 0
			if rm_operand.displacement - 1 shl 64 >= -80000000h & rm_operand.displacement < 1 shl 64
				rm_operand.displacement = rm_operand.displacement - 1 shl 64
			else if rm_operand.displacement < -80000000h | rm_operand.displacement >= 80000000h
				err 'address value out of signed range'
			end if
		end if
		dd rm_operand.displacement
	end if
	if imm_size = 1
		db imm
	else if imm_size = 2
		dw imm
	else if imm_size = 4
		dd imm
	else if imm_size = 8
		x86.simm32 imm
	end if
end macro

macro x86.simm32 imm
	local imm64
	if imm eqtype 0.0
		virtual at 0
			emit qword:imm
			load imm64:qword from 0
			imm64 = +imm64
		end virtual
	else
		imm64 = imm
	end if
	if imm64 relativeto 0 & imm64 - 1 shl 64 >= -80000000h & imm64 < 1 shl 64
		dd imm64 - 1 shl 64
	else
		if imm64 relativeto 0 & (imm64 >= 80000000h | imm64 < -80000000h)
			err 'immediate value out of signed range'
		end if
		dd imm64
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
		end if
		if @src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
			if defined size & size > 1
				x86.select_operand_prefix @dest,size
				x86.store_instruction basecode+1,@dest,@src.rm
			else
				x86.store_instruction basecode,@dest,@src.rm
			end if
		else if @src.type = 'mem' & @dest.type = 'reg'
			if defined size & size > 1
				x86.select_operand_prefix @src,size
				x86.store_instruction basecode+3,@src,@dest.rm
			else
				x86.store_instruction basecode+2,@src,@dest.rm
			end if
		else if @src.type = 'imm' & ( @dest.type = 'reg' | @dest.type = 'mem' )
			if defined size & size > 1
				x86.select_operand_prefix @dest,size
				if @src.imm eqtype 0.0
					virtual at 0
						emit size:@src.imm
						load @src.imm:size from 0
						@src.imm = +@src.imm
					end virtual
				end if
				if @src.imm relativeto 0 & @src.imm < 80h & @src.imm >= -80h
					x86.store_instruction 83h,@dest,basecode shr 3,1,@src.imm
				else if @src.imm relativeto 0 & @src.imm - 1 shl (size shl 3) >= -80h & @src.imm < 1 shl (size shl 3)
					@src.imm = @src.imm - 1 shl (size shl 3)
					x86.store_instruction 83h,@dest,basecode shr 3,1,@src.imm
				else if @dest.type = 'reg' & @dest.rm = 0
					if @dest.prefix
						db @dest.prefix
					end if
					db basecode+5
					if size = 2
						dw @src.imm
					else if size = 4
						dd @src.imm
					else
						x86.simm32 @src.imm
					end if
				else
					x86.store_instruction 81h,@dest,basecode shr 3,size,@src.imm
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

iterate <instr,postbyte>, not,2, neg,3, mul,4, div,6, idiv,7
	macro instr? src*
		x86.parse_operand @src,src
		if @src.size = 0
			err 'operand size not specified'
		end if
		if @src.type = 'mem' | @src.type = 'reg'
			if @src.size > 1
				x86.select_operand_prefix @src,@src.size
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
	local size,ext
	if @dest.size = 0 & @src.size = 0 & @src.type <> 'sreg' & @dest.type <> 'sreg'
		err 'operand size not specified'
	else if @dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
		err 'operand sizes do not match'
	else
		size = @dest.size or @src.size
	end if
	if @src.type = 'reg' & @dest.type = 'mem' & @src.rm = 0 & @dest.address_registers eq 0 & ~ @dest.auto_relative
		if x86.mode = 64
			if @dest.segment_prefix & @dest.segment_prefix >= 64h
				db @dest.segment_prefix
			end if
			if @dest.mode = 16
				err 'illegal addressing mode'
			end if
			if @dest.displacement_size < 8
				db 67h
			end if
		else
			if @dest.segment_prefix & @dest.segment_prefix <> 3Eh
				db @dest.segment_prefix
			end if
			if @dest.mode <> x86.mode
				db 67h
			end if
		end if
		if defined size & size > 1
			x86.store_operand_prefix size
			db 0A3h
		else
			db 0A2h
		end if
		if @dest.mode = 16
			dw @dest.address
		else if @dest.displacement_size < 8
			dd @dest.address
		else
			dq @dest.address
		end if
	else if @src.type = 'mem' & @dest.type = 'reg' & @dest.rm = 0 & @src.address_registers eq 0 & ~ @src.auto_relative
		if x86.mode = 64
			if @src.segment_prefix & @src.segment_prefix >= 64h
				db @src.segment_prefix
			end if
			if @src.mode = 16
				err 'illegal addressing mode'
			end if
			if @src.displacement_size < 8
				db 67h
			end if
		else
			if @src.segment_prefix & @src.segment_prefix <> 3Eh
				db @src.segment_prefix
			end if
			if @src.mode <> x86.mode
				db 67h
			end if
		end if
		if defined size & size > 1
			x86.store_operand_prefix size
			db 0A1h
		else
			db 0A0h
		end if
		if @src.mode = 16
			dw @src.address
		else if @src.displacement_size < 8
			dd @src.address
		else
			dq @src.address
		end if
	else if @src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		if defined size & size > 1
			x86.select_operand_prefix @dest,size
			x86.store_instruction 89h,@dest,@src.rm
		else
			x86.store_instruction 88h,@dest,@src.rm
		end if
	else if @src.type = 'mem' & @dest.type = 'reg'
		if defined size & size > 1
			x86.select_operand_prefix @src,size
			x86.store_instruction 8Bh,@src,@dest.rm
		else
			x86.store_instruction 8Ah,@src,@dest.rm
		end if
	else if @src.type = 'imm' & @dest.type = 'mem'
		if defined size & size > 1
			x86.select_operand_prefix @dest,size
			x86.store_instruction 0C7h,@dest,0,size,@src.imm
		else
			x86.store_instruction 0C6h,@dest,0,1,@src.imm
		end if
	else if @src.type = 'imm' & @dest.type = 'reg'
		if 1 metadataof (1 metadataof @src.imm) relativeto x86.creg & @src.imm relativeto 1 elementof @src.imm
			if (x86.mode <> 64 & @dest.size = 4) | (x86.mode = 64 & @dest.size = 8)
				ext = 20h + 1 metadataof (1 metadataof @src.imm) - x86.creg
				@src.rm = 1 metadataof @src.imm - 1 elementof (1 metadataof @src.imm)
				x86.store_instruction <0Fh,ext>,@dest,@src.rm
			else
				err 'invalid operand size'
			end if
		else
			if defined size & size > 1
				if @src.imm eqtype 0.0
					virtual at 0
						emit size:@src.imm
						load @src.imm:size from 0
						@src.imm = +@src.imm
					end virtual
				end if
				if size = 8 & @src.imm relativeto 0 & @src.imm < 80000000h & @src.imm >= -80000000h
					x86.select_operand_prefix @dest,size
					x86.store_instruction 0C7h,@dest,0,size,@src.imm
				else if size = 8 & @src.imm relativeto 0 & @src.imm - 1 shl 64 < 80000000h & @src.imm - 1 shl 64 >= -80000000h
					@src.imm = @src.imm - 1 shl 64
					x86.select_operand_prefix @dest,size
					x86.store_instruction 0C7h,@dest,0,size,@src.imm
				else
					x86.store_operand_prefix size,@dest.rm
					db 0B8h + @dest.rm and 111b
					if size = 2
						dw @src.imm
					else if size = 4
						dd @src.imm
					else
						dq @src.imm
					end if
				end if
			else
				x86.store_operand_prefix 0,@dest.rm
				db 0B0h + @dest.rm and 111b
				db @src.imm
			end if
		end if
	else if @src.type = 'reg' & @dest.type = 'imm'
		if 1 metadataof (1 metadataof @dest.imm) relativeto x86.creg & @dest.imm relativeto 1 elementof @dest.imm
			if (x86.mode <> 64 & @src.size = 4) | (x86.mode = 64 & @src.size = 8)
				ext = 22h + 1 metadataof (1 metadataof @dest.imm) - x86.creg
				@dest.rm = 1 metadataof @dest.imm - 1 elementof (1 metadataof @dest.imm)
				x86.store_instruction <0Fh,ext>,@src,@dest.rm
			else
				err 'invalid operand size'
			end if
		else
			err 'invalid combination of operands'
		end if
	else if @src.type = 'sreg' & @dest.type = 'reg'
		if defined size & size > 1
			x86.select_operand_prefix @dest,size
			x86.store_instruction 8Ch,@dest,@src.rm
		else
			err 'invalid operand size'
		end if
	else if @src.type = 'sreg' & @dest.type = 'mem'
		if defined size & size = 2
			x86.store_instruction 8Ch,@dest,@src.rm
		else
			err 'invalid operand size'
		end if
	else if @dest.type = 'sreg' & @dest.rm <> 1 & ( @src.type = 'reg' | @src.type = 'mem' )
		if defined size & size <> 1
			x86.store_instruction 8Eh,@src,@dest.rm
		else
			err 'invalid operand size'
		end if
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
	end if
	if @src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		if defined size & size > 1
			x86.select_operand_prefix @dest,size
			x86.store_instruction 85h,@dest,@src.rm
		else
			x86.store_instruction 84h,@dest,@src.rm
		end if
	else if @src.type = 'mem' & @dest.type = 'reg'
		if defined size & size > 1
			x86.select_operand_prefix @src,size
			x86.store_instruction 85h,@src,@dest.rm
		else
			x86.store_instruction 84h,@src,@dest.rm
		end if
	else if @src.type = 'imm' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		if defined size & size > 1
			if @dest.type = 'reg' & @dest.rm = 0
				x86.store_operand_prefix size
				db 0A9h
				if size = 2
					dw @src.imm
				else if size = 4
					dd @src.imm
				else
					x86.simm32 @src.imm
				end if
			else
				x86.select_operand_prefix @dest,size
				x86.store_instruction 0F7h,@dest,0,size,@src.imm
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
	end if
	if @src.type = 'reg' & @dest.type = 'reg'
		if defined size & size > 1
			if (@src.rm & @dest.rm) | (size = 4 & @src.rm or @dest.rm = 0)
				x86.select_operand_prefix @src,size
				x86.store_instruction 87h,@src,@dest.rm
			else
				@src.rm = @src.rm or @dest.rm
				x86.store_operand_prefix size,@src.rm
				db 90h + @src.rm and 111b
			end if
		else
			x86.store_instruction 86h,@src,@dest.rm
		end if
	else if @src.type = 'reg' & @dest.type = 'mem'
		if defined size & size > 1
			x86.select_operand_prefix @dest,size
			x86.store_instruction 87h,@dest,@src.rm
		else
			x86.store_instruction 86h,@dest,@src.rm
		end if
	else if @src.type = 'mem' & @dest.type = 'reg'
		if defined size & size > 1
			x86.select_operand_prefix @src,size
			x86.store_instruction 87h,@src,@dest.rm
		else
			x86.store_instruction 86h,@src,@dest.rm
		end if
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,postbyte>, inc,0 ,dec,1
	macro instr? dest*
		x86.parse_operand @dest,dest
		if @dest.size = 0
			err 'operand size not specified'
		end if
		if @dest.type = 'mem' | (x86.mode = 64 & @dest.type = 'reg')
			if @dest.size > 1
				x86.select_operand_prefix @dest,@dest.size
				x86.store_instruction 0FFh,@dest,postbyte
			else
				x86.store_instruction 0FEh,@dest,postbyte
			end if
		else if @dest.type = 'reg'
			if @dest.size > 1
				x86.store_operand_prefix @dest.size
				db 40h + @dest.rm + postbyte shl 3
			else
				x86.store_instruction 0FEh,@dest,postbyte
			end if
		else
			err 'invalid operand'
		end if
	end macro
end iterate

macro imul? dest*,src1,src2
	match , src1 src2
		x86.parse_operand @dest,dest
		if @dest.size = 0
			err 'operand size not specified'
		end if
		if @dest.type = 'mem' | @dest.type = 'reg'
			if @dest.size > 1
				x86.select_operand_prefix @dest,@dest.size
				x86.store_instruction 0F7h,@dest,5
			else
				x86.store_instruction 0F6h,@dest,5
			end if
		else
			err 'invalid operand'
		end if
	else
		x86.parse_operand @dest,dest
		x86.parse_operand @src,src1
		local size
		match , src2
			if @dest.size = 0 & @src.size = 0
				err 'operand size not specified'
			else if @dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
				err 'operand sizes do not match'
			else
				size = @dest.size or @src.size
			end if
			if @dest.type = 'reg' & (@src.type = 'reg' | @src.type = 'mem')
				x86.select_operand_prefix @src,size
				x86.store_instruction <0Fh,0AFh>,@src,@dest.rm
			else if @src.type = 'imm' & @dest.type = 'reg'
				x86.select_operand_prefix @dest,size
				if @src.imm eqtype 0.0
					virtual at 0
						emit size:@src.imm
						load @src.imm:size from 0
						@src.imm = +@src.imm
					end virtual
				end if
				if @src.imm relativeto 0 & @src.imm < 80h & @src.imm >= -80h
					x86.store_instruction 6Bh,@dest,@dest.rm,1,@src.imm
				else if @src.imm relativeto 0 & @src.imm - 1 shl (size shl 3) >= -80h & @src.imm < 1 shl (size shl 3)
					@src.imm = @src.imm - 1 shl (size shl 3)
					x86.store_instruction 6Bh,@dest,@dest.rm,1,@src.imm
				else
					x86.store_instruction 69h,@dest,@dest.rm,size,@src.imm
				end if
			else
				err 'invalid operand'
			end if
		else
			x86.parse_operand @aux,src2
			if @dest.size = 0 & @src.size = 0 & @aux.size = 0
				err 'operand size not specified'
			else if @dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
				err 'operand sizes do not match'
			else
				size = @dest.size or @src.size
				if @aux.size <> 0 & @aux.size <> size
					err 'operand sizes do not match'
				end if
			end if
			if @aux.type = 'imm' & ( @src.type = 'mem' | @src.type = 'reg' ) & @dest.type = 'reg'
				x86.select_operand_prefix @src,size
				if @aux.imm eqtype 0.0
					virtual at 0
						emit size:@aux.imm
						load @aux.imm:size from 0
						@aux.imm = +@aux.imm
					end virtual
				end if
				if @aux.imm relativeto 0 & @aux.imm < 80h & @aux.imm >= -80h
					x86.store_instruction 6Bh,@src,@dest.rm,1,@aux.imm
				else if @aux.imm relativeto 0 & @aux.imm - 1 shl (size shl 3) >= -80h & @aux.imm < 1 shl (size shl 3)
					@aux.imm = @aux.imm - 1 shl (size shl 3)
					x86.store_instruction 6Bh,@src,@dest.rm,1,@aux.imm
				else
					x86.store_instruction 69h,@src,@dest.rm,size,@aux.imm
				end if
			else
				err 'invalid operand'
			end if
		end match
	end match
end macro

macro x86.push_instruction operand_size,src
	x86.parse_operand @src,src
	if @src.size = 0
		@src.size = operand_size
	else if (operand_size <> 0 & @src.size <> operand_size) | (@src.size <> 2 & ((@src.size <> 4 & x86.mode < 64) | (@src.size <> 8 & x86.mode = 64)))
		err 'invalid operand size'
	end if
	if (x86.mode <> 16 & @src.size = 2) | (x86.mode = 16 & @src.size = 4)
		@src.prefix = 66h
	end if
	if @src.type = 'mem'
		x86.store_instruction 0FFh,@src,110b
	else if @src.type = 'reg'
		if @src.prefix
			db @src.prefix
		end if
		if @src.rm and 1000b
			db 41h
			@src.rm = @src.rm and 111b
		end if
		db 50h + @src.rm
	else if @src.type = 'sreg'
		if @src.prefix
			db @src.prefix
		end if
		if @src.rm >= 4
			db 0Fh,0A0h + (@src.rm-4) shl 3
		else if x86.mode <> 64
			db 6 + @src.rm shl 3
		else
			err 'invalid operand'
		end if
	else if @src.type = 'imm'
		if @src.prefix
			db @src.prefix
		end if
		if @src.size = 0
			if x86.mode = 16
				@src.size = 2
			else if x86.mode = 64
				@src.size = 8
			else
				@src.size = 4
			end if
		end if
		if @src.imm eqtype 0.0
			virtual at 0
				emit @src.size:@src.imm
				load @src.imm:@src.size from 0
				@src.imm = +@src.imm
			end virtual
		end if
		if @src.size = 8 & @src.imm relativeto 0
			if @src.imm-10000000000000000h >= -80000000h & @src.imm < 10000000000000000h
				@src.imm = @src.imm - 10000000000000000h
			else if @src.imm >= 80000000h | @src.imm < -80000000h
				err 'immediate value out of signed range'
			end if
		end if
		if @src.imm relativeto 0 & @src.imm < 80h & @src.imm >= -80h
			db 6Ah
			db @src.imm
		else if @src.size = 2 & @src.imm relativeto 0 & @src.imm-10000h >= -80h & @src.imm < 10000h
			@src.imm = @src.imm - 10000h
			db 6Ah
			db @src.imm
		else if @src.size = 4 & @src.imm relativeto 0 & @src.imm-100000000h >= -80h & @src.imm < 100000000h
			@src.imm = @src.imm - 100000000h
			db 6Ah
			db @src.imm
		else
			db 68h
			if @src.size = 2 | (@src.size = 0 & x86.mode = 16)
				dw @src.imm
			else
				dd @src.imm
			end if
		end if
	else
		err 'invalid operand'
	end if
end macro

macro x86.pop_instruction operand_size,dest
	x86.parse_operand @dest,dest
	if @dest.size = 0
		@dest.size = operand_size
	else if (operand_size <> 0 & @dest.size <> operand_size) | (@dest.size <> 2 & ((@dest.size <> 4 & x86.mode < 64) | (@dest.size <> 8 & x86.mode = 64)))
		err 'invalid operand size'
	end if
	if (x86.mode <> 16 & @dest.size = 2) | (x86.mode = 16 & @dest.size = 4)
		@dest.prefix = 66h
	end if
	if @dest.type = 'mem'
		x86.store_instruction 8Fh,@dest,0
	else if @dest.type = 'reg'
		if @dest.prefix
			db @dest.prefix
		end if
		if @dest.rm and 1000b
			db 41h
			@dest.rm = @dest.rm and 111b
		end if
		db 58h + @dest.rm
	else if @dest.type = 'sreg'
		if @dest.prefix
			db @dest.prefix
		end if
		if @dest.rm >= 4
			db 0Fh,0A1h + (@dest.rm-4) shl 3
		else if @dest.rm <> 1 & x86.mode <> 64
			db 7 + @dest.rm shl 3
		else
			err 'invalid operand'
		end if
	else
		err 'invalid operand'
	end if
end macro


iterate reg, ax,cx,dx,bx,sp,bp,si,di,r8w,r9w,r10w,r11w,r12w,r13w,r14w,r15w, \
	     eax,ecx,edx,ebx,esp,ebp,esi,edi,r8d,r9d,r10d,r11d,r12d,r13d,r14d,r15d, \
	     rax,rcx,rdx,rbx,rsp,rbp,rsi,rdi,r8,r9,r10,r11,r12,r13,r14,r15, \
	     es,cs,ss,ds,fs,gs
	define x86.compact.reg? {reg}
end iterate

macro x86.compact: instruction,operand
	match {reg} more, x86.compact.operand
		instruction,reg
		match {reg2} tail, x86.compact.more:
			x86.compact instruction,more
		else
			err 'only register operands allowed in compact syntax'
		end match
	else
		instruction,operand
	end match
end macro

macro push? src*
	x86.compact x86.push_instruction 0,src
end macro

macro pushw? src*
	x86.compact x86.push_instruction 2,src
end macro

macro pushd? src*
	x86.compact x86.push_instruction 4,src
end macro

macro pushq? src*
	x86.compact x86.push_instruction 8,src
end macro

macro pop? dest*
	x86.compact x86.pop_instruction 0,dest
end macro

macro popw? dest*
	x86.compact x86.pop_instruction 2,dest
end macro

macro popd? dest*
	x86.compact x86.pop_instruction 4,dest
end macro

macro popq? dest*
	x86.compact x86.pop_instruction 8,dest
end macro

macro retn? operand
	match imm, operand
		db 0C2h
		dw imm
	else
		db 0C3h
	end match
end macro

macro retnw? operand
	if x86.mode < 64
		x86.store_operand_prefix 2
		match imm, operand
			db 0C2h
			dw imm
		else
			db 0C3h
		end match
	else
		err 'illegal instruction'
	end if
end macro

macro retnd? operand
	if x86.mode < 64
		x86.store_operand_prefix 4
		match imm, operand
			db 0C2h
			dw imm
		else
			db 0C3h
		end match
	else
		err 'illegal instruction'
	end if
end macro

macro retnq? operand
	if x86.mode = 64
		match imm, operand
			db 0C2h
			dw imm
		else
			db 0C3h
		end match
	else
		err 'instruction requires long mode'
	end if
end macro

macro retf? operand
	match imm, operand
		db 0CAh
		dw imm
	else
		db 0CBh
	end match
end macro

macro retfw? operand
	x86.store_operand_prefix 2
	match imm, operand
		db 0CAh
		dw imm
	else
		db 0CBh
	end match
end macro

macro retfd? operand
	x86.store_operand_prefix 4
	match imm, operand
		db 0CAh
		dw imm
	else
		db 0CBh
	end match
end macro

macro retfq? operand
	x86.store_operand_prefix 8
	match imm, operand
		db 0CAh
		dw imm
	else
		db 0CBh
	end match
end macro

macro ret? operand
	retn operand
end macro

macro retw? operand
	retnw operand
end macro

macro retd? operand
	retnd operand
end macro

macro retq? operand
	retnq operand
end macro

macro lea? dest*,src*
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	if @src.type = 'mem' & @dest.type = 'reg'
		x86.select_operand_prefix @src,@dest.size
		x86.store_instruction 8Dh,@src,@dest.rm
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,opcode>, les,0C4h, lds,0C5h
	macro instr? dest*,src*
		if x86.mode < 64
			x86.parse_operand @dest,dest
			x86.parse_operand @src,src
			if (@dest.size = 2 & (@src.size <> 0 & @src.size <> 4)) | (@dest.size = 4 & (@src.size <> 0 & @src.size <> 6))
				err 'invalid operand size'
			end if
			if @src.type = 'mem' & @dest.type = 'reg'
				x86.select_operand_prefix @src,@dest.size
				x86.store_instruction opcode,@src,@dest.rm
			else
				err 'invalid combination of operands'
			end if
		else
			err 'illegal instruction'
		end if
	end macro
end iterate

iterate <instr,ext>, lss,0B2h, lfs,0B4h, lgs,0B5h
	macro instr? dest*,src*
		x86.parse_operand @dest,dest
		x86.parse_operand @src,src
		if (@dest.size = 2 & (@src.size <> 0 & @src.size <> 4)) | (@dest.size = 4 & (@src.size <> 0 & @src.size <> 6)) | (@dest.size = 8 & (@src.size <> 0 & @src.size <> 10))
			err 'invalid operand size'
		end if
		if @src.type = 'mem' & @dest.type = 'reg'
			x86.select_operand_prefix @src,@dest.size
			x86.store_instruction <0Fh,ext>,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro
end iterate

macro x86.shift_instruction opcode,dest,cnt
	x86.parse_operand @dest,dest
	x86.parse_operand @src,cnt
	if @dest.size = 0
		err 'operand size not specified'
	end if
	if @src.size and not 1
		err 'invalid operand size'
	end if
	if @src.type = 'reg' & @src.size = 1 & @src.rm = 1 & ( @dest.type = 'reg' | @dest.type = 'mem' )
		if @dest.size > 1
			x86.select_operand_prefix @dest,@dest.size
			x86.store_instruction 0D3h,@dest,opcode
		else
			x86.store_instruction 0D2h,@dest,opcode
		end if
	else if @src.type = 'imm' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		if @dest.size >1
			x86.select_operand_prefix @dest,@dest.size
			if @src.imm = 1
				x86.store_instruction 0D1h,@dest,opcode
			else
				x86.store_instruction 0C1h,@dest,opcode,1,@src.imm
			end if
		else
			if @src.imm = 1
				x86.store_instruction 0D0h,@dest,opcode
			else
				x86.store_instruction 0C0h,@dest,opcode,1,@src.imm
			end if
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

iterate <instr,ext>, movzx,0B6h, movsx,0BEh
	macro instr? dest*,src*
		x86.parse_operand @dest,dest
		x86.parse_operand @src,src
		if @dest.size <= @src.size
			err 'operand sizes do not match'
		end if
		if @dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg')
			if @src.size = 2
				x86.select_operand_prefix @src,@dest.size
				x86.store_instruction <0Fh,ext+1>,@src,@dest.rm
			else if @src.size = 1
				x86.select_operand_prefix @src,@dest.size
				x86.store_instruction <0Fh,ext>,@src,@dest.rm
			else if @src.size <> 0
				err 'invalid operand size'
			else
				err 'operand size not specified'
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro
end iterate

macro movsxd? dest*,src*
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	if @dest.size <= @src.size
		err 'operand sizes do not match'
	end if
	if @dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg')
		if @src.size = 0 | @src.size = 4
			x86.select_operand_prefix @src,@dest.size
			x86.store_instruction 63h,@src,@dest.rm
		else
			err 'invalid operand size'
		end if
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,postbyte>, bt,4, bts,5, btr,6, btc,7
	macro instr? dest*,src*
		x86.parse_operand @dest,dest
		x86.parse_operand @src,src
		local size
		if @src.type = 'reg' & (@dest.type = 'mem' | @dest.type = 'reg')
			if @dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
				err 'operand sizes do not match'
			else
				size = @dest.size or @src.size
			end if
			x86.select_operand_prefix @dest,size
			x86.store_instruction <0Fh,0A3h+(postbyte-4) shl 3>,@dest,@src.rm
		else if @src.type = 'imm' & (@dest.type = 'mem' | @dest.type = 'reg')
			if @src.size <> 0 & @src.size <> 1
				err 'invalid operand size'
			end if
			if @dest.size
				x86.select_operand_prefix @dest,@dest.size
				x86.store_instruction <0Fh,0BAh>,@dest,postbyte,1,@src.imm
			else
				err 'operand size not specified'
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro
end iterate

iterate <instr,ext>, bsf,0BCh, bsr,0BDh
	macro instr? dest*,src*
		x86.parse_operand @dest,dest
		x86.parse_operand @src,src
		local size
		if @dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
			err 'operand sizes do not match'
		else
			size = @dest.size or @src.size
		end if
		if @dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg')
			x86.select_operand_prefix @src,@dest.size
			x86.store_instruction <0Fh,ext>,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro
end iterate

iterate <instr,ext>, shld,0A4h, shrd,0ACh
	macro instr? dest*,src*,cnt*
		x86.parse_operand @dest,dest
		x86.parse_operand @src,src
		x86.parse_operand @aux,cnt
		local size
		if @dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
			err 'operand sizes do not match'
		else
			size = @dest.size or @src.size
		end if
		if @aux.size <> 0 & @aux.size <> 1
			err 'invalid operand size'
		end if
		if @aux.type = 'reg' & @aux.size = 1 & @aux.rm = 1 & @src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
			x86.select_operand_prefix @dest,size
			x86.store_instruction <0Fh,ext+1>,@dest,@src.rm
		else if @aux.type = 'imm' & @src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
			x86.select_operand_prefix @dest,size
			x86.store_instruction <0Fh,ext>,@dest,@src.rm,1,@aux.imm
		else
			err 'invalid combination of operands'
		end if
	end macro
end iterate

iterate <instr,ext>, cmpxchg,0B0h, xadd,0C0h
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
		end if
		if @src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
			if defined size & size > 1
				x86.select_operand_prefix @dest,size
				x86.store_instruction <0Fh,ext+1>,@dest,@src.rm
			else
				x86.store_instruction <0Fh,ext>,@dest,@src.rm
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro
end iterate

macro bswap? dest*
	x86.parse_operand @dest,dest
	if @dest.type = 'reg' & @dest.size > 2
		x86.store_operand_prefix @dest.size,@dest.rm
		db 0Fh,0C8h + @dest.rm and 111b
	else
		err 'invalid operand'
	end if
end macro

macro cmpxchg8b? dest*
	x86.parse_operand @dest,dest
	if @dest.size <> 0 & @dest.size <> 8
		err 'invalid operand size'
	end if
	if @dest.type = 'mem'
		x86.store_instruction <0Fh,0C7h>,@dest,1
	else
		err 'invalid operand'
	end if
end macro

macro cmpxchg16b? dest*
	x86.parse_operand @dest,dest
	if @dest.size <> 0 & @dest.size <> 16
		err 'invalid operand size'
	end if
	if @dest.type = 'mem'
		x86.store_operand_prefix 8
		x86.store_instruction <0Fh,0C7h>,@dest,1
	else
		err 'invalid operand'
	end if
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
	if ns.type = 'imm'
		if ns.size = 0
			ns.size = x86.mode shr 3
		end if
		if ns.imm relativeto 0 & (ns.imm < 0 | ns.imm >= 1 shl (ns.size*8))
			err 'value out of range'
		end if
	end if
end macro

macro call? dest*
	x86.parse_jump_operand @dest,dest
	if @dest.type = 'far'
		if @dest.jump_type & @dest.jump_type <> 'far'
			err 'invalid operand'
		else if x86.mode < 64
			if @dest.size = 0
				if x86.mode = 16
					db 9Ah
					dw @dest.offset,@dest.segment
				else
					db 9Ah
					dd @dest.offset
					dw @dest.segment
				end if
			else if @dest.size = 4 | @dest.size = 6
				x86.store_operand_prefix (@dest.size-2)
				db 9Ah
				if @dest.size = 4
					dw @dest.offset,@dest.segment
				else
					dd @dest.offset
					dw @dest.segment
				end if
			else
				err 'invalid operand size'
			end if
		else
			err 'illegal instruction'
		end if
	else if @dest.type = 'mem' | @dest.type = 'reg'
		if @dest.size = 6 | @dest.size = 10
			if @dest.jump_type & @dest.jump_type <> 'far'
				err 'invalid operand'
			end if
			x86.select_operand_prefix @dest,(@dest.size-2)
			x86.store_instruction 0FFh,@dest,11b
		else if @dest.size = 8 & x86.mode = 64
			if @dest.jump_type & @dest.jump_type <> 'near'
				err 'invalid operand'
			end if
			x86.store_instruction 0FFh,@dest,10b
		else if @dest.size = 4 & x86.mode < 64
			if @dest.jump_type | @dest.type = 'reg'
				if @dest.jump_type = 'far'
					x86.select_operand_prefix @dest,2
					x86.store_instruction 0FFh,@dest,11b
				else
					x86.select_operand_prefix @dest,4
					x86.store_instruction 0FFh,@dest,10b
				end if
			else
				if x86.mode = 16
					x86.select_operand_prefix @dest,2
					x86.store_instruction 0FFh,@dest,11b
				else
					x86.select_operand_prefix @dest,4
					x86.store_instruction 0FFh,@dest,10b
				end if
			end if
		else if @dest.size = 2 & x86.mode < 64
			if @dest.jump_type & @dest.jump_type <> 'near'
				err 'invalid operand'
			end if
			x86.select_operand_prefix @dest,2
			x86.store_instruction 0FFh,@dest,10b
		else if @dest.size = 0
			if x86.mode = 64
				@dest.prefix = 48h
			end if
			if @dest.jump_type = 'far'
				x86.store_instruction 0FFh,@dest,11b
			else if @dest.jump_type = 'near'
				x86.store_instruction 0FFh,@dest,10b
			else
				err 'operand size not specified'
			end if
		else
			err 'invalid operand'
		end if
	else if @dest.type = 'imm'
		if @dest.jump_type & @dest.jump_type <> 'near'
			err 'invalid operand'
		end if
		if @dest.size = 8 & x86.mode = 64
			if @dest.imm relativeto $ & (@dest.imm-($+4) < -80000000h | @dest.imm-($+4) >= 80000000h)
				err 'relative jump out of range'
			else
				db 0E8h
				dd @dest.imm-($+4)
			end if
		else if @dest.size = 2 & x86.mode <> 64
			x86.store_operand_prefix 2
			db 0E8h
			dw @dest.imm-($+2)
		else if @dest.size = 4 & x86.mode <> 64
			x86.store_operand_prefix 4
			db 0E8h
			dd @dest.imm-($+4)
		else
			err 'invalid operand size'
		end if
	else
		err 'invalid operand'
	end if
end macro

macro jmp? dest*
	x86.parse_jump_operand @dest,dest
	if @dest.type = 'far'
		if @dest.jump_type & @dest.jump_type <> 'far'
			err 'invalid operand'
		else if x86.mode < 64
			if @dest.size = 0
				db 0EAh
				if x86.mode = 16
					dw @dest.offset,@dest.segment
				else
					dd @dest.offset
					dw @dest.segment
				end if
			else if @dest.size = 4 | @dest.size = 6
				x86.store_operand_prefix (@dest.size-2)
				db 0EAh
				if @dest.size = 4
					dw @dest.offset,@dest.segment
				else
					dd @dest.offset
					dw @dest.segment
				end if
			else
				err 'invalid operand size'
			end if
		else
			err 'illegal instruction'
		end if
	else if @dest.type = 'mem' | @dest.type = 'reg'
		if @dest.size = 6 | @dest.size = 10
			if @dest.jump_type & @dest.jump_type <> 'far'
				err 'invalid operand'
			end if
			x86.select_operand_prefix @dest,(@dest.size-2)
			x86.store_instruction 0FFh,@dest,101b
		else if @dest.size = 8 & x86.mode = 64
			if @dest.jump_type & @dest.jump_type <> 'near'
				err 'invalid operand'
			end if
			x86.store_instruction 0FFh,@dest,100b
		else if @dest.size = 4 & x86.mode < 64
			if @dest.jump_type | @dest.type = 'reg'
				if @dest.jump_type = 'far'
					x86.select_operand_prefix @dest,2
					x86.store_instruction 0FFh,@dest,101b
				else
					x86.select_operand_prefix @dest,4
					x86.store_instruction 0FFh,@dest,100b
				end if
			else
				if x86.mode = 16
					x86.select_operand_prefix @dest,2
					x86.store_instruction 0FFh,@dest,101b
				else
					x86.select_operand_prefix @dest,4
					x86.store_instruction 0FFh,@dest,100b
				end if
			end if
		else if @dest.size = 2 & x86.mode < 64
			if @dest.jump_type & @dest.jump_type <> 'near'
				err 'invalid operand'
			end if
			x86.select_operand_prefix @dest,2
			x86.store_instruction 0FFh,@dest,100b
		else if @dest.size = 0
			if @dest.jump_type = 'far'
				x86.store_instruction 0FFh,@dest,101b
			else if @dest.jump_type = 'near'
				x86.store_instruction 0FFh,@dest,100b
			else
				err 'operand size not specified'
			end if
		else
			err 'invalid operand size'
		end if
	else if @dest.type = 'imm'
		if @dest.size = 8 & x86.mode = 64
			if @dest.jump_type = 'short'
				db 0EBh
				if @dest.imm-($+1) < 80h & @dest.imm-($+1) >= -80h
					db @dest.imm-($+1)
				else
					err 'relative jump out of range'
					db ?
				end if
			else if @dest.jump_type = 'near'
				if @dest.imm relativeto $ & (@dest.imm-($+4) < -80000000h | @dest.imm-($+4) >= 80000000h)
					err 'relative jump out of range'
				else
					db 0E9h
					dd @dest.imm-($+4)
				end if
			else if ~ @dest.jump_type
				if ~ $ relativeto 0 & @dest.imm relativeto 0
					@dest.imm = @dest.imm + $ - $ scale 0
					err 'invalid address'
				end if
				if @dest.unresolved | ( @dest.imm relativeto $ & @dest.imm-($+2) < 80h & @dest.imm-($+2) >= -80h )
					db 0EBh
					db @dest.imm-($+1)
				else if @dest.imm relativeto $ & (@dest.imm-($+4) < -80000000h | @dest.imm-($+4) >= 80000000h)
					err 'relative jump out of range'
				else
					db 0E9h
					dd @dest.imm-($+4)
				end if
			else
				err 'invalid operand'
			end if
		else if @dest.size = 2 & x86.mode <> 64
			x86.store_operand_prefix 2
			if @dest.jump_type = 'near'
				db 0E9h
				dw @dest.imm-($+2)
			else if @dest.jump_type = 'short'
				db 0EBh
				if (@dest.imm-($+2)) and 0FFFFh < 80h | (@dest.imm-($+2)) and 0FFFFh >= 0FF80h
					db (@dest.imm-($+1)) and 0FFh
				else
					err 'relative jump out of range'
					db ?
				end if
			else if ~ @dest.jump_type
				if ~ $ relativeto 0 & @dest.imm relativeto 0
					@dest.imm = @dest.imm + $ - $ scale 0
					err 'invalid address'
				end if
				if @dest.unresolved | ( @dest.imm relativeto $ & ( (@dest.imm-($+2)) and 0FFFFh < 80h | (@dest.imm-($+2)) and 0FFFFh >= 0FF80h ) )
					db 0EBh
					db (@dest.imm-($+1)) and 0FFh
				else
					db 0E9h
					if @dest.imm relativeto $
						dw (@dest.imm-($+2)) and 0FFFFh
					else
						dw @dest.imm-($+2)
					end if
				end if
			else
				err 'invalid operand'
			end if
		else if @dest.size = 4 & x86.mode <> 64
			x86.store_operand_prefix 4
			if @dest.jump_type = 'near'
				db 0E9h
				dd @dest.imm-($+4)
			else if @dest.jump_type = 'short'
				db 0EBh
				if @dest.imm-($+1) < 80h & @dest.imm-($+1) >= -80h
					db @dest.imm-($+1)
				else
					err 'relative jump out of range'
					db ?
				end if
			else if ~ @dest.jump_type
				if ~ $ relativeto 0 & @dest.imm relativeto 0
					@dest.imm = @dest.imm + $ - $ scale 0
					err 'invalid address'
				end if
				if @dest.unresolved | ( @dest.imm relativeto $ & @dest.imm-($+2) < 80h & @dest.imm-($+2) >= -80h )
					db 0EBh
					db @dest.imm-($+1)
				else
					db 0E9h
					dd @dest.imm-($+4)
				end if
			else
				err 'invalid operand'
			end if
		else
			err 'invalid operand size'
		end if
	else
		err 'invalid operand'
	end if
end macro

iterate <instr,opcode>, jo,70h, jno,71h, jc,72h, jb,72h, jnae,72h, jnc,73h, jnb,73h, jae,73h, jz,74h, je,74h, jnz,75h, jne,75h, jna,76h, jbe,76h, ja,77h, jnbe,77h, \
			js,78h, jns,79h, jp,7Ah, jpe,7Ah, jnp,7Bh, jpo,7Bh, jl,7Ch, jnge,7Ch, jnl,7Dh, jge,7Dh, jng,7Eh, jle,7Eh, jg,7Fh, jnle,7Fh
	macro instr? dest*
		x86.parse_jump_operand @dest,dest
		if @dest.type = 'imm' & @dest.jump_type <> 'far'
			if x86.mode <> 64 & @dest.size <> 8
				x86.store_operand_prefix @dest.size
			else if x86.mode = 64 & @dest.size <> 8
				err 'invalid operand size'
			end if
			if ~ $ relativeto 0 & @dest.imm relativeto 0
				@dest.imm = @dest.imm + $ - $ scale 0
				err 'invalid address'
			end if
			if @dest.jump_type <> 'near' & ( @dest.unresolved | ( @dest.imm relativeto $ & @dest.imm-($+2) < 80h & @dest.imm-($+2) >= -80h ) )
				db opcode
				db @dest.imm-($+1)
			else if @dest.jump_type <> 'near' & @dest.size = 2 & @dest.imm relativeto $ & ( (@dest.imm-($+2)) and 0FFFFh < 80h | (@dest.imm-($+2)) and 0FFFFh >= 0FF80h )
				db opcode
				db (@dest.imm-($+1)) and 0FFh
			else if @dest.jump_type = 'short'
				err 'relative jump out of range'
				db ?,?
			else
				db 0Fh,10h+opcode
				if @dest.size = 2
					if @dest.imm relativeto $
						dw (@dest.imm-($+2)) and 0FFFFh
					else
						dw @dest.imm-($+2)
					end if
				else
					dd @dest.imm-($+4)
				end if
			end if
		else
			err 'invalid operand'
		end if
	end macro
end iterate

iterate <instr,opcode,len>, loopnz,0E0h,0, loopne,0E0h,0, loopz,0E1h,0, loope,0E1h,0, loop,0E2h,0, \
			    loopnzw,0E0h,2, loopnew,0E0h,2, loopzw,0E1h,2, loopew,0E1h,2, loopw,0E2h,2, \
			    loopnzd,0E0h,4, loopned,0E0h,4, loopzd,0E1h,4, looped,0E1h,4, loopd,0E2h,4, \
			    loopnzq,0E0h,8, loopneq,0E0h,8, loopzq,0E1h,8, loopeq,0E1h,8, loopq,0E2h,8, \
			    jcxz,0E3h,2, jecxz,0E3h,4, jrcxz,0E3h,8
	macro instr? dest*
		x86.parse_jump_operand @dest,dest
		if @dest.type = 'imm' & ( @dest.jump_type = 'short' | ~ @dest.jump_type )
			if len & len shl 3 <> x86.mode
				if len = 8 | (len = 2 & x86.mode = 64)
					err 'illegal instruction'
				end if
				db 67h
			end if
			if @dest.size shl 3 <> x86.mode
				if @dest.size = 8 | x86.mode = 64
					err 'invalid operand size'
				end if
				db 66h
			end if
			db opcode
			if @dest.imm-($+1) < 80h & @dest.imm-($+1) >= -80h
				db @dest.imm-($+1)
			else if @dest.size = 2 & ( (@dest.imm-($+2)) and 0FFFFh < 80h | (@dest.imm-($+2)) and 0FFFFh >= 0FF80h )
				db (@dest.imm-($+1)) and 0FFh
			else
				err 'relative jump out of range'
				db ?
			end if
		else
			err 'invalid operand'
		end if
	end macro
end iterate

macro daa?
	if x86.mode < 64
		db 27h
	else
		err 'illegal instruction'
	end if
end macro

macro das?
	if x86.mode < 64
		db 2Fh
	else
		err 'illegal instruction'
	end if
end macro

macro aaa?
	if x86.mode < 64
		db 37h
	else
		err 'illegal instruction'
	end if
end macro

macro aas?
	if x86.mode < 64
		db 3Fh
	else
		err 'illegal instruction'
	end if
end macro

macro nop?
	db 90h
end macro

macro cbw?
	x86.store_operand_prefix 2
	db 98h
end macro

macro cwde?
	x86.store_operand_prefix 4
	db 98h
end macro

macro cdqe?
	x86.store_operand_prefix 8
	db 98h
end macro

macro cwd?
	x86.store_operand_prefix 2
	db 99h
end macro

macro cdq?
	x86.store_operand_prefix 4
	db 99h
end macro

macro cqo?
	x86.store_operand_prefix 8
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

macro int1?
	db 0F1h
end macro

macro iret?
	if x86.mode = 64
		db 48h
	end if
	db 0CFh
end macro

macro iretw?
	x86.store_operand_prefix 2
	db 0CFh
end macro

macro iretd?
	x86.store_operand_prefix 4
	db 0CFh
end macro

macro iretq?
	x86.store_operand_prefix 8
	db 0CFh
end macro

macro aam? number:10
	if x86.mode < 64
		db 0D4h,number
	else
		err 'illegal instruction'
	end if
end macro

macro aad? number:10
	if x86.mode < 64
		db 0D5h,number
	else
		err 'illegal instruction'
	end if
end macro

macro salc?
	db 0D6h
end macro

macro lock? instr&
	db 0F0h
	instr
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

macro pushfw?
	x86.store_operand_prefix 2
	db 9Ch
end macro

macro popfw?
	x86.store_operand_prefix 2
	db 9Dh
end macro

macro pushfd?
	if x86.mode < 64
		x86.store_operand_prefix 4
		db 9Ch
	else
		err 'illegal instruction'
	end if
end macro

macro popfd?
	if x86.mode < 64
		x86.store_operand_prefix 4
		db 9Dh
	else
		err 'illegal instruction'
	end if
end macro

macro pushfq?
	if x86.mode = 64
		db 9Ch
	else
		err 'instruction requires long mode'
	end if
end macro

macro popfq?
	if x86.mode = 64
		db 9Dh
	else
		err 'instruction requires long mode'
	end if
end macro

macro pusha?
	if x86.mode < 64
		db 60h
	else
		err 'illegal instruction'
	end if
end macro

macro popa?
	if x86.mode < 64
		db 61h
	else
		err 'illegal instruction'
	end if
end macro

macro pushaw?
	if x86.mode < 64
		x86.store_operand_prefix 2
		db 60h
	else
		err 'illegal instruction'
	end if
end macro

macro popaw?
	if x86.mode < 64
		x86.store_operand_prefix 2
		db 61h
	else
		err 'illegal instruction'
	end if
end macro

macro pushad?
	if x86.mode < 64
		x86.store_operand_prefix 4
		db 60h
	else
		err 'illegal instruction'
	end if
end macro

macro popad?
	if x86.mode < 64
		x86.store_operand_prefix 4
		db 61h
	else
		err 'illegal instruction'
	end if
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
	x86.store_operand_prefix 2
	db 0A5h
end macro

macro movsq?
	x86.store_operand_prefix 8
	db 0A5h
end macro

macro cmpsb?
	db 0A6h
end macro

macro cmpsw?
	x86.store_operand_prefix 2
	db 0A7h
end macro

macro cmpsq?
	x86.store_operand_prefix 8
	db 0A7h
end macro

macro stosb?
	db 0AAh
end macro

macro stosw?
	x86.store_operand_prefix 2
	db 0ABh
end macro

macro stosd?
	x86.store_operand_prefix 4
	db 0ABh
end macro

macro stosq?
	x86.store_operand_prefix 8
	db 0ABh
end macro

macro lodsb?
	db 0ACh
end macro

macro lodsw?
	x86.store_operand_prefix 2
	db 0ADh
end macro

macro lodsd?
	x86.store_operand_prefix 4
	db 0ADh
end macro

macro lodsq?
	x86.store_operand_prefix 8
	db 0ADh
end macro

macro scasb?
	db 0AEh
end macro

macro scasw?
	x86.store_operand_prefix 2
	db 0AFh
end macro

macro scasd?
	x86.store_operand_prefix 4
	db 0AFh
end macro

macro scasq?
	x86.store_operand_prefix 8
	db 0AFh
end macro

macro insb?
	db 06Ch
end macro

macro insw?
	x86.store_operand_prefix 2
	db 06Dh
end macro

macro insd?
	x86.store_operand_prefix 4
	db 06Dh
end macro

macro outsb?
	db 06Eh
end macro

macro outsw?
	x86.store_operand_prefix 2
	db 06Fh
end macro

macro outsd?
	x86.store_operand_prefix 4
	db 06Fh
end macro

macro xlatb?
	db 0D7h
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
	end if
	if @src.type = 'mem' & @src.mod = 0 & @dest.type = 'mem' & @dest.mod = 0 & ( (x86.mode < 64 & @src.mode = 16 & @src.rm = 4 & @dest.mode = 16 & @dest.rm = 5) | (@src.mode > 16 & @src.rm = 6 & @dest.mode = @src.mode & @dest.rm = 7) ) & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h | (x86.mode = 64 & @dest.segment_prefix < 64h))
		if @src.segment_prefix & @src.segment_prefix <> 3Eh & (x86.mode < 64 | @src.segment_prefix >= 64h)
			db @src.segment_prefix
		end if
		if defined size & size > 1
			if @dest.mode <> x86.mode
				db 67h
			end if
			x86.store_operand_prefix size
			db 0A5h
		else
			if @dest.mode <> x86.mode
				db 67h
			end if
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
	end if
	if @src.type = 'mem' & @src.mod = 0 & @dest.type = 'mem' & @dest.mod = 0 & ( (x86.mode < 64 & @src.mode = 16 & @src.rm = 4 & @dest.mode = 16 & @dest.rm = 5) | (@src.mode > 16 & @src.rm = 6 & @dest.mode = @src.mode & @dest.rm = 7) ) & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h | (x86.mode = 64 & @dest.segment_prefix < 64h))
		if @src.segment_prefix & @src.segment_prefix <> 3Eh & (x86.mode < 64 | @src.segment_prefix >= 64h)
			db @src.segment_prefix
		end if
		if defined size & size > 1
			if @dest.mode <> x86.mode
				db 67h
			end if
			x86.store_operand_prefix size
			db 0A7h
		else
			if @dest.mode <> x86.mode
				db 67h
			end if
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
	end if
	if @dest.type = 'mem' & @dest.mod = 0 & ( (x86.mode < 64 & @dest.mode = 16 & @dest.rm = 5) | (@dest.mode > 16 & @dest.rm = 7) ) & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h | (x86.mode = 64 & @dest.segment_prefix < 64h))
		if @dest.size > 1
			if @dest.mode <> x86.mode
				db 67h
			end if
			x86.store_operand_prefix @dest.size
			db 0ABh
		else
			if @dest.mode <> x86.mode
				db 67h
			end if
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
	end if
	if @src.type = 'mem' & @src.mod = 0 & ( (x86.mode < 64 & @src.mode = 16 & @src.rm = 4) | (@src.mode > 16 & @src.rm = 6) )
		if @src.segment_prefix & @src.segment_prefix <> 3Eh & (x86.mode < 64 | @src.segment_prefix >= 64h)
			db @src.segment_prefix
		end if
		if @src.size > 1
			if @src.mode <> x86.mode
				db 67h
			end if
			x86.store_operand_prefix @src.size
			db 0ADh
		else
			if @src.mode <> x86.mode
				db 67h
			end if
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
	end if
	if @dest.type = 'mem' & @dest.mod = 0 & ( (x86.mode < 64 & @dest.mode = 16 & @dest.rm = 5) | (@dest.mode > 16 & @dest.rm = 7) ) & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h | (x86.mode = 64 & @dest.segment_prefix < 64h))
		if @dest.size > 1
			if @dest.mode <> x86.mode
				db 67h
			end if
			x86.store_operand_prefix @dest.size
			db 0AFh
		else
			if @dest.mode <> x86.mode
				db 67h
			end if
			db 0AEh
		end if
	else
		err 'invalid operand'
	end if
end macro

macro ins? dest*,src*
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	if @dest.size = 0
		err 'operand size not specified'
	else if @dest.size = 8
		err 'invalid operand size'
	end if
	if @src.type = 'reg' & @src.size = 2 & @src.rm = 2 & @dest.type = 'mem' & @dest.mod = 0 & ( (x86.mode < 64 & @dest.mode = 16 & @dest.rm = 5) | (@dest.mode > 16 & @dest.rm = 7) ) & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h | (x86.mode = 64 & @dest.segment_prefix < 64h))
		if @dest.size > 1
			if @dest.mode <> x86.mode
				db 67h
			end if
			x86.store_operand_prefix @dest.size
			db 06Dh
		else
			if @dest.mode <> x86.mode
				db 67h
			end if
			db 06Ch
		end if
	else
		err 'invalid operand'
	end if
end macro

macro outs? dest*,src*
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	if @src.size = 0
		err 'operand size not specified'
	else if @src.size = 8
		err 'invalid operand size'
	end if
	if @dest.type = 'reg' & @dest.size = 2 & @dest.rm = 2 & @src.type = 'mem' & @src.mod = 0 & ( (x86.mode < 64 & @src.mode = 16 & @src.rm = 4) | (@src.mode > 16 & @src.rm = 6) )
		if @src.segment_prefix & @src.segment_prefix <> 3Eh & (x86.mode < 64 | @src.segment_prefix >= 64h)
			db @src.segment_prefix
		end if
		if @src.size > 1
			if @src.mode <> x86.mode
				db 67h
			end if
			x86.store_operand_prefix @src.size
			db 06Fh
		else
			if @src.mode <> x86.mode
				db 67h
			end if
			db 06Eh
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
	if @src.type = 'mem' & @src.mod = 0 & ( (x86.mode < 64 & @src.mode = 16 & @src.rm = 7) | (@src.mode > 16 & @src.rm = 3) )
		if @src.segment_prefix & @src.segment_prefix <> 3Eh & (x86.mode < 64 | @src.segment_prefix >= 64h)
			db @src.segment_prefix
		end if
		if @src.mode <> x86.mode
			db 67h
		end if
		db 0D7h
	else
		err 'invalid operand'
	end if
end macro

macro in? dest*,src*
	x86.parse_operand @dest,dest
	x86.parse_operand @src,src
	if @dest.size = 0
		err 'operand size not specified'
	else if @dest.size = 8
		err 'invalid operand size'
	end if
	if @src.type = 'reg' & @src.size = 2 & @src.rm = 2 & @dest.type = 'reg' & @dest.rm = 0
		if @dest.size > 1
			x86.store_operand_prefix @dest.size
			db 0EDh
		else
			db 0ECh
		end if
	else if @src.type = 'imm' & @dest.type = 'reg' & @dest.rm = 0
		if @dest.size > 1
			x86.store_operand_prefix @dest.size
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
	else if @src.size = 8
		err 'invalid operand size'
	end if
	if @dest.type = 'reg' & @dest.size = 2 & @dest.rm = 2 & @src.type = 'reg' & @src.rm = 0
		if @src.size > 1
			x86.store_operand_prefix @src.size
			db 0EFh
		else
			db 0EEh
		end if
	else if @dest.type = 'imm' & @src.type = 'reg' & @src.rm = 0
		if @src.size > 1
			x86.store_operand_prefix @src.size
			db 0E7h,@dest.imm
		else
			db 0E6h,@dest.imm
		end if
	else
		err 'invalid combination of operands'
	end if
end macro

macro bound? dest*,src*
	if x86.mode < 64
		x86.parse_operand @dest,dest
		x86.parse_operand @src,src
		local size
		if @dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
			err 'operand sizes do not match'
		else
			size = @dest.size or @src.size
		end if
		if @src.type = 'mem' & @dest.type = 'reg'
			x86.select_operand_prefix @src,size
			x86.store_instruction 62h,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	else
		err 'illegal instruction'
	end if
end macro

macro enter? alloc*,nesting*
	x86.parse_operand @src,alloc
	x86.parse_operand @aux,nesting
	if (@src.size <> 0 & @src.size <> 2) | (@aux.size <> 0 & @aux.size <> 1)
		err 'invalid operand size'
	end if
	if @src.type = 'imm' & @aux.type = 'imm'
		db 0C8h
		dw @src.imm
		db @aux.imm
	else
		err 'invalid operand'
	end if
end macro

macro leave?
	db 0C9h
end macro

iterate <cond,code>, o,0, no,1, c,2, b,2, nae,2, nc,3, nb,3, ae,3, z,4, e,4, nz,5, ne,5, na,6, be,6, a,7, nbe,7, \
		     s,8, ns,9, p,0Ah, pe,0Ah, np,0Bh, po,0Bh, l,0Ch, nge,0Ch, nl,0Dh, ge,0Dh, ng,0Eh, le,0Eh, g,0Fh, nle,0Fh

	macro set#cond? dest*
		x86.parse_operand @dest,dest
		if @dest.size > 1
			err 'invalid operand size'
		end if
		if @dest.type = 'reg' | @dest.type = 'mem'
			x86.store_instruction <0Fh,90h+code>,@dest,0
		else
			err 'invalid operand'
		end if
	end macro

	macro cmov#cond? dest*,src*
		x86.parse_operand @dest,dest
		x86.parse_operand @src,src
		local size
		if @dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
			err 'operand sizes do not match'
		else
			size = @dest.size or @src.size
		end if
		if @dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg')
			x86.select_operand_prefix @src,@dest.size
			x86.store_instruction <0Fh,40h+code>,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate

iterate <instr,opcode>, fcmovb,<0DAh,0C0h>, fcmove,<0DAh,0C8h>, fcmovbe,<0DAh,0D0h>, fcmovu,<0DAh,0D8h>, \
			fcmovnb,<0DBh,0C0h>, fcmovne,<0DBh,0C8h>, fcmovnbe,<0DBh,0D0h>, fcmovnu,<0DBh,0D8h>

	macro instr? dest*,src*
		x87.parse_operand @dest,dest
		x87.parse_operand @src,src
		if @dest.type = 'streg' & @dest.rm = 0 & @src.type = 'streg'
			db opcode + @src.rm
		else
			err 'invalid operand'
		end if
	end macro

end iterate

iterate <instr,opcode,postbyte>, fucomi,0DBh,5, fucomip,0DFh,5, fcomi,0DBh,6, fcomip,0DFh,6

	macro instr? src:st1
		x87.parse_operand @src,src
		if @src.type = 'streg'
			db opcode, 11b shl 6 + postbyte shl 3 + @src.rm
		else
			err 'invalid operand'
		end if
	end macro

end iterate

macro swapgs?
	if x86.mode = 64
		db 0Fh,1,0F8h
	else
		err 'instruction requires long mode'
	end if
end macro

macro syscall?
	if x86.mode = 64
		db 0Fh,5
	else
		err 'instruction requires long mode'
	end if
end macro

macro sysret?
	if x86.mode = 64
		db 0Fh,7
	else
		err 'instruction requires long mode'
	end if
end macro

macro sysretq?
	if x86.mode = 64
		db 48h
		db 0Fh,7
	else
		err 'instruction requires long mode'
	end if
end macro

macro clts?
	db 0Fh,6
end macro

macro invd?
	db 0Fh,8
end macro

macro wbinvd?
	db 0Fh,9
end macro

macro wrmsr?
	db 0Fh,30h
end macro

macro wrmsrq?
	if x86.mode = 64
		db 48h
		db 0Fh,30h
	else
		err 'instruction requires long mode'
	end if
end macro

macro rdtsc?
	db 0Fh,31h
end macro

macro rdmsr?
	db 0Fh,32h
end macro

macro rdmsrq?
	if x86.mode = 64
		db 48h
		db 0Fh,32h
	else
		err 'instruction requires long mode'
	end if
end macro

macro rdpmc?
	db 0Fh,33h
end macro

macro sysenter?
	db 0Fh,34h
end macro

macro sysexit?
	db 0Fh,35h
end macro

macro sysexitq?
	if x86.mode = 64
		db 48h
		db 0Fh,35h
	else
		err 'instruction requires long mode'
	end if
end macro

macro cpuid?
	db 0Fh,0A2h
end macro

macro rsm?
	db 0Fh,0AAh
end macro

macro arpl? dest*,src*
	if x86.mode < 64
		x86.parse_operand @dest,dest
		x86.parse_operand @src,src
		local size
		if @dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
			err 'operand sizes do not match'
		else
			size = @dest.size or @src.size
		end if
		if @src.type = 'reg' & (@dest.type = 'mem' | @dest.type = 'reg')
			x86.select_operand_prefix @dest,size
			x86.store_instruction <63h>,@dest,@src.rm
		else
			err 'invalid combination of operands'
		end if
	else
		err 'illegal instruction'
	end if
end macro

iterate <instr,ext,postbyte>, lldt,0,2, ltr,0,3, verr,0,4, verw,0,5, lmsw,1,6
	macro instr? dest*
		x86.parse_operand @dest,dest
		if @dest.size <> 0 & @dest.size <> 2
			err 'invalid operand size'
		end if
		if @dest.type = 'reg' | @dest.type = 'mem'
			x86.store_instruction <0Fh,ext>,@dest,postbyte
		else
			err 'invalid operand'
		end if
	end macro
end iterate

iterate <instr,ext,postbyte>, sldt,0,0, str,0,1, smsw,1,4
	macro instr? dest*
		x86.parse_operand @dest,dest
		if @dest.type = 'reg'
			x86.select_operand_prefix @dest,@dest.size
			x86.store_instruction <0Fh,ext>,@dest,postbyte
		else if @dest.type = 'mem'
			if @dest.size <> 0 & @dest.size <> 2
				err 'invalid operand size'
			end if
			x86.store_instruction <0Fh,ext>,@dest,postbyte
		else
			err 'invalid operand'
		end if
	end macro
end iterate

iterate <instr,postbyte>, lgdt,2, lidt,3, sgdt,0, sidt,1
	macro instr? dest*
		x86.parse_operand @dest,dest
		if @dest.size <> 0 & ( (x86.mode < 64 & @dest.size <> 5 & @dest.size <> 6) | (x86.mode = 64 & @dest.size <> 10) )
			err 'invalid operand size'
		end if
		if @dest.type = 'mem'
			if @dest.size = 6
				x86.select_operand_prefix @dest,4
			else if @dest.size = 5
				x86.select_operand_prefix @dest,2
			end if
			x86.store_instruction <0Fh,1>,@dest,postbyte
		else
			err 'invalid operand'
		end if
	end macro
end iterate

iterate <instr,ext>, lar,2, lsl,3
	macro instr? dest*,src*
		x86.parse_operand @dest,dest
		x86.parse_operand @src,src
		if @dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg')
			if @src.size and not 2
				err 'invalid operand size'
			end if
			x86.select_operand_prefix @src,@dest.size
			x86.store_instruction <0Fh,ext>,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro
end iterate

macro invlpg? dest*
	x86.parse_operand @dest,dest
	if @dest.type = 'mem'
		x86.store_instruction <0Fh,1>,@dest,7
	else
		err 'invalid operand'
	end if
end macro

include '80387.inc'
include 'ext/sse2.inc'

repeat 8, i:8
	element xmm#i? : SSE.reg + i
end repeat

iterate <instr,postbyte>, fxsave64,0, fxrstor64,1
	macro instr? src*
		if x86.mode = 64
			x86.parse_operand @src,src
			if @src.type = 'mem'
				if @src.size and not 512
					err 'invalid operand size'
				end if
				x86.select_operand_prefix @src,8
				x86.store_instruction <0Fh,0AEh>,@src,postbyte
			else
				err 'invalid operand'
			end if
		else
			err 'instruction requires long mode'
		end if
	end macro
end iterate
