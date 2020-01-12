
if ~ defined MPX

	restore MPX	; this ensures that symbol cannot be forward-referenced
	MPX = 1

	element MPX.bnd

	repeat 4, i:0
		element bnd#i? : MPX.bnd + i
	end repeat

	macro MPX.parse_operand ns,op
		x86.parse_operand ns,op
		if ns.type = 'imm' & ns.size = 0 & ns.imm eq 1 elementof ns.imm & 1 metadataof ns.imm relativeto MPX.bnd
			ns.type = 'bnd'
			ns.mod = 11b
			ns.rm = 1 metadataof ns.imm - MPX.bnd
			ns.size = 0
		end if
	end macro

	macro MPX.parse_sib_operand ns,op&
		match [b=,si], op
			ns.split = 1
			ns.segment_prefix = 0
			ns.prefix = 0
			ns.opcode_prefix = 0
			ns.rex_prefix = 0
			ns.size = 0
			ns.type = 'mem'
			ns.base_part = +b
			ns.index_part = +si
			if x86.mode = 64
				ns.mode = 64
				ns.address_registers_type = x86.r64
			else
				ns.mode = 32
				ns.address_registers_type = x86.r32
			end if
			ns.base_registers = 0
			ns.index_registers = 0
			repeat elementsof ns.base_part
				if % metadataof ns.base_part relativeto x86.r16 | % metadataof ns.base_part relativeto x86.r32 | % metadataof ns.base_part relativeto x86.r64 | % metadataof ns.address relativeto x86.ip
					ns.base_registers = ns.base_registers + % elementof ns.base_part * % scaleof ns.base_part
				end if
			end repeat
			repeat elementsof ns.index_part
				if % metadataof ns.index_part relativeto x86.r16 | % metadataof ns.index_part relativeto x86.r32 | % metadataof ns.index_part relativeto x86.r64 | % metadataof ns.address relativeto x86.ip
					ns.index_registers = ns.index_registers + % elementof ns.index_part * % scaleof ns.index_part
				end if
			end repeat
			ns.displacement = ns.base_part - ns.base_registers + ns.index_part - ns.index_registers
			if ns.index_registers eq 0
				ns.index = 4
				ns.scale = 1
			else if elementsof ns.index_registers = 1 & 1 metadataof ns.index_registers relativeto ns.address_registers_type & 1 metadataof ns.index_registers - ns.address_registers_type <> 4
				ns.index = 1 metadataof ns.index_registers - ns.address_registers_type
				ns.scale = 1 scaleof ns.index_registers
			else
				err 'invalid address'
			end if
			if ns.base_registers eq 0
				ns.rm = 4
				ns.base = 5
				ns.index_only = 1
			else if ns.base_registers eq 1 elementof ns.base_registers & 1 metadataof ns.base_registers relativeto ns.address_registers_type
				ns.base = 1 metadataof ns.base_registers - ns.address_registers_type
				if ns.index = 4 & ns.base <> 4
					ns.rm = ns.base
				else
					ns.rm = 4
				end if
				ns.index_only = 0
			else
				err 'invalid address'
			end if
			ns.auto_relative = 0
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
				end if
			end if
		else
			ns.split = 0
			x86.parse_operand ns,op
		end match
	end macro

	macro bndmk? dest*,src*&
		MPX.parse_operand @dest,dest
		MPX.parse_sib_operand @src,src
		if @dest.type = 'bnd' & @src.type = 'mem'
			if @src.split & ~ 0 scaleof @src.base_part eq 0
				err 'invalid base address'
			end if
			if (x86.mode = 64 & @src.size and not 8) | (x86.mode < 64 & @src.size and not 4)
				err 'invalid operand size'
			end if
			@src.opcode_prefix = 0F3h
			x86.store_instruction <0Fh,1Bh>,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro bndmov? dest*,src*
		MPX.parse_operand @dest,dest
		MPX.parse_operand @src,src
		if @dest.type = 'bnd' & (@src.type = 'bnd' | @src.type = 'mem')
			if (x86.mode = 64 & @src.size and not 16) | (x86.mode < 64 & @src.size and not 8)
				err 'invalid operand size'
			end if
			if @src.type = 'mem' & @src.mode <> x86.mode
				err 'invalid address'
			end if
			@src.opcode_prefix = 66h
			x86.store_instruction <0Fh,1Ah>,@src,@dest.rm
		else if @dest.type = 'mem' & @src.type = 'bnd'
			if (x86.mode = 64 & @dest.size and not 16) | (x86.mode < 64 & @dest.size and not 8)
				err 'invalid operand size'
			end if
			if @dest.type = 'mem' & @dest.mode <> x86.mode
				err 'invalid address'
			end if
			@dest.opcode_prefix = 66h
			x86.store_instruction <0Fh,1Bh>,@dest,@src.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	iterate <instr,prefix,ext>, bndcl,0F3h,1Ah, bndcu,0F2h,1Ah, bndcn,0F2h,1Bh

		macro instr? dest*,src*
			MPX.parse_operand @dest,dest
			x86.parse_operand @src,src
			if @dest.type = 'bnd' & (@src.type = 'reg' | @src.type = 'mem')
				if (x86.mode = 64 & @src.size and not 8) | (x86.mode < 64 & @src.size and not 4)
					err 'invalid operand size'
				end if
				if @src.type = 'mem' & @src.mode <> x86.mode
					err 'invalid address'
				end if
				@src.opcode_prefix = prefix
				x86.store_instruction <0Fh,ext>,@src,@dest.rm
			else
				err 'invalid operand'
			end if
		end macro

	end iterate

	macro bndldx? dest*,src*&
		MPX.parse_operand @dest,dest
		MPX.parse_sib_operand @src,src
		if @dest.type = 'bnd' & @src.type = 'mem'
			if @src.scale > 1 | ( @src.split & ~ 0 scaleof @src.index_part eq 0 )
				err 'invalid index'
			end if
			x86.store_instruction <0Fh,1Ah>,@src,@dest.rm
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro bndstx? operands*&
		match [dest] =, src, operands
			MPX.parse_sib_operand @dest,[dest]
			MPX.parse_operand @src,src
			if @dest.type = 'mem' & @src.type = 'bnd'
				if @dest.scale > 1 | ( @dest.split & ~ 0 scaleof @dest.index_part eq 0 )
					err 'invalid index'
				end if
				x86.store_instruction <0Fh,1Bh>,@dest,@src.rm
			else
				err 'invalid combination of operands'
			end if
		else
			err 'invalid combination of operands'
		end match
	end macro

end if
