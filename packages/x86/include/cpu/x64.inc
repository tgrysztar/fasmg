
define x86 x86

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


calminstruction calminstruction?.initsym? var*, val&
	publish var, val
end calminstruction

calminstruction calminstruction?.asm? line&
	local	name, i
	initsym name, name.0
	match	name.i, name
	compute i, i+1
	arrange name, name.i
	publish name, line
	arrange line, =assemble name
	assemble line
end calminstruction

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
	match	<v>, v
	jyes	symbolic
    append:
	arrange instruction, instruction=,v
	jump	convert
    numeric:
	compute v, v
    symbolic:
	local proxy, base, i
	initsym base, proxy
	initsym i, 0
	compute i, i+1
	arrange proxy, base.i
	publish proxy, v
	arrange instruction, instruction=,proxy
	jump	convert
    ready:
	assemble instruction
end calminstruction


x86.emit.byte = 1
calminstruction (name) ?! flag&
	publish flag, x86.emit.byte
end calminstruction
if 0
	word	x86.emit.word
	dword	x86.emit.dword
	qword	x86.emit.qword
end if
restruc ?

if defined x86.emit.word
	purge word?
	calminstruction word? value*
		emit	2, value
	end calminstruction
end if
if defined x86.emit.dword
	purge dword?
	calminstruction dword? value*
		emit	4, value
	end calminstruction
end if
if defined x86.emit.qword
	purge qword?
	calminstruction qword? value*
		emit	8, value
	end calminstruction
end if


define @dest @dest
define @src @src
define @src2 @src2
define @aux @aux

iterate context, @dest,@src,@src2,@aux

	namespace context

		iterate name,	size, type, segment_prefix, prefix, opcode_prefix, rex_prefix, \
				imm, unresolved, displacement, displacement_size, auto_relative, \
				address, address_registers, segment, offset, jump_type, \
				mode, mod, rm, \
				scale, index, base
			define name
		end iterate

		calminstruction x86.parse_operand#context operand

			local	i, pre, suf, sym

			compute segment_prefix, 0
			compute prefix, 0
			compute opcode_prefix, 0
			compute rex_prefix, 0

			compute size, 0
			compute displacement_size, 0

			transform operand

			match	pre suf, operand
			jno	no_size_prefix
			transform pre, x86
			jno	no_size_prefix
			match	:size, pre
			jno	no_size_prefix
			arrange operand, suf
		      no_size_prefix:

			match	[address], operand
			jyes	memory_operand
			match	=ptr? address, operand
			jyes	memory_operand
			match	segment:offset, operand
			jyes	far_operand

		  immediate_operand:
			compute type, 'imm'
			compute imm, +operand

			compute unresolved, 0
			check	defined operand
			jyes	operand_resolved
			compute unresolved, 1
		      operand_resolved:

			check	imm eq 1 elementof imm
			jno	operand_ready
			check	1 metadataof (1 metadataof imm) relativeto x86.reg
			jyes	register_operand
			check	1 metadataof imm relativeto x86.sreg
			jyes	segment_register_operand

		      operand_ready:
			exit

		  register_operand:

			compute type, 'reg'
			compute mode, x86.mode
			compute mod, 11b
			compute rm, 1 metadataof imm - 1 elementof (1 metadataof imm)
			check	size & size <> 1 metadataof (1 metadataof imm) - x86.reg
			jyes	operand_sizes_do_not_match
			compute size, 1 metadataof (1 metadataof imm) - x86.reg
			check	rm < 0
			jyes	register_precluding_rex
			check	size = 1 & rm >= 4 & rm < 8
			jyes	register_requiring_rex

			exit

		      register_precluding_rex:
			compute rm, x86.REX_FORBIDDEN - rm
			exit

		      register_requiring_rex:
			compute rm, x86.REX_REQUIRED + rm
			exit

		  segment_register_operand:

			compute type, 'sreg'
			compute mode, x86.mode
			compute mod, 11b
			compute rm, 1 metadataof imm - x86.sreg
			check	size & size <> 2 & size <> 4
			jyes	invalid_operand_size

			exit

		  memory_operand:
			compute type, 'mem'

			match	segment:address, address
			jno	segment_prefix_ok
			check	segment eq 1 elementof segment & 1 metadataof segment relativeto x86.sreg
			jno	invalid_operand
			compute segment, 1 metadataof segment - x86.sreg
			check	segment >= 4
			jyes	segment_prefix_386
			compute segment_prefix, 26h + segment shl 3
			jump	segment_prefix_ok
		      segment_prefix_386:
			compute segment_prefix, 64h + segment-4
		      segment_prefix_ok:

			compute mode, 0

			match	pre suf, address
			jno	no_address_size_prefix
			transform pre, x86
			jno	no_address_size_prefix
			match	:pre, pre
			jno	no_address_size_prefix
			arrange address, suf
			check	pre = 2 | pre = 4 | pre = 8
			jno	invalid_address_size
			compute mode, pre shl 3
		      no_address_size_prefix:

			compute scale, 0
			compute index, 0
			compute base, 0

			check	size
			jyes	size_override
			compute size, sizeof address
		      size_override:

			compute address, address
			compute address_registers, 0
			compute i, 1
		      extract_registers:
			check	i > elementsof address
			jyes	registers_extracted
			check	i metadataof address relativeto x86.r16 | i metadataof address relativeto x86.r32 | i metadataof address relativeto x86.r64 | i metadataof address relativeto x86.ip
			jno	next_term
			compute address_registers, address_registers + i elementof address * i scaleof address
		      next_term:
			compute i, i+1
			jump	extract_registers
		      registers_extracted:
			compute displacement, address - address_registers
			compute auto_relative, 0

			check	address_registers eq 0
			jyes	direct_address
			check	mode & mode <> 0 scaleof (1 metadataof (1 metadataof address_registers)) shl 3 & ~ 1 metadataof address_registers relativeto x86.ip
			jyes	invalid_address
			check	1 metadataof address_registers relativeto x86.r64 | 1 metadataof address_registers relativeto x86.r32
			jyes	address_32bit_64bit
			check	1 metadataof address_registers relativeto x86.r16
			jyes	address_16bit
			check	address_registers eq rip | address_registers eq eip
			jyes	rip_relative_address
			jump	invalid_address

		    rip_relative_address:
			compute mode, 0 scaleof (1 metadataof address_registers) shl 3
			compute mod, 0
			compute rm, 5
			compute displacement_size, 4
			exit

		    direct_address:
			compute mod, 0
			check	x86.mode = 64
			jyes	direct_address_in_long_mode
			check	mode = 0
			jno	mode_ok
			compute mode, x86.mode
			check	mode = 16 & displacement relativeto 0 & displacement >= 10000h
			jno	mode_ok
			compute mode, 32
		      mode_ok:
			check	mode = 16
			jyes	direct_address_16bit
		      direct_address_32bit:
			compute rm, 5
			compute displacement_size, 4
			exit
		      direct_address_16bit:
			compute rm, 6
			compute displacement_size, 2
			exit

		      direct_address_in_long_mode:
			compute displacement_size, 4
			check	mode = 0 & segment_prefix < 64h
			jyes	auto_relative_address
			check	mode = 16
			jyes	invalid_address_size
			compute rm, 4
			compute base, 5
			compute index, 4
			compute scale, 1
			check	mode = 32
			jno	direct_address_displacement_ready
			check	 ~ displacement relativeto 0 | displacement >= 100000000h | displacement < -100000000h
			jyes	address_out_of_range
			compute displacement, displacement and 0FFFFFFFFh
		      direct_address_displacement_ready:
			check	displacement relativeto 0 & displacement > 7FFFFFFFh & displacement < 100000000h
			jyes	direct_address_switch_to_32bit
			compute mode, 64
			compute displacement_size, 8
			exit
		      direct_address_switch_to_32bit:
			compute mode, 32
			exit

		     auto_relative_address:
			compute mode, 64
			compute rm, 5
			compute auto_relative, 1
			exit

		    address_16bit:
			compute mode, 16

			check	address_registers relativeto bx+si
			jyes	rm_0
			check	address_registers relativeto bx+di
			jyes	rm_1
			check	address_registers relativeto bp+si
			jyes	rm_2
			check	address_registers relativeto bp+di
			jyes	rm_3
			check	address_registers relativeto si
			jyes	rm_4
			check	address_registers relativeto di
			jyes	rm_5
			check	address_registers relativeto bp
			jyes	rm_6
			check	address_registers relativeto bx
			jyes	rm_7
			jump	invalid_address

		      rm_0:
			compute rm, 0
			jump	rm_ok
		      rm_1:
			compute rm, 1
			jump	rm_ok
		      rm_2:
			compute rm, 2
			jump	rm_ok
		      rm_3:
			compute rm, 3
			jump	rm_ok
		      rm_4:
			compute rm, 4
			jump	rm_ok
		      rm_5:
			compute rm, 5
			jump	rm_ok
		      rm_6:
			compute rm, 6
			jump	rm_ok
		      rm_7:
			compute rm, 7
		      rm_ok:

			check	displacement relativeto 0
			jno	displacement_16bit
			check	displacement = 0 & rm <> 6
			jyes	displacement_empty
			check	displacement<80h & displacement>=-80h
			jyes	displacement_8bit
			check	displacement-10000h>=-80h & displacement<10000h
			jyes	displacement_8bit_wrap_16bit
		      displacement_16bit:
			compute displacement_size, 2
			compute mod, 2
			exit
		      displacement_empty:
			compute displacement_size, 0
			compute mod, 0
			exit
		      displacement_8bit_wrap_16bit:
			compute displacement, displacement-10000h
		      displacement_8bit:
			compute displacement_size, 1
			compute mod, 1
			exit

		    address_32bit_64bit:

			local	address_registers_type
			check	1 metadataof address_registers relativeto x86.r64
			jyes	address_64bit
		      address_32bit:
			compute mode, 32
			compute address_registers_type, x86.r32
			jump	check_address_registers
		      address_64bit:
			compute mode, 64
			compute address_registers_type, x86.r64
		      check_address_registers:
			check	2 scaleof address_registers = 0
			jyes	one_register
			check	3 scaleof address_registers = 0 & 2 metadataof address_registers relativeto address_registers_type
			jyes	two_registers
			jump	invalid_address

		      one_register:
			compute scale, 1 scaleof address_registers
			compute base, 1 metadataof address_registers - address_registers_type
			check	scale = 1
			jyes	one_register_unscaled
			check	base <> 4 & (scale = 4 | scale = 8)
			jyes	one_register_scaled
			check	base <> 4 & (scale = 2 | scale = 3 | scale = 5 | scale = 9)
			jyes	one_register_split
			jump	invalid_address
		      one_register_unscaled:
			check	base and 111b = 4
			jyes	one_register_unscaled_in_sib
			compute rm, base
			jump	setup_displacement
		      one_register_unscaled_in_sib:
			compute rm, 4
			compute index, 4
			jump	setup_displacement
		      one_register_scaled:
			compute rm, 4
			compute index, base
			compute base, 5
			jump	index_only
		      one_register_split:
			compute rm, 4
			compute index, base
			compute scale, scale - 1
			jump	setup_displacement
		      two_registers:
			compute rm,4
			check	1 scaleof address_registers = 1
			jyes	base_first
			check	2 scaleof address_registers = 1
			jyes	base_second
			jump	invalid_address
		      base_first:
			compute base, 1 metadataof address_registers - address_registers_type
			compute index, 2 metadataof address_registers - address_registers_type
			compute scale, 2 scaleof address_registers
			jump	process_sib
		      base_second:
			compute base, 2 metadataof address_registers - address_registers_type
			compute index, 1 metadataof address_registers - address_registers_type
			compute scale, 1 scaleof address_registers
		      process_sib:
			check	index = 4
			jyes	forbidden_index
			check	(x86.mode <> 64 & segment_prefix = 36h) & index = 5 & scale = 1
			jyes	switch_to_index
			check	(x86.mode = 64 | segment_prefix = 3Eh) & base = 5 & scale = 1
			jyes	switch_to_base
			check	scale and (scale-1) | scale > 8
			jyes	invalid_address
			jump	setup_displacement
		      forbidden_index:
			check	scale = 1
			jno	invalid_address
			compute index, base
			compute base, 4
			jump	setup_displacement
		      switch_to_index:
			compute index, base
			compute base,5
			jump	setup_displacement
		      switch_to_base:
			compute base, index
			compute index, 5
			jump	setup_displacement

		      setup_displacement:
			check	displacement relativeto 0
			jno	displacement_32bit
			check	displacement = 0 & rm and 111b <> 5 & (rm <> 4 | base and 111b <> 5)
			jyes	displacement_empty
			check	displacement < 80h & displacement >= -80h
			jyes	displacement_8bit
			check	displacement - 1 shl mode >= -80h & displacement < 1 shl mode
			jyes	displacement_8bit_wrap
			check	(x86.mode = 64 | segment_prefix = 3Eh) & base = 5 & index = 5 & scale = 1
			jno	displacement_32bit
			compute scale, 2
			jump	index_only
		      displacement_32bit:
			compute displacement_size, 4
			compute mod, 2
			exit
		      displacement_8bit_wrap:
			compute displacement, displacement - 1 shl mode
			jump	displacement_8bit
		      index_only:
			compute displacement_size, 4
			compute mod, 0
			exit

		  far_operand:
			compute type, 'far'

			check	size & size <> 4 & size <> 6 & size <> 10
			jyes	operand_sizes_do_not_match

			exit

		  invalid_operand:
			err	'invalid operand'
			exit
		  invalid_operand_size:
			err	'invalid operand size'
			exit
		  operand_sizes_do_not_match:
			err	'operand sizes do not match'
			exit
		  invalid_address:
			err	'invalid address'
			exit
		  invalid_address_size:
			err	'invalid address size'
			exit
		  address_out_of_range:
			err	'address out of range'
			exit

		end calminstruction

		calminstruction x86.parse_jump_operand#context operand

			match	=far? operand, operand
			jyes	far_jump
			match	=near? operand, operand
			jyes	near_jump
			match	=short? operand, operand
			jyes	short_jump
			compute jump_type, ''
			jump	parse_operand
		    far_jump:
			compute jump_type, 'far'
			jump	parse_operand
		    near_jump:
			compute jump_type, 'near'
			jump	parse_operand
		    short_jump:
			compute jump_type, 'short'

		    parse_operand:

			call	x86.parse_operand#context, operand

			check	type = 'imm'
			jno	done

			check	size = 0
			jno	verify_target_address

			compute size, x86.mode shr 3

		    verify_target_address:

			check	imm relativeto 0
			jno	done
			check	imm < 0
			jyes	negative
			check	imm >= 1 shl (size*8)
			jno	done
		    out_of_range:
			err	'value out of range'
			exit
		    negative:
			check	imm < - 1 shl (size*8-1)
			jyes	out_of_range
			compute imm, imm and (1 shl (size*8) - 1)

		    done:

		end calminstruction

		calminstruction x86.select_operand_prefix#context size*

			check	(size = 2 & x86.mode <> 16) | (size = 4 & x86.mode = 16)
			jyes	prefix_66h
			check	size = 8
			jyes	prefix_48h
			check	size <> 0 & size <> 2 & size <> 4
			jyes	invalid_size
			exit

		    prefix_66h:
			compute prefix, 66h
			exit

		    prefix_48h:
			compute prefix, 48h
			exit

		    invalid_size:
			err	'invalid operand size'

		end calminstruction

		calminstruction x86.store_instruction#context opcode*,reg*,imm_size:0,immediate

			check	segment_prefix
			jno	segment_prefix_ok

			check	mode = 64
			jyes	segment_in_long_mode
			check	mode = 16 & ( rm = 2 | rm = 3 | ( mod > 0 & rm = 6 ) )
			jyes	ss_segment_default
			check	mode = 32 & ( ( mod > 0 & rm = 5 ) | ( rm = 4 & base = 4 ) | ( mod > 0 & rm = 4 & base = 5 ) )
			jyes	ss_segment_default

		    ds_segment_default:
			check	segment_prefix = 3Eh
			jyes	segment_prefix_ok
			jump	store_segment_prefix
		    ss_segment_default:
			check	segment_prefix = 36h
			jyes	segment_prefix_ok
			jump	store_segment_prefix
		    segment_in_long_mode:
			check	segment_prefix < 64h
			jyes	segment_prefix_ok
		    store_segment_prefix:
			emit	1, segment_prefix
		    segment_prefix_ok:

			check	mod <> 11b & mode <> x86.mode
			jno	addressing_prefix_ok
			check	mode = 64 | (mode = 16 & x86.mode = 64)
			jno	store_addressing_prefix
			err	'illegal addressing mode'
		    store_addressing_prefix:
			emit	1, 67h
		    addressing_prefix_ok:

			check	(reg or rm) and x86.REX_REQUIRED
			jno	rex_1
			compute rex_prefix, rex_prefix or 40h
		    rex_1:
			check	rm and 1000b | (mod <> 11b & mode > 16 & rm = 4 & base and 1000b)
			jno	rex_2
			compute rex_prefix, rex_prefix or 41h
		    rex_2:
			check	mod <> 11b & mode > 16 & rm = 4 & index and 1000b
			jno	rex_4
			compute rex_prefix, rex_prefix or 42h
		    rex_4:
			check	reg and 1000b
			jno	rex_8
			compute rex_prefix, rex_prefix or 44h
		    rex_8:
			check	prefix = 48h
			jno	operand_prefix
			compute rex_prefix, rex_prefix or 48h
			jump	operand_prefix_ok

		    operand_prefix:
			check	prefix
			jno	operand_prefix_ok
			emit	1, prefix
		    operand_prefix_ok:

			check	opcode_prefix
			jno	opcode_prefix_ok
			emit	1, opcode_prefix
		    opcode_prefix_ok:

			check	rex_prefix
			jno	rex_prefix_ok
			check	x86.mode < 64
			jyes	instruction_requires_long_mode
			check	(reg or rm) and x86.REX_FORBIDDEN
			jno	store_rex_prefix
			err	'disallowed combination of registers'
			jump	store_rex_prefix
		    instruction_requires_long_mode:
			err	'instruction requires long mode'
		    store_rex_prefix:
			emit	1, rex_prefix
		    rex_prefix_ok:

			asm	db opcode
			emit	1, mod shl 6 + (reg and 111b) shl 3 + rm and 111b

			check	mod <> 11b & rm = 4 & mode <> 16
			jno	sib_ok
			emit	1, (bsf scale) shl 6 + (index and 111b) shl 3 + base and 111b
		    sib_ok:

			check	displacement_size = 1
			jyes	displacement_8bit
			check	displacement_size = 2
			jyes	displacement_16bit
			check	displacement_size = 4 | displacement_size = 8
			jno	displacement_ok

			compute displacement, displacement

			check	auto_relative
			jno	auto_relative_ok
			check	imm_size < 8
			jyes	adjust_auto_relative_displacement
			compute displacement, displacement - ($ + 4 + 4)
			jump	auto_relative_ok
		      adjust_auto_relative_displacement:
			compute displacement, displacement - ($ + 4 + imm_size)
		      auto_relative_ok:

			check	mode = 64 & displacement relativeto 0
			jno	displacement_ready
			check	displacement - 1 shl 64 >= -80000000h & displacement < 1 shl 64
			jyes	adjust_displacement_wrap
			check	displacement >= -80000000h & displacement < 80000000h
			jyes	displacement_ready
			err	'address value out of signed range'
		      adjust_displacement_wrap:
			compute displacement, displacement - 1 shl 64
		      displacement_ready:

			call	dword, displacement

			jump	displacement_ok
		    displacement_16bit:
			call	word, displacement
			jump	displacement_ok
		    displacement_8bit:
			emit	1, displacement
		    displacement_ok:

			check	imm_size = 1
			jyes	immediate_8bit
			check	imm_size = 2
			jyes	immediate_16bit
			check	imm_size = 4
			jyes	immediate_32bit
			check	imm_size = 8
			jno	immediate_ok
			call	x86.simm32, immediate
			jump	immediate_ok
		    immediate_32bit:
			compute imm, +immediate
			call	dword, imm
			jump	immediate_ok
		    immediate_16bit:
			compute imm, +immediate
			call	word, imm
			jump	immediate_ok
		    immediate_8bit:
			compute imm, +immediate
			emit	1, imm
		    immediate_ok:

		end calminstruction

	end namespace

end iterate

calminstruction x86.store_operand_prefix size*, reg:0

	local	rex_prefix

	compute rex_prefix, 0

	check	(size = 2 & x86.mode <> 16) | (size = 4 & x86.mode = 16)
	jyes	prefix_66h
	check	size = 8
	jyes	rex_8
	check	size <> 0 & size <> 2 & size <> 4
	jno	check_register
	err	'invalid operand size'
	jump	check_register

    prefix_66h:
	emit	1, 66h
	jump	check_register

    rex_8:
	compute rex_prefix, 48h

    check_register:
	check	reg and 1000b
	jyes	rex_1
	check	reg and x86.REX_REQUIRED
	jno	rex_ready
	compute rex_prefix, rex_prefix or 40h
	jump	rex_ready
    rex_1:
	compute rex_prefix, rex_prefix or 41h

    rex_ready:
	check	rex_prefix
	jno	rex_prefix_ok
	check	x86.mode < 64
	jyes	instruction_requires_long_mode
	check	reg and x86.REX_FORBIDDEN
	jno	store_rex_prefix
	err	'disallowed combination of registers'
	jump	store_rex_prefix
    instruction_requires_long_mode:
	err	'instruction requires long mode'
    store_rex_prefix:
	emit	1, rex_prefix
    rex_prefix_ok:

end calminstruction

calminstruction x86.simm32 immediate*
	compute immediate, +immediate
	check	immediate eqtype 0.0
	jno	check_value
	asm	virtual at 0
	asm	emit 8: immediate
	asm	load immediate:8 from 0
	asm	end virtual
	compute immediate, +immediate
      check_value:
	check	immediate relativeto 0
	jyes	check_range
	call	dword, immediate
	exit
      check_range:
	check	immediate - 1 shl 64 >= -80000000h & immediate < 1 shl 64
	jyes	wrap
	check	immediate >= 80000000h | immediate < -80000000h
	jyes	out_of_range
	emit	4, immediate
	exit
      wrap:
	emit	4, immediate - 1 shl 64
	exit
      out_of_range:
	err	'immediate value out of signed range'
end calminstruction


iterate <instr,basecode>, add,0, or,8, adc,10h, sbb,18h, and,20h, sub,28h, xor,30h, cmp,38h

	calminstruction instr? dest*,src*

		call	x86.parse_operand@dest, dest
		call	x86.parse_operand@src, src

		local	opcode, rm, size

		compute opcode, basecode

		check	@dest.size = 0 & @src.size = 0
		jyes	operand_size_not_specified
		check	@dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
		jyes	operand_sizes_do_not_match

		compute size, @dest.size or @src.size

	    main:

		check	@src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		jyes	reg_rm
		check	@src.type = 'mem' & @dest.type = 'reg'
		jyes	rm_reg
		check	@src.type = 'imm' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		jyes	rm_imm

		err	'invalid combination of operands'
		exit

	    operand_size_not_specified:
		err	'operand size not specified'
		compute size, 0
		jump	main

	    operand_sizes_do_not_match:
		err	'operand sizes do not match'
		compute size, 0
		jump	main

	    reg_rm:
		check	size > 1
		jno	reg_rm_store
		call	x86.select_operand_prefix@dest, size
		compute opcode, opcode + 1
	    reg_rm_store:
		call	x86.store_instruction@dest, opcode,@src.rm
		exit

	    rm_reg:
		compute opcode, opcode + 2
		check	size > 1
		jno	rm_reg_store
		call	x86.select_operand_prefix@src, size
		compute opcode, opcode + 1
	    rm_reg_store:
		call	x86.store_instruction@src, opcode,@dest.rm
		exit

	    rm_imm:
		check	size > 1
		jyes	rm_imm_word
		check	@dest.type = 'reg' & @dest.rm = 0
		jyes	al_imm

		compute opcode, opcode shr 3
		xcall	x86.store_instruction@dest, (80h),opcode,byte,@src.imm
		exit

	    al_imm:
		emit	1, opcode+4
		emit	1, @src.imm
		exit

	    rm_imm_word:

		call	x86.select_operand_prefix@dest, size

		check	@src.imm eqtype 0.0
		jno	rm_imm_optimize

		asm	virtual at 0
		asm	emit size: @src.imm
		asm	load @src.imm:size from 0
		asm	end virtual
		compute @src.imm, +@src.imm

	    rm_imm_optimize:
		check	@src.imm relativeto 0 & @src.imm < 80h & @src.imm >= -80h
		jyes	rm_simm
		check	@src.imm relativeto 0 & @src.imm - 1 shl (size shl 3) >= -80h & @src.imm < 1 shl (size shl 3)
		jyes	rm_simm_wrap
		check	@dest.type = 'reg' & @dest.rm = 0
		jyes	ax_imm

		compute rm, opcode shr 3
		xcall	x86.store_instruction@dest, (81h),rm,size,@src.imm
		exit

	    ax_imm:
		check	@dest.prefix
		jno	ax_imm_prefix_ok
		emit	1, @dest.prefix
	      ax_imm_prefix_ok:
		emit	1, opcode+5
		check	size = 4
		jyes	imm32
		check	size = 8
		jyes	simm32
		asm	emit size: @src.imm
		exit
	      imm32:
		call	dword, @src.imm
		exit
	      simm32:
		call	x86.simm32, @src.imm
		exit

	    rm_simm_wrap:
		compute @src.imm, @src.imm - 1 shl (size shl 3)

	    rm_simm:
		compute rm, opcode shr 3
		xcall	x86.store_instruction@dest, (83h),rm,byte,@src.imm

	end calminstruction

end iterate

iterate <instr,postbyte>, not,2, neg,3, mul,4, div,6, idiv,7

	calminstruction instr? src*

		call	x86.parse_operand@src, src

		check	@src.size = 0
		jyes	operand_size_not_specified

	    main:
		check	@src.type = 'mem' | @src.type = 'reg'
		jno	invalid_operand
		check	@src.size > 1
		jyes	rm_word

		xcall	x86.store_instruction@src, (0F6h),(postbyte)
		exit

	    rm_word:
		call	x86.select_operand_prefix@src, @src.size
		xcall	x86.store_instruction@src, (0F7h),(postbyte)
		exit

	    operand_size_not_specified:
		err	'operand size not specified'
		jump	main

	    invalid_operand:
		err	'invalid operand'
		exit

	end calminstruction

end iterate

calminstruction mov? dest*,src*

	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src

	local	ext, rm, size

	check	@dest.size = 0 & @src.size = 0 & @dest.type <> 'sreg'
	jyes	operand_size_not_specified
	check	@dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
	jyes	operand_sizes_do_not_match

	compute size, @dest.size or @src.size

    main:

	check	@src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
	jyes	mov_rm_reg
	check	@src.type = 'mem' & @dest.type = 'reg'
	jyes	mov_reg_mem
	check	@src.type = 'imm' & ( @dest.type = 'reg' | @dest.type = 'mem' )
	jyes	mov_rm_imm
	check	@src.type = 'reg' & @dest.type = 'imm'
	jyes	mov_creg_reg
	check	@src.type = 'sreg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
	jyes	mov_rm_sreg
	check	@dest.type = 'sreg' & @dest.rm <> 1 & ( @src.type = 'reg' | @src.type = 'mem' )
	jyes	mov_sreg_rm

    invalid_combination_of_operands:
	err	'invalid combination of operands'
	exit

    operand_size_not_specified:
	err	'operand size not specified'
	compute size, 0
	jump	main

    operand_sizes_do_not_match:
	err	'operand sizes do not match'
	compute size, 0
	jump	main

    mov_rm_reg:
	check	@src.type = 'reg' & @dest.type = 'mem' & @src.rm = 0 & @dest.address_registers eq 0 & \
		~ @dest.auto_relative & ( @dest.displacement_size <> 8 | ~ @dest.displacement relativeto 0 | @dest.displacement and 0FFFFFFFF80000000h <> 0FFFFFFFF80000000h)
	jyes	mov_dirmem_ax
	check	size > 1
	jno	mov_reg_rm_8bit
	call	x86.select_operand_prefix@dest, size
	xcall	x86.store_instruction@dest, (89h),@src.rm
	exit
      mov_reg_rm_8bit:
	xcall	x86.store_instruction@dest, (88h),@src.rm
	exit

    mov_reg_mem:
	check	@src.type = 'mem' & @dest.type = 'reg' & @dest.rm = 0 & @src.address_registers eq 0 & \
		~ @src.auto_relative & ( @src.displacement_size <> 8 | ~ @src.displacement relativeto 0 | @src.displacement and 0FFFFFFFF80000000h <> 0FFFFFFFF80000000h)
	jyes	mov_ax_dirmem
	check	size > 1
	jno	mov_mem_reg_8bit
	call	x86.select_operand_prefix@src, size
	xcall	x86.store_instruction@src, (8Bh),@dest.rm
	exit
      mov_mem_reg_8bit:
	xcall	x86.store_instruction@src, (8Ah),@dest.rm
	exit

    mov_dirmem_ax:
	check	x86.mode = 64
	jyes	mov_dirmem_ax_longmode
	check	@dest.segment_prefix = 0 | @dest.segment_prefix = 3Eh
	jyes	dest_seg_ok
	emit	1, @dest.segment_prefix
      dest_seg_ok:
	check	@dest.mode = x86.mode
	jyes	dest_addr_size_ok
	emit	1, 67h
      dest_addr_size_ok:
	check	size > 1
	jno	mov_dirmem_al
	call	x86.store_operand_prefix, size
	emit	1, 0A3h
	jump	dest_displacement
      mov_dirmem_al:
	emit	1, 0A2h
      dest_displacement:
	check	@dest.mode = 16
	jyes	dest_displacement_16bit
	check	@dest.displacement_size = 8
	jyes	dest_displacement_64bit
	call	dword, @dest.address
	exit
      dest_displacement_16bit:
	call	word, @dest.address
	exit
      dest_displacement_64bit:
	call	qword, @dest.address
	exit
      mov_dirmem_ax_longmode:
	check	@dest.displacement_size = 8 & @dest.displacement relativeto 0 & @dest.displacement >= 0 & @dest.displacement < 100000000h
	jno	dest_displacement_size_ok
	compute @dest.displacement_size, 4
      dest_displacement_size_ok:
	check	@dest.segment_prefix & @dest.segment_prefix >= 64h
	jno	dest_longmode_seg_ok
	emit	1, @dest.segment_prefix
      dest_longmode_seg_ok:
	check	@dest.mode = 16
	jyes	illegal_addressing_mode
	check	@dest.displacement_size = 8
	jyes	dest_addr_size_ok
	emit	1, 67h
	jump	dest_addr_size_ok

    mov_ax_dirmem:
	check	x86.mode = 64
	jyes	mov_ax_dirmem_longmode
	check	@src.segment_prefix = 0 | @src.segment_prefix = 3Eh
	jyes	src_seg_ok
	emit	1, @src.segment_prefix
      src_seg_ok:
	check	@src.mode = x86.mode
	jyes	src_addr_size_ok
	emit	1, 67h
      src_addr_size_ok:
	check	size > 1
	jno	mov_al_dirmem
	call	x86.store_operand_prefix, size
	emit	1, 0A1h
	jump	src_displacement
      mov_al_dirmem:
	emit	1, 0A0h
      src_displacement:
	check	@src.mode = 16
	jyes	src_displacement_16bit
	check	@src.displacement_size = 8
	jyes	src_displacement_64bit
	call	dword, @src.address
	exit
      src_displacement_16bit:
	call	word, @src.address
	exit
      src_displacement_64bit:
	call	qword, @src.address
	exit
      mov_ax_dirmem_longmode:
	check	@src.displacement_size = 8 & @src.displacement relativeto 0 & @src.displacement >= 0 & @src.displacement < 100000000h
	jno	src_displacement_size_ok
	compute @src.displacement_size, 4
      src_displacement_size_ok:
	check	@src.segment_prefix & @src.segment_prefix >= 64h
	jno	src_longmode_seg_ok
	emit	1, @src.segment_prefix
      src_longmode_seg_ok:
	check	@src.mode = 16
	jyes	illegal_addressing_mode
	check	@src.displacement_size = 8
	jyes	src_addr_size_ok
	emit	1, 67h
	jump	src_addr_size_ok

    mov_rm_imm:
	check	@dest.type = 'mem'
	jyes	mov_mem_imm
	check	@dest.type = 'reg' & 1 metadataof (1 metadataof @src.imm) relativeto x86.creg & @src.imm relativeto 1 elementof @src.imm
	jyes	mov_reg_creg

    mov_reg_imm:
	check	size > 1
	jno	mov_reg_imm_8bit
	check	@src.imm eqtype 0.0
	jno	mov_reg_imm_optimize
	asm	virtual at 0
	asm	emit size: @src.imm
	asm	load @src.imm:size from 0
	asm	end virtual
	compute @src.imm, +@src.imm
      mov_reg_imm_optimize:
	check	size = 8 & @src.imm relativeto 0 & @src.imm < 80000000h & @src.imm >= -80000000h
	jyes	mov_reg_simm
	check	size = 8 & @src.imm relativeto 0 & @src.imm - 1 shl 64 < 80000000h & @src.imm - 1 shl 64 >= -80000000h
	jyes	mov_reg_simm_wrap
	call	x86.store_operand_prefix, size,@dest.rm
	emit	1, 0B8h + @dest.rm and 111b
	check	size = 2
	jyes	src_imm_16bit
	check	size = 4
	jyes	src_imm_32bit
	call	qword, @src.imm
	exit
      src_imm_32bit:
	call	dword, @src.imm
	exit
      src_imm_16bit:
	call	word, @src.imm
	exit
      mov_reg_imm_8bit:
	xcall	x86.store_operand_prefix, (0),@dest.rm
	emit	1, 0B0h + @dest.rm and 111b
	emit	1, @src.imm
	exit
      mov_reg_simm_wrap:
	compute @src.imm, @src.imm - 1 shl 64
      mov_reg_simm:
	call	x86.select_operand_prefix@dest, size
	xcall	x86.store_instruction@dest, (0C7h),(0),size,@src.imm
	exit

    mov_mem_imm:
	check	size > 1
	jno	mov_mem_imm_8bit
	call	x86.select_operand_prefix@dest, size
	xcall	x86.store_instruction@dest, (0C7h),(0),size,@src.imm
	exit
      mov_mem_imm_8bit:
	xcall	x86.store_instruction@dest, (0C6h),(0),byte,@src.imm
	exit

    mov_reg_creg:
	check	(x86.mode <> 64 & @dest.size = 4) | (x86.mode = 64 & @dest.size = 8)
	jno	invalid_operand_size
	compute ext, 20h + 1 metadataof (1 metadataof @src.imm) - x86.creg
	compute rm, 1 metadataof @src.imm - 1 elementof (1 metadataof @src.imm)
	xcall	x86.store_instruction@dest, <0Fh,ext>,rm
	exit

    mov_creg_reg:
	check	1 metadataof (1 metadataof @dest.imm) relativeto x86.creg & @dest.imm relativeto 1 elementof @dest.imm
	jno	invalid_combination_of_operands
	check	(x86.mode <> 64 & @src.size = 4) | (x86.mode = 64 & @src.size = 8)
	jno	invalid_operand_size
	compute ext, 22h + 1 metadataof (1 metadataof @dest.imm) - x86.creg
	compute rm, 1 metadataof @dest.imm - 1 elementof (1 metadataof @dest.imm)
	xcall	x86.store_instruction@src, <0Fh,ext>,rm
	exit

    mov_rm_sreg:
	check	@dest.type = 'mem'
	jyes	mov_mem_sreg
      mov_reg_sreg:
	check	size > 1
	jno	invalid_operand_size
	call	x86.select_operand_prefix@dest, size
	jump	mov_rm_sreg_store
      mov_mem_sreg:
	check	size and not 2
	jyes	invalid_operand_size
      mov_rm_sreg_store:
	xcall	x86.store_instruction@dest, (8Ch),@src.rm
	exit

    mov_sreg_rm:
	check	size = 1
	jyes	invalid_operand_size
	xcall	x86.store_instruction@src, (8Eh),@dest.rm
	exit

    invalid_operand_size:
	err	'invalid operand size'
	exit

    illegal_addressing_mode:
	err	'illegal addressing mode'
	exit

end calminstruction

calminstruction test? dest*,src*

	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src

	local	ext, rm, size

	check	@dest.size = 0 & @src.size = 0
	jyes	operand_size_not_specified
	check	@dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
	jyes	operand_sizes_do_not_match

	compute size, @dest.size or @src.size

    main:

	check	@src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
	jyes	test_reg_rm
	check	@src.type = 'mem' & @dest.type = 'reg'
	jyes	test_mem_reg
	check	@src.type = 'imm' & ( @dest.type = 'reg' | @dest.type = 'mem' )
	jyes	test_rm_imm

    invalid_combination_of_operands:
	err	'invalid combination of operands'
	exit

    operand_size_not_specified:
	err	'operand size not specified'
	compute size, 0
	jump	main

    operand_sizes_do_not_match:
	err	'operand sizes do not match'
	compute size, 0
	jump	main

    test_reg_rm:
	check	size > 1
	jno	test_reg_rm_8bit
	call	x86.select_operand_prefix@dest, size
	xcall	x86.store_instruction@dest, (85h),@src.rm
	exit
      test_reg_rm_8bit:
	xcall	x86.store_instruction@dest, (84h),@src.rm
	exit

    test_mem_reg:
	check	size > 1
	jno	test_mem_reg_8bit
	call	x86.select_operand_prefix@src, size
	xcall	x86.store_instruction@src, (85h),@dest.rm
	exit
      test_mem_reg_8bit:
	xcall	x86.store_instruction@src, (84h),@dest.rm
	exit

    test_rm_imm:
	check	size > 1
	jno	test_rm_imm_8bit
	check	@dest.type = 'reg' & @dest.rm = 0
	jyes	test_ax_imm
	call	x86.select_operand_prefix@dest, size
	xcall	x86.store_instruction@dest, (0F7h),(0),size,@src.imm
	exit

      test_ax_imm:
	call	x86.store_operand_prefix, size
	emit	1, 0A9h
	check	size = 2
	jyes	src_imm_16bit
	check	size = 4
	jyes	src_imm_32bit
	call	x86.simm32, @src.imm
	exit
      src_imm_16bit:
	call	word, @src.imm
	exit
      src_imm_32bit:
	call	dword, @src.imm
	exit

      test_rm_imm_8bit:
	check	@dest.type = 'reg' & @dest.rm = 0
	jyes	test_al_imm
	xcall	x86.store_instruction@dest, (0F6h),(0),byte,@src.imm
	exit
      test_al_imm:
	emit	1, 0A8h
	emit	1, @src.imm
	exit

end calminstruction

calminstruction xchg? dest*,src*

	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src

	local	ext, rm, size

	check	@dest.size = 0 & @src.size = 0
	jyes	operand_size_not_specified
	check	@dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
	jyes	operand_sizes_do_not_match

	compute size, @dest.size or @src.size

    main:

	check	@src.type = 'reg' & @dest.type = 'reg'
	jyes	xchg_reg_reg
	check	@src.type = 'reg' & @dest.type = 'mem'
	jyes	xchg_reg_rm
	check	@src.type = 'mem' & @dest.type = 'reg'
	jyes	xchg_rm_reg

    invalid_combination_of_operands:
	err	'invalid combination of operands'
	exit

    operand_size_not_specified:
	err	'operand size not specified'
	compute size, 0
	jump	main

    operand_sizes_do_not_match:
	err	'operand sizes do not match'
	compute size, 0
	jump	main

    xchg_reg_reg:
	check	(@src.rm & @dest.rm) | (size = 4 & @src.rm or @dest.rm = 0)
	jyes	xchg_rm_reg
	check	size > 1
	jno	xchg_rm_reg_8bit
	compute @src.rm, @src.rm or @dest.rm
	call	x86.store_operand_prefix, size,@src.rm
	emit	1, 90h + @src.rm and 111b
	exit

    xchg_reg_rm:
	check	size > 1
	jno	xchg_reg_rm_8bit
	call	x86.select_operand_prefix@dest, size
	xcall	x86.store_instruction@dest, (87h),@src.rm
	exit
      xchg_reg_rm_8bit:
	xcall	x86.store_instruction@dest, (86h),@src.rm
	exit

    xchg_rm_reg:
	check	size > 1
	jno	xchg_rm_reg_8bit
	call	x86.select_operand_prefix@src, size
	xcall	x86.store_instruction@src, (87h),@dest.rm
	exit
      xchg_rm_reg_8bit:
	xcall	x86.store_instruction@src, (86h),@dest.rm
	exit

end calminstruction

iterate <instr,postbyte>, inc,0 ,dec,1

	calminstruction instr? dest*

		call	x86.parse_operand@dest, dest

		check	@dest.size
		jyes	main

		err	'operand size not specified'

	    main:
		check	@dest.type = 'mem' | (x86.mode = 64 & @dest.type = 'reg')
		jyes	inc_rm
		check	@dest.type = 'reg'
		jyes	inc_reg

		err	'invalid operand'
		exit

	    inc_reg:
		check	@dest.size > 1
		jno	inc_rm_8bit
		call	x86.store_operand_prefix, @dest.size
		emit	1, 40h + @dest.rm + postbyte shl 3
		exit

	    inc_rm:
		check	@dest.size > 1
		jno	inc_rm_8bit
		xcall	x86.select_operand_prefix@dest, @dest.size
		xcall	x86.store_instruction@dest, (0FFh),(postbyte)
		exit
	      inc_rm_8bit:
		xcall	x86.store_instruction@dest, (0FEh),(postbyte)

	end calminstruction

end iterate

calminstruction imul? dest*,src&

	local	size

	call	x86.parse_operand@dest, dest

	match	, src
	jyes	imul_rm

	local	src1, src2

	match	src1 =, src2, src
	jyes	imul_second_source

	call	x86.parse_operand@src, src

	check	@dest.size = 0 & @src.size = 0
	jyes	operand_size_not_specified
	check	@dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
	jyes	operand_sizes_do_not_match

	compute size, @dest.size or @src.size

	check	@dest.type = 'reg' & (@src.type = 'reg' | @src.type = 'mem')
	jyes	imul_reg_rm

	check	@src.type = 'imm' & @dest.type = 'reg'
	jno	invalid_combination_of_operands

	compute @aux.type, @src.type
	compute @aux.imm, @src.imm
	compute @src.type, @dest.type
	compute @src.mod, @dest.mod
	compute @src.rm, @dest.rm

	jump	main

    imul_second_source:
	call	x86.parse_operand@src, src1
	call	x86.parse_operand@aux, src2

	check	@dest.size = 0 & @src.size = 0 & @aux.size = 0
	jyes	operand_size_not_specified

	compute size, @dest.size or @src.size or @aux.size

	check	(@dest.size & @dest.size <> size) | (@src.size & @src.size <> size) | (@aux.size & @aux.size <> size)
	jyes	operand_sizes_do_not_match

	jump	main

    operand_size_not_specified:
	err	'operand size not specified'
	compute size, 0
	jump	main

    operand_sizes_do_not_match:
	err	'operand sizes do not match'
	compute size, 0
	jump	main

    main:
	check	@aux.type = 'imm' & ( @src.type = 'mem' | @src.type = 'reg' ) & @dest.type = 'reg'
	jyes	imul_reg_rm_imm

    invalid_combination_of_operands:
	err	'invalid combination of operands'
	exit

    imul_rm:
	check	@dest.size
	jyes	imul_rm_size_ok
	err	'operand size not specified'
      imul_rm_size_ok:
	check	@dest.type = 'mem' | @dest.type = 'reg'
	jno	invalid_combination_of_operands
	check	@dest.size > 1
	jno	imul_rm_8bit
	xcall	x86.select_operand_prefix@dest, @dest.size
	xcall	x86.store_instruction@dest, (0F7h),(5)
	exit
      imul_rm_8bit:
	xcall	x86.store_instruction@dest, (0F6h),(5)
	exit

    imul_reg_rm:
	call	x86.select_operand_prefix@src, size
	xcall	x86.store_instruction@src, <0Fh,0AFh>,@dest.rm
	exit

    imul_reg_rm_imm:
	call	x86.select_operand_prefix@src, size
	check	@aux.imm eqtype 0.0
	jno	imul_reg_rm_imm_optimize
	asm	virtual at 0
	asm	emit size: @aux.imm
	asm	load @aux.imm:size from 0
	asm	end virtual
	compute @aux.imm, +@aux.imm
      imul_reg_rm_imm_optimize:
	check	@aux.imm relativeto 0 & @aux.imm < 80h & @aux.imm >= -80h
	jyes	imul_reg_rm_simm
	check	@aux.imm relativeto 0 & @aux.imm - 1 shl (size shl 3) >= -80h & @aux.imm < 1 shl (size shl 3)
	jyes	imul_reg_rm_simm_wrap
	xcall	x86.store_instruction@src, (69h),@dest.rm,size,@aux.imm
	exit
      imul_reg_rm_simm_wrap:
	compute @aux.imm, @aux.imm - 1 shl (size shl 3)
      imul_reg_rm_simm:
	xcall	x86.store_instruction@src, (6Bh),@dest.rm,byte,@aux.imm
	exit

end calminstruction

calminstruction x86.push_instruction size:0,src*

	call	x86.parse_operand@src, src

	check	size <> 0 & @src.size and not size
	jyes	invalid_operand_size
	compute size, size or @src.size
	check	size = 0 | size = 2 | (size = 4 & x86.mode < 64) | (size = 8 & x86.mode = 64)
	jyes	main

    invalid_operand_size:
	err	'invalid operand size'

    main:
	check	(x86.mode <> 16 & size = 2) | (x86.mode = 16 & size = 4)
	jno	prefix_ready
	compute @src.prefix, 66h
      prefix_ready:

	check	@src.type = 'mem'
	jyes	push_mem

	check	@src.prefix
	jno	prefix_stored
	emit	1, @src.prefix
      prefix_stored:

	check	@src.type = 'reg'
	jyes	push_reg
	check	@src.type = 'sreg'
	jyes	push_sreg
	check	@src.type = 'imm'
	jyes	push_imm

    invalid_operand:
	err	'invalid operand'
	exit

    push_mem:
	xcall	x86.store_instruction@src, (0FFh),(110b)
	exit

    push_reg:
	check	@src.rm and 1000b
	jyes	push_new_reg
	emit	1, 50h + @src.rm
	exit
      push_new_reg:
	emit	1, 41h
	emit	1, 50h + @src.rm and 111b
	exit

    push_sreg:
	check	@src.rm >= 4
	jyes	push_sreg_386
	check	x86.mode = 64
	jyes	invalid_operand
	emit	1, 6 + @src.rm shl 3
	exit
      push_sreg_386:
	emit	1, 0Fh
	emit	1, 0A0h + (@src.rm-4) shl 3
	exit

    push_imm:
	check	size
	jyes	push_imm_size_ok
	check	x86.mode = 16
	jyes	push_imm_16bit
	check	x86.mode = 32
	jyes	push_imm_32bit
	compute size, 8
	jump	push_imm_size_ok
      push_imm_32bit:
	compute size, 4
	jump	push_imm_size_ok
      push_imm_16bit:
	compute size, 2
      push_imm_size_ok:

	check	@src.imm eqtype 0.0
	jno	push_imm_check
	asm	virtual at 0
	asm	emit size: @src.imm
	asm	load @src.imm:size from 0
	asm	end virtual
	compute @src.imm, +@src.imm
      push_imm_check:
	check	size = 8 & @src.imm relativeto 0
	jno	push_imm_optimize
	check	@src.imm - 10000000000000000h >= -80000000h & @src.imm < 10000000000000000h
	jyes	push_imm_wrap
	check	@src.imm >= 80000000h | @src.imm < -80000000h
	jyes	out_of_signed_range
	jump	push_imm_optimize
      push_imm_wrap:
	compute @src.imm, @src.imm - 10000000000000000h
      push_imm_optimize:
	check	@src.imm relativeto 0 & @src.imm < 80h & @src.imm >= -80h
	jyes	push_simm
	check	size = 2 & @src.imm relativeto 0 & @src.imm - 10000h >= -80h & @src.imm < 10000h
	jyes	push_simm_wrap
	check	size = 4 & @src.imm relativeto 0 & @src.imm - 100000000h >= -80h & @src.imm < 100000000h
	jyes	push_simm_wrap
	emit	1, 68h
	check	size = 2
	jyes	src_imm_16bit
	call	dword, @src.imm
	exit
      src_imm_16bit:
	call	word, @src.imm
	exit
      push_simm_wrap:
	compute @src.imm, @src.imm - 1 shl (size shl 3)
      push_simm:
	emit	1, 6Ah
	emit	1, @src.imm
	exit

    out_of_signed_range:
	err	'immediate value out of signed range'
	exit

end calminstruction

calminstruction x86.pop_instruction size:0,dest*

	call	x86.parse_operand@dest, dest

	check	size <> 0 & @dest.size and not size
	jyes	invalid_operand_size
	compute size, size or @dest.size
	check	size = 0 | size = 2 | (size = 4 & x86.mode < 64) | (size = 8 & x86.mode = 64)
	jyes	main

    invalid_operand_size:
	err	'invalid operand size'

    main:
	check	(x86.mode <> 16 & size = 2) | (x86.mode = 16 & size = 4)
	jno	prefix_ready
	compute @dest.prefix, 66h
      prefix_ready:

	check	@dest.type = 'mem'
	jyes	pop_mem

	check	@dest.prefix
	jno	prefix_stored
	emit	1, @dest.prefix
      prefix_stored:

	check	@dest.type = 'reg'
	jyes	pop_reg
	check	@dest.type = 'sreg'
	jyes	pop_sreg

    invalid_operand:
	err	'invalid operand'
	exit

    pop_mem:
	xcall	x86.store_instruction@dest, (8Fh),(0)
	exit

    pop_reg:
	check	@dest.rm and 1000b
	jyes	pop_new_reg
	emit	1, 58h + @dest.rm
	exit
      pop_new_reg:
	emit	1, 41h
	emit	1, 58h + @dest.rm and 111b
	exit

    pop_sreg:
	check	@dest.rm = 1
	jyes	invalid_operand
	check	@dest.rm >= 4
	jyes	pop_sreg_386
	check	x86.mode = 64
	jyes	invalid_operand
	emit	1, 7 + @dest.rm shl 3
	exit
      pop_sreg_386:
	emit	1, 0Fh
	emit	1, 0A1h + (@dest.rm-4) shl 3
	exit

end calminstruction

iterate reg, ax,cx,dx,bx,sp,bp,si,di,r8w,r9w,r10w,r11w,r12w,r13w,r14w,r15w, \
	     eax,ecx,edx,ebx,esp,ebp,esi,edi,r8d,r9d,r10d,r11d,r12d,r13d,r14d,r15d, \
	     rax,rcx,rdx,rbx,rsp,rbp,rsi,rdi,r8,r9,r10,r11,r12,r13,r14,r15, \
	     es,cs,ss,ds,fs,gs
	define x86.compact.reg? {reg}
end iterate

iterate <instr,	size,	handler>,\
	push,	0,	push_instruction,\
	pushw,	2,	push_instruction,\
	pushd,	4,	push_instruction,\
	pushq,	8,	push_instruction,\
	pop,	0,	pop_instruction,\
	popw,	2,	pop_instruction,\
	popd,	4,	pop_instruction,\
	popq,	8,	pop_instruction

	calminstruction instr? operand
		local	head, tail

		match	head tail, operand
		jno	plain
		transform head, x86.compact
		jno	plain
		match	{head}, head
		jno	plain
	loop:
		xcall	x86.handler, (size), head
		match	head tail?, tail
		jno	done
		transform head, x86.compact
		jno	error
		match	{head}, head
		jyes	loop
	error:
		err	'only register operands allowed in compact syntax'
		exit
	plain:
		xcall	x86.handler, (size), operand
	done:
	end calminstruction
end iterate

iterate <instr,opcode>, ret,0C2h, retn,0C2h, retf,0CAh

	calminstruction instr? operand
		match	, operand
		jyes	ret_short
		check	operand
		jno	ret_short
		emit	1, opcode
		call	word, operand
		exit
	    ret_short:
		emit	1, opcode + 1
	end calminstruction

end iterate

iterate <instr,size,opcode>, retw,2,0C2h, retnw,2,0C2h, retd,4,0C2h, retnd,4,0C2h, retfw,2,0CAh, retfd,4,0CAh

	calminstruction instr? operand
		check	x86.mode < 64
		jyes	illegal_instruction
		xcall	x86.store_operand_prefix, (size)
		match	, operand
		jyes	ret_short
		emit	1, opcode
		call	word, operand
		exit
	    ret_short:
		emit	1, opcode + 1
		exit
	    illegal_instruction:
		err	'illegal instruction'
	end calminstruction

end iterate

iterate <instr,opcode>, retq,0C2h, retnq,0C2h, retfq,0CAh

	calminstruction instr? operand
		check	x86.mode = 64
		jno	instruction_requires_long_mode
		match	, operand
		jyes	ret_short
		emit	1, opcode
		call	word, operand
		exit
	    ret_short:
		emit	1, opcode + 1
		exit
	    instruction_requires_long_mode:
		err	'instruction requires long mode'
	end calminstruction

end iterate

calminstruction lea? dest*,src*
	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src
	check	@src.type = 'mem' & @dest.type = 'reg'
	jno	invalid_combination_of_operands
	xcall	x86.select_operand_prefix@src, @dest.size
	xcall	x86.store_instruction@src, (8Dh),@dest.rm
	exit
    invalid_combination_of_operands:
	err	'invalid combination of operands'
end calminstruction

iterate <instr,opcode>, les,0C4h, lds,0C5h

	calminstruction instr? dest*,src*
		check	x86.mode = 64
		jyes	illegal_instruction
		call	x86.parse_operand@dest, dest
		call	x86.parse_operand@src, src
		check	(@dest.size = 2 & (@src.size <> 0 & @src.size <> 4)) | (@dest.size = 4 & (@src.size <> 0 & @src.size <> 6))
		jyes	invalid_operand_size
		check	@src.type = 'mem' & @dest.type = 'reg'
		jno	invalid_combination_of_operands
		xcall	x86.select_operand_prefix@src, @dest.size
		xcall	x86.store_instruction@src, (opcode),@dest.rm
		exit
	    illegal_instruction:
		err	'illegal instruction'
		exit
	    invalid_operand_size:
		err	'invalid operand size'
		exit
	    invalid_combination_of_operands:
		err	'invalid combination of operands'
	end calminstruction

end iterate

iterate <instr,opcode>, lss,<0Fh,0B2h>, lfs,<0Fh,0B4h>, lgs,<0Fh,0B5h>

	calminstruction instr? dest*,src*
		call	x86.parse_operand@dest, dest
		call	x86.parse_operand@src, src
		check	(@dest.size = 2 & (@src.size <> 0 & @src.size <> 4)) | (@dest.size = 4 & (@src.size <> 0 & @src.size <> 6)) | (@dest.size = 8 & (@src.size <> 0 & @src.size <> 10))
		jyes	invalid_operand_size
		check	@src.type = 'mem' & @dest.type = 'reg'
		jno	invalid_combination_of_operands
		xcall	x86.select_operand_prefix@src, @dest.size
		xcall	x86.store_instruction@src, <opcode>,@dest.rm
		exit
	    invalid_operand_size:
		err	'invalid operand size'
	    invalid_combination_of_operands:
		err	'invalid combination of operands'
	end calminstruction

end iterate

iterate <instr,postbyte>, rol,0, ror,1, rcl,2, rcr,3, shl,4, sal, 4, shr,5, sar,7

	calminstruction instr? dest*,cnt*

		call	x86.parse_operand@dest, dest
		call	x86.parse_operand@src, cnt

		check	@dest.size = 0
		jyes	operand_size_not_specified
		check	@src.size and not 1
		jyes	invalid_operand_size

	    main:
		check	@src.type = 'reg' & @src.size = 1 & @src.rm = 1 & ( @dest.type = 'reg' | @dest.type = 'mem' )
		jyes	shift_rm_cl
		check	@src.type = 'imm' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		jyes	shift_rm_imm

		err	'invalid combination of operands'
		exit

	    shift_rm_cl:
		check	@dest.size > 1
		jno	shift_r8_cl
		xcall	x86.select_operand_prefix@dest, @dest.size
		xcall	x86.store_instruction@dest, (0D3h),(postbyte)
		exit
	      shift_r8_cl:
		xcall	x86.store_instruction@dest, (0D2h),(postbyte)
		exit
	    shift_rm_imm:
		check	@dest.size > 1
		jno	shift_rm8_imm
		xcall	x86.select_operand_prefix@dest, @dest.size
		check	@src.imm = 1
		jyes	shift_rm_1
		xcall	x86.store_instruction@dest, (0C1h),(postbyte),byte,@src.imm
		exit
	      shift_rm_1:
		xcall	x86.store_instruction@dest, (0D1h),(postbyte)
		exit
	      shift_rm8_imm:
		check	@src.imm = 1
		jyes	shift_rm8_1
		xcall	x86.store_instruction@dest, (0C0h),(postbyte),byte,@src.imm
		exit
	      shift_rm8_1:
		xcall	x86.store_instruction@dest, (0D0h),(postbyte)
		exit

	    operand_size_not_specified:
		err	'operand size not specified'
		jump	main
	    invalid_operand_size:
		err	'invalid operand size'
		jump	main

	end calminstruction

end iterate

iterate <instr,ext>, shld,0A4h, shrd,0ACh

	calminstruction instr? dest*,src*,cnt*
		call	x86.parse_operand@dest, dest
		call	x86.parse_operand@src, src
		call	x86.parse_operand@aux, cnt
		check	@aux.size and not 1
		jyes	invalid_operand_size
		check	@dest.size and not @src.size
		jno	main
		err	'operand sizes do not match'
	    main:
		check	@aux.type = 'reg' & @aux.size = 1 & @aux.rm = 1 & @src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		jyes	shld_cl
		check	@aux.type = 'imm' & @src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		jyes	shld_imm
		err	'invalid combination of operands'
		exit
	    shld_cl:
		xcall	x86.select_operand_prefix@dest, @src.size
		xcall	x86.store_instruction@dest, <0Fh,ext+1>,@src.rm
		exit
	    shld_imm:
		xcall	x86.select_operand_prefix@dest, @src.size
		xcall	x86.store_instruction@dest, <0Fh,ext>,@src.rm,byte,@aux.imm
		exit
	    invalid_operand_size:
		err	'invalid operand size'
	end calminstruction

end iterate

iterate <instr,postbyte>, bt,4, bts,5, btr,6, btc,7

	calminstruction instr? dest*,src*
		call	x86.parse_operand@dest, dest
		call	x86.parse_operand@src, src
		check	@src.type = 'reg' & (@dest.type = 'mem' | @dest.type = 'reg')
		jyes	bt_rm_reg
		check	@src.type = 'imm' & (@dest.type = 'mem' | @dest.type = 'reg')
		jyes	bt_rm_imm
		err	'invalid combination of operands'
		exit
	    bt_rm_reg:
		local	opcode
		check	@dest.size and not @src.size
		jno	bt_rm_reg_ok
		err	'operand sizes do not match'
	      bt_rm_reg_ok:
		compute opcode, 0A3h+(postbyte-4) shl 3
		xcall	x86.select_operand_prefix@dest, @src.size
		xcall	x86.store_instruction@dest, <0Fh,opcode>,@src.rm
		exit
	    bt_rm_imm:
		check	@src.size and not 1
		jno	bt_rm_imm_ok
		err	'invalid operand size'
	      bt_rm_imm_ok:
		check	@dest.size
		jno	operand_size_not_specified
		xcall	x86.select_operand_prefix@dest, @dest.size
		xcall	x86.store_instruction@dest, <0Fh,0BAh>,(postbyte),byte,@src.imm
		exit
	    operand_size_not_specified:
		err	'operand size not specified'
	end calminstruction

end iterate

iterate <instr,ext>, bsf,0BCh, bsr,0BDh

	calminstruction instr? dest*,src*
		call	x86.parse_operand@dest, dest
		call	x86.parse_operand@src, src
		check	@dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg')
		jyes	bsf_rm_reg
		err	'invalid combination of operands'
		exit
	    bsf_rm_reg:
		check	@src.size and not @dest.size
		jno	bsf_rm_reg_ok
		err	'operand sizes do not match'
	      bsf_rm_reg_ok:
		xcall	x86.select_operand_prefix@src, @dest.size
		xcall	x86.store_instruction@src, <0Fh,ext>,@dest.rm
	end calminstruction

end iterate

iterate <instr,ext>, movzx,0B6h, movsx,0BEh

	calminstruction instr? dest*,src*
		call	x86.parse_operand@dest, dest
		call	x86.parse_operand@src, src
		check	@dest.size > @src.size
		jyes	size_ok
		err	'operand sizes do not match'
	    size_ok:
		check	@dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg')
		jyes	operands_ok
		err	'invalid combination of operands'
		exit
	    operands_ok:
		check	@src.size = 2
		jyes	movzx_word
		check	@src.size = 1 | (@src.size = 0 & @dest.size = 2)
		jyes	movzx_byte
		check	@src.size
		jyes	invalid_operand_size
		err	'operand size not specified'
	    movzx_word:
		xcall	x86.select_operand_prefix@src, @dest.size
		xcall	x86.store_instruction@src, <0Fh,ext+1>,@dest.rm
		exit
	    movzx_byte:
		xcall	x86.select_operand_prefix@src, @dest.size
		xcall	x86.store_instruction@src, <0Fh,ext>,@dest.rm
		exit
	    invalid_operand_size:
		err	'invalid operand size'
	end calminstruction

end iterate

calminstruction movsxd? dest*,src*
	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src
	check	@dest.size > @src.size
	jyes	size_ok
	err	'operand sizes do not match'
    size_ok:
	check	@dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg')
	jyes	operands_ok
	err	'invalid combination of operands'
	exit
    operands_ok:
	check	@src.size and not 4
	jyes	invalid_operand_size
	xcall	x86.select_operand_prefix@src, @dest.size
	xcall	x86.store_instruction@src, (63h),@dest.rm
	exit
    invalid_operand_size:
	err	'invalid operand size'
end calminstruction

iterate <cond,code>, o,0, no,1, c,2, b,2, nae,2, nc,3, nb,3, ae,3, z,4, e,4, nz,5, ne,5, na,6, be,6, a,7, nbe,7, \
		     s,8, ns,9, p,0Ah, pe,0Ah, np,0Bh, po,0Bh, l,0Ch, nge,0Ch, nl,0Dh, ge,0Dh, ng,0Eh, le,0Eh, g,0Fh, nle,0Fh

	calminstruction set#cond? dest*
		call	x86.parse_operand@dest, dest
		check	@dest.size > 1
		jno	size_ok
		err	'invalid operand size'
	    size_ok:
		check	@dest.type = 'reg' | @dest.type = 'mem'
		jyes	operand_ok
		err	'invalid operand'
		exit
	    operand_ok:
		xcall	x86.store_instruction@dest, <0Fh,90h+code>,(0)
	end calminstruction

	calminstruction cmov#cond? dest*,src*
		call	x86.parse_operand@dest, dest
		call	x86.parse_operand@src, src
		check	@dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg')
		jyes	cmov_rm_reg
		err	'invalid combination of operands'
		exit
	    cmov_rm_reg:
		check	@src.size and not @dest.size
		jno	cmov_rm_reg_ok
		err	'operand sizes do not match'
	      cmov_rm_reg_ok:
		xcall	x86.select_operand_prefix@src, @dest.size
		xcall	x86.store_instruction@src, <0Fh,40h+code>,@dest.rm
	end calminstruction

end iterate

calminstruction call? dest*

	call	x86.parse_jump_operand@dest, dest

	check	@dest.type = 'imm'
	jyes	call_imm
	check	@dest.type = 'mem' | @dest.type = 'reg'
	jyes	call_rm
	check	@dest.type = 'far'
	jyes	call_direct_far

    invalid_operand:
	err	'invalid operand'
	exit
    illegal_instruction:
	err	'illegal instruction'
	exit

    call_direct_far:
	check	@dest.jump_type & @dest.jump_type <> 'far'
	jyes	invalid_operand
	check	x86.mode = 64
	jyes	illegal_instruction
	check	@dest.size = 0
	jyes	prefix_ok
	local	size
	compute size, @dest.size - 2
	call	x86.store_operand_prefix, size
      prefix_ok:
	check	@dest.size and not 4 & @dest.size and not 6
	jyes	invalid_operand
	emit	1, 9Ah
	check	(@dest.size = 0 & x86.mode = 16) | @dest.size = 4
	jyes	far_dword
	call	dword, @dest.offset
	call	word, @dest.segment
	exit
      far_dword:
	call	word, @dest.offset
	call	word, @dest.segment
	exit

    call_rm:
	check	@dest.size = 6 | @dest.size = 10
	jyes	call_rm_pword
	check	@dest.size = 8 & x86.mode = 64
	jyes	call_rm_qword
	check	@dest.size = 4 & x86.mode < 64
	jyes	call_rm_dword
	check	@dest.size = 2 & x86.mode < 64
	jyes	call_rm_word
	check	@dest.size
	jyes	invalid_operand
	check	x86.mode = 64
	jno	rex_prefix_ok
	compute @dest.prefix, 48h
      rex_prefix_ok:
	check	@dest.jump_type = 'far'
	jyes	call_rm_far
	check	@dest.jump_type = 'near'
	jyes	call_rm_near
	err	'operand size not specified'
	exit

      call_rm_pword:
	check	@dest.jump_type & @dest.jump_type <> 'far'
	jyes	invalid_operand
	local	size
	compute size, @dest.size - 2
	call	x86.select_operand_prefix@dest, size
      call_rm_far:
	xcall	x86.store_instruction@dest, (0FFh),(11b)
	exit

      call_rm_qword:
	check	@dest.jump_type & @dest.jump_type <> 'near'
	jyes	invalid_operand
	jump	call_rm_near

      call_rm_dword:
	check	@dest.jump_type | @dest.type = 'reg'
	jno	call_rm_dword_auto
	check	@dest.jump_type = 'far'
	jyes	call_rm_dword_far
      call_rm_dword_near:
	xcall	x86.select_operand_prefix@dest, (4)
	jump	call_rm_near
      call_rm_dword_auto:
	check	x86.mode = 16
	jno	call_rm_dword_near
      call_rm_dword_far:
	xcall	x86.select_operand_prefix@dest, (2)
	jump	call_rm_far

      call_rm_word:
	check	@dest.jump_type & @dest.jump_type <> 'near'
	jyes	invalid_operand
	xcall	x86.select_operand_prefix@dest, (2)
      call_rm_near:
	xcall	x86.store_instruction@dest, (0FFh),(10b)
	exit

    call_imm:
	check	@dest.jump_type & @dest.jump_type <> 'near'
	jyes	invalid_operand
	check	@dest.size = 8 & x86.mode = 64
	jyes	call_imm_qword
	check	@dest.size = 2 & x86.mode < 64
	jyes	call_imm_word
	check	@dest.size = 4 & x86.mode < 64
	jno	invalid_operand
	xcall	x86.store_operand_prefix, (4)
	emit	1, 0E8h
	compute @dest.imm, @dest.imm-($+4)
	call	dword, @dest.imm
	exit
      call_imm_word:
	xcall	x86.store_operand_prefix, (2)
	emit	1, 0E8h
	compute @dest.imm, @dest.imm-($+2)
	check	@dest.imm relativeto 0
	jno	store_word
	compute @dest.imm, @dest.imm and 0FFFFh
      store_word:
	call	word, @dest.imm
	exit
      call_imm_qword:
	emit	1, 0E8h
	compute @dest.imm, @dest.imm-($+4)
	check	@dest.imm relativeto 0 & (@dest.imm < -80000000h | @dest.imm >= 80000000h)
	jno	store_dword
	err	'relative jump out of range'
      store_dword:
	call	dword, @dest.imm
	exit

end calminstruction

calminstruction jmp? dest*

	call	x86.parse_jump_operand@dest, dest

	check	@dest.type = 'imm'
	jyes	jmp_imm
	check	@dest.type = 'mem' | @dest.type = 'reg'
	jyes	jmp_rm
	check	@dest.type = 'far'
	jyes	jmp_direct_far

    invalid_operand:
	err	'invalid operand'
	exit
    illegal_instruction:
	err	'illegal instruction'
	exit

    jmp_direct_far:
	check	@dest.jump_type & @dest.jump_type <> 'far'
	jyes	invalid_operand
	check	x86.mode = 64
	jyes	illegal_instruction
	check	@dest.size = 0
	jyes	prefix_ok
	local	size
	compute size, @dest.size - 2
	call	x86.store_operand_prefix, size
      prefix_ok:
	check	@dest.size and not 4 & @dest.size and not 6
	jyes	invalid_operand
	emit	1, 0EAh
	check	(@dest.size = 0 & x86.mode = 16) | @dest.size = 4
	jyes	far_dword
	call	dword, @dest.offset
	call	word, @dest.segment
	exit
      far_dword:
	call	word, @dest.offset
	call	word, @dest.segment
	exit

    jmp_rm:
	check	@dest.size = 6 | @dest.size = 10
	jyes	jmp_rm_pword
	check	@dest.size = 8 & x86.mode = 64
	jyes	jmp_rm_qword
	check	@dest.size = 4 & x86.mode < 64
	jyes	jmp_rm_dword
	check	@dest.size = 2 & x86.mode < 64
	jyes	jmp_rm_word
	check	@dest.size
	jyes	invalid_operand
	check	x86.mode = 64
	jno	rex_prefix_ok
	compute @dest.prefix, 48h
      rex_prefix_ok:
	check	@dest.jump_type = 'far'
	jyes	jmp_rm_far
	check	@dest.jump_type = 'near'
	jyes	jmp_rm_near
	err	'operand size not specified'
	exit

      jmp_rm_pword:
	check	@dest.jump_type & @dest.jump_type <> 'far'
	jyes	invalid_operand
	local	size
	compute size, @dest.size - 2
	call	x86.select_operand_prefix@dest, size
      jmp_rm_far:
	xcall	x86.store_instruction@dest, (0FFh),(101b)
	exit

      jmp_rm_qword:
	check	@dest.jump_type & @dest.jump_type <> 'near'
	jyes	invalid_operand
	jump	jmp_rm_near

      jmp_rm_dword:
	check	@dest.jump_type | @dest.type = 'reg'
	jno	jmp_rm_dword_auto
	check	@dest.jump_type = 'far'
	jyes	jmp_rm_dword_far
      jmp_rm_dword_near:
	xcall	x86.select_operand_prefix@dest, (4)
	jump	jmp_rm_near
      jmp_rm_dword_auto:
	check	x86.mode = 16
	jno	jmp_rm_dword_near
      jmp_rm_dword_far:
	xcall	x86.select_operand_prefix@dest, (2)
	jump	jmp_rm_far

      jmp_rm_word:
	check	@dest.jump_type & @dest.jump_type <> 'near'
	jyes	invalid_operand
	xcall	x86.select_operand_prefix@dest, (2)
      jmp_rm_near:
	xcall	x86.store_instruction@dest, (0FFh),(100b)
	exit

    jmp_imm:
	check	@dest.jump_type = 'far'
	jyes	invalid_operand
	check	(@dest.size = 8 & x86.mode < 64) | (@dest.size < 8 & x86.mode = 64)
	jyes	invalid_operand
	check	@dest.size = 8
	jyes	jmp_imm_prefix_ok
	call	x86.store_operand_prefix, @dest.size
      jmp_imm_prefix_ok:
	check	@dest.jump_type = 'short'
	jyes	jmp_imm_short_verify
	check	@dest.jump_type = 'near'
	jyes	jmp_imm_near
	check	~ $ relativeto 0 & @dest.imm relativeto 0
	jno	jmp_optimize
	compute @dest.imm, @dest.imm + $ - 0 scaleof $
	err	'invalid address'
      jmp_optimize:
	check	@dest.unresolved
	jyes	jmp_imm_short
	check	@dest.imm relativeto $
	jno	jmp_imm_near
	check	(@dest.imm-($+2)) < 80h & (@dest.imm-($+2)) >= -80h
	jyes	jmp_imm_short
	check	(@dest.imm-($+2)) and (1 shl (@dest.size*8) - 1) < 80h | (@dest.imm-($+2)) and (1 shl (@dest.size*8) - 1) >= 1 shl (@dest.size*8) - 80h
	jyes	jmp_imm_short
      jmp_imm_near:
	check	@dest.size = 8
	jyes	jmp_imm_qword
	check	@dest.size = 2
	jyes	jmp_imm_word
	check	@dest.size = 4
	jno	invalid_operand
	emit	1, 0E9h
	compute @dest.imm, @dest.imm-($+4)
	call	dword, @dest.imm
	exit
      jmp_imm_word:
	emit	1, 0E9h
	compute @dest.imm, @dest.imm-($+2)
	check	@dest.imm relativeto 0
	jno	store_word
	compute @dest.imm, @dest.imm and 0FFFFh
      store_word:
	call	word, @dest.imm
	exit
      jmp_imm_qword:
	emit	1, 0E9h
	compute @dest.imm, @dest.imm-($+4)
	call	dword, @dest.imm
	check	@dest.imm relativeto 0 & (@dest.imm < -80000000h | @dest.imm >= 80000000h)
	jyes	relative_jump_out_of_range
	exit
      jmp_imm_short_verify:
	check	(@dest.imm-($+2)) < 80h & (@dest.imm-($+2)) >= -80h
	jyes	jmp_imm_short
	check	(@dest.imm-($+2)) and (1 shl (@dest.size*8) - 1) < 80h | (@dest.imm-($+2)) and (1 shl (@dest.size*8) - 1) >= 1 shl (@dest.size*8) - 80h
	jyes	jmp_imm_short
	emit	1, 0EBh
	emit	1
      relative_jump_out_of_range:
	err	'relative jump out of range'
	exit
      jmp_imm_short:
	emit	1, 0EBh
	compute @dest.imm, (@dest.imm-($+1)) and 0FFh
	emit	1, @dest.imm
	exit

end calminstruction

iterate <instr,opcode>, jo,70h, jno,71h, jc,72h, jb,72h, jnae,72h, jnc,73h, jnb,73h, jae,73h, jz,74h, je,74h, jnz,75h, jne,75h, jna,76h, jbe,76h, ja,77h, jnbe,77h, \
			js,78h, jns,79h, jp,7Ah, jpe,7Ah, jnp,7Bh, jpo,7Bh, jl,7Ch, jnge,7Ch, jnl,7Dh, jge,7Dh, jng,7Eh, jle,7Eh, jg,7Fh, jnle,7Fh

	calminstruction instr? dest*

		call	x86.parse_jump_operand@dest, dest

		check	@dest.type <> 'imm' | @dest.jump_type = 'far'
		jyes	invalid_operand

		check	x86.mode = 64 | @dest.size = 8
		jyes	long
		call	x86.store_operand_prefix, @dest.size
		jump	prefix_ok
	    long:
		check	x86.mode < 64 | @dest.size <> 8
		jyes	invalid_operand
	    prefix_ok:

		check	~ $ relativeto 0 & @dest.imm relativeto 0
		jno	optimize
		compute @dest.imm, @dest.imm + $ - 0 scaleof $
		err	'invalid address'
	    optimize:

		check	@dest.jump_type <> 'near' & ( @dest.unresolved | ( @dest.imm relativeto $ & @dest.imm-($+2) < 80h & @dest.imm-($+2) >= -80h ) )
		jyes	short
		check	@dest.jump_type <> 'near' & @dest.imm relativeto $ & ( (@dest.imm-($+2)) and (1 shl (@dest.size*8) - 1) < 80h | (@dest.imm-($+2)) and (1 shl (@dest.size*8) - 1) >= 1 shl (@dest.size*8) - 80h )
		jyes	short
		check	@dest.jump_type = 'short'
		jyes	relative_jump_out_of_range

		emit	1, 0Fh
		emit	1, 10h+opcode

		check	@dest.size = 2
		jyes	relative_word
		compute @dest.imm, @dest.imm-($+4)
		call	dword, @dest.imm
		exit
	      relative_word:
		compute @dest.imm, @dest.imm-($+2)
		check	@dest.imm relativeto 0
		jno	store_word
		compute @dest.imm, @dest.imm and 0FFFFh
	      store_word:
		call	word, @dest.imm
		exit

	    short:
		compute @dest.imm, (@dest.imm-($+2)) and 0FFh
		emit	1, opcode
		emit	1, @dest.imm
		exit

	    relative_jump_out_of_range:
		emit	2
		err	'relative jump out of range'
		exit

	    invalid_operand:
		err	'invalid operand'
		exit

	end calminstruction
end iterate

iterate <instr,opcode,len>, loopnz,0E0h,0, loopne,0E0h,0, loopz,0E1h,0, loope,0E1h,0, loop,0E2h,0, \
			    loopnzw,0E0h,2, loopnew,0E0h,2, loopzw,0E1h,2, loopew,0E1h,2, loopw,0E2h,2, \
			    loopnzd,0E0h,4, loopned,0E0h,4, loopzd,0E1h,4, looped,0E1h,4, loopd,0E2h,4, \
			    loopnzq,0E0h,8, loopneq,0E0h,8, loopzq,0E1h,8, loopeq,0E1h,8, loopq,0E2h,8, \
			    jcxz,0E3h,2, jecxz,0E3h,4, jrcxz,0E3h,8
	calminstruction instr? dest*

		call	x86.parse_jump_operand@dest, dest

		check	@dest.type = 'imm' & ( @dest.jump_type = 'short' | ~ @dest.jump_type )
		jno	invalid_operand

		check	len shl 3 and not x86.mode
		jno	address_prefix_ok
		check	len = 8 | (len = 2 & x86.mode = 64)
		jyes	illegal_instruction
		emit	1, 67h
	    address_prefix_ok:
		check	@dest.size shl 3 <> x86.mode
		jno	operand_prefix_ok
		check	@dest.size = 8 | x86.mode = 64
		jyes	invalid_operand
		emit	1, 66h
	    operand_prefix_ok:

		emit	1, opcode

		check	@dest.imm-($+1) < 80h & @dest.imm-($+1) >= -80h
		jyes	relative_offset_ok
		check	(@dest.imm-($+2)) and (1 shl (@dest.size*8) - 1) < 80h | (@dest.imm-($+2)) and (1 shl (@dest.size*8) - 1) >= 1 shl (@dest.size*8) - 80h
		jyes	relative_offset_ok
		emit	1
		err	'relative jump out of range'
		exit
	    relative_offset_ok:
		compute @dest.imm, (@dest.imm-($+1)) and 0FFh
		emit	1, @dest.imm
		exit

	    illegal_instruction:
		err	'illegal instruction'
		exit
	    invalid_operand:
		err	'invalid operand'
		exit

	end calminstruction
end iterate

iterate <instr,opcode>, nop,90h, int3,0CCh, into,0CEh, int1,0F1h, salc,0D6h, \
			hlt,0F4h, cmc,0F5h, clc,0F8h, stc,0F9h, cli,0FAh, sti,0FBh, cld,0FCh, std,0FDh, \
			pushf,9Ch, popf,9Dh, sahf,9Eh, lahf,9Fh, leave,0C9h, \
			insb,6Ch, outsb,6Eh, movsb,0A4h, cmpsb,0A6h, stosb,0AAh, lodsb,0ACh, scasb,0AEh, xlatb,0D7h, \
			clts,<0Fh,6>, invd,<0Fh,8>, wbinvd,<0Fh,9>, wrmsr,<0Fh,30h>, rdtsc,<0Fh,31h>, rdmsr,<0Fh,32h>, rdpmc,<0Fh,33h>, \
			sysenter,<0Fh,34h>, sysexit,<0Fh,35h>, cpuid,<0Fh,0A2h>, rsm,<0Fh,0AAh>

	match	byte1=,byte2, opcode
		calminstruction instr?
			emit	1, byte1
			emit	1, byte2
		end calminstruction
	else
		calminstruction instr?
			emit	1, opcode
		end calminstruction
	end match

end iterate

iterate <instr,opcode>, cbw,98h, cwd,99h, iretw,0CFh, \
			insw,6Dh, outsw,6Fh, movsw,0A5h, cmpsw,0A7h, stosw,0ABh, lodsw,0ADh, scasw,0AFh

	calminstruction instr?
		xcall	x86.store_operand_prefix, (2)
		emit	1, opcode
	end calminstruction

end iterate

iterate <instr,opcode>, cwde,98h, cdq,99h, iretd,0CFh, \
			insd,6Dh, outsd,6Fh, stosd,0ABh, lodsd,0ADh, scasd,0AFh

	calminstruction instr?
		xcall	x86.store_operand_prefix, (4)
		emit	1, opcode
	end calminstruction

end iterate

iterate <instr,opcode>, cdqe,98h, cqo,99h, iretq,0CFh, \
			movsq,0A5h, cmpsq,0A7h, stosq,0ABh, lodsq,0ADh, scasq,0AFh, \
			sysretq,<0Fh,7>, wrmsrq,<0Fh,30h>, rdmsrq,<0Fh,32h>, sysexitq,<0Fh,35h>

	match	byte1=,byte2, opcode
		calminstruction instr?
			xcall	x86.store_operand_prefix, (8)
			emit	1, byte1
			emit	1, byte2
		end calminstruction
	else
		calminstruction instr?
			xcall	x86.store_operand_prefix, (8)
			emit	1, opcode
		end calminstruction
	end match

end iterate

iterate <instr,opcode>, swapgs,<0Fh,1,0F8h>, syscall,<0Fh,5>, sysret,<0Fh,7>

	calminstruction instr?
		check	x86.mode = 64
		jyes	ok
		err	'instruction requires long mode'
		exit
	    ok:
		asm	db opcode
	end calminstruction

end iterate

iterate <prefix,opcode>, lock,0F0h, repnz,0F2h, repne,0F2h, rep,0F3h, repz,0F3h, repe,0F3h

	calminstruction prefix? instr&
		emit	1, opcode
		assemble instr
	end calminstruction

end iterate

calminstruction int? number*
	emit	1, 0CDh
	emit	1, number
end calminstruction

calminstruction iret?
	check	x86.mode < 64
	jyes	prefix_ok
	emit	1, 48h
    prefix_ok:
	emit	1, 0CFh
end calminstruction

iterate <instr,opcode>, daa,27h, das,2Fh, aaa,37h, aas,3Fh, pusha,60h, popa,61h

	calminstruction instr?
		check	x86.mode < 64
		jyes	allowed
		err	'illegal instruction'
		exit
	    allowed:
		emit	1, opcode
	end calminstruction

end iterate

calminstruction aam? number:10
	check	x86.mode < 64
	jyes	allowed
	err	'illegal instruction'
	exit
    allowed:
	emit	1, 0D4h
	emit	1, number
end calminstruction

calminstruction aad? number:10
	check	x86.mode < 64
	jyes	allowed
	err	'illegal instruction'
	exit
    allowed:
	emit	1, 0D5h
	emit	1, number
end calminstruction

iterate <instr,opcode>, pushaw,60h, popaw,61h, pushfw,9Ch, popfw,9Dh

	calminstruction instr?
		check	x86.mode < 64
		jyes	allowed
		err	'illegal instruction'
		exit
	    allowed:
		xcall	x86.store_operand_prefix, (2)
		emit	1, opcode
	end calminstruction

end iterate

iterate <instr,opcode>, pushad,60h, popad,61h, pushfd,9Ch, popfd,9Dh

	calminstruction instr?
		check	x86.mode < 64
		jyes	allowed
		err	'illegal instruction'
		exit
	    allowed:
		xcall	x86.store_operand_prefix, (4)
		emit	1, opcode
	end calminstruction

end iterate

iterate <instr,opcode>, pushfq,9Ch, popfq,9Dh

	calminstruction instr?
		check	x86.mode = 64
		jyes	allowed
		err	'instruction requires long mode'
		exit
	    allowed:
		emit	1, opcode
	end calminstruction

end iterate

calminstruction movs? dest*,src*
	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src
	local	size
	check	@dest.size = 0 & @src.size = 0
	jyes	operand_size_not_specified
	check	@dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
	jyes	operand_sizes_do_not_match
	compute size, @dest.size or @src.size
    size_ok:
	check	@src.type = 'mem' & @src.mod = 0 & @dest.type = 'mem' & @dest.mod = 0 & ( (x86.mode < 64 & @src.mode = 16 & @src.rm = 4 & @dest.mode = 16 & @dest.rm = 5) | (@src.mode > 16 & @src.rm = 6 & @dest.mode = @src.mode & @dest.rm = 7) ) & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h | (x86.mode = 64 & @dest.segment_prefix < 64h))
	jno	invalid_combination_of_operands
	check	@src.segment_prefix = 0 | @src.segment_prefix = 3Eh | (x86.mode = 64 & @src.segment_prefix < 64h)
	jyes	segment_prefix_ok
	emit	1, @src.segment_prefix
    segment_prefix_ok:
	check	@dest.mode = x86.mode
	jyes	address_prefix_ok
	emit	1, 67h
    address_prefix_ok:
	check	size > 1
	jyes	movs_word
	emit	1, 0A4h
	exit
    movs_word:
	call	x86.store_operand_prefix, size
	emit	1, 0A5h
	exit
    operand_size_not_specified:
	err	'operand size not specified'
	compute size, 0
	jump	size_ok
    operand_sizes_do_not_match:
	err	'operand sizes do not match'
	compute size, 0
	jump	size_ok
    invalid_combination_of_operands:
	err	'invalid combination of operands'
	exit
end calminstruction

calminstruction cmps? src*,dest*
	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src
	local	size
	check	@dest.size = 0 & @src.size = 0
	jyes	operand_size_not_specified
	check	@dest.size <> 0 & @src.size <> 0 & @dest.size <> @src.size
	jyes	operand_sizes_do_not_match
	compute size, @dest.size or @src.size
    size_ok:
	check	@src.type = 'mem' & @src.mod = 0 & @dest.type = 'mem' & @dest.mod = 0 & ( (x86.mode < 64 & @src.mode = 16 & @src.rm = 4 & @dest.mode = 16 & @dest.rm = 5) | (@src.mode > 16 & @src.rm = 6 & @dest.mode = @src.mode & @dest.rm = 7) ) & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h | (x86.mode = 64 & @dest.segment_prefix < 64h))
	jno	invalid_combination_of_operands
	check	@src.segment_prefix = 0 | @src.segment_prefix = 3Eh | (x86.mode = 64 & @src.segment_prefix < 64h)
	jyes	segment_prefix_ok
	emit	1, @src.segment_prefix
    segment_prefix_ok:
	check	@dest.mode = x86.mode
	jyes	address_prefix_ok
	emit	1, 67h
    address_prefix_ok:
	check	size > 1
	jyes	cmps_word
	emit	1, 0A6h
	exit
    cmps_word:
	call	x86.store_operand_prefix, size
	emit	1, 0A7h
	exit
    operand_size_not_specified:
	err	'operand size not specified'
	compute size, 0
	jump	size_ok
    operand_sizes_do_not_match:
	err	'operand sizes do not match'
	compute size, 0
	jump	size_ok
    invalid_combination_of_operands:
	err	'invalid combination of operands'
	exit
end calminstruction

calminstruction stos? dest*
	call	x86.parse_operand@dest, dest
	check	@dest.size
	jyes	size_ok
	err	'operand size not specified'
    size_ok:
	check	@dest.type = 'mem' & @dest.mod = 0 & ( (x86.mode < 64 & @dest.mode = 16 & @dest.rm = 5) | (@dest.mode > 16 & @dest.rm = 7) ) & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h | (x86.mode = 64 & @dest.segment_prefix < 64h))
	jno	invalid_operand
	check	@dest.mode = x86.mode
	jyes	address_prefix_ok
	emit	1, 67h
    address_prefix_ok:
	check	@dest.size > 1
	jyes	stos_word
	emit	1, 0AAh
	exit
    stos_word:
	call	x86.store_operand_prefix, @dest.size
	emit	1, 0ABh
	exit
    invalid_operand:
	err	'invalid operand'
	exit
end calminstruction

calminstruction lods? src*
	call	x86.parse_operand@src, src
	check	@src.size
	jyes	size_ok
	err	'operand size not specified'
    size_ok:
	check	@src.type = 'mem' & @src.mod = 0 & ( (x86.mode < 64 & @src.mode = 16 & @src.rm = 4) | (@src.mode > 16 & @src.rm = 6) )
	jno	invalid_operand
	check	@src.segment_prefix = 0 | @src.segment_prefix = 3Eh | (x86.mode = 64 & @src.segment_prefix < 64h)
	jyes	segment_prefix_ok
	emit	1, @src.segment_prefix
    segment_prefix_ok:
	check	@src.mode = x86.mode
	jyes	address_prefix_ok
	emit	1, 67h
    address_prefix_ok:
	check	@src.size > 1
	jyes	lods_word
	emit	1, 0ACh
	exit
    lods_word:
	call	x86.store_operand_prefix, @src.size
	emit	1, 0ADh
	exit
    invalid_operand:
	err	'invalid operand'
	exit
end calminstruction

calminstruction scas? dest*
	call	x86.parse_operand@dest, dest
	check	@dest.size
	jyes	size_ok
	err	'operand size not specified'
    size_ok:
	check	@dest.type = 'mem' & @dest.mod = 0 & ( (x86.mode < 64 & @dest.mode = 16 & @dest.rm = 5) | (@dest.mode > 16 & @dest.rm = 7) ) & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h | (x86.mode = 64 & @dest.segment_prefix < 64h))
	jno	invalid_operand
	check	@dest.mode = x86.mode
	jyes	address_prefix_ok
	emit	1, 67h
    address_prefix_ok:
	check	@dest.size > 1
	jyes	scas_word
	emit	1, 0AEh
	exit
    scas_word:
	call	x86.store_operand_prefix, @dest.size
	emit	1, 0AFh
	exit
    invalid_operand:
	err	'invalid operand'
	exit
end calminstruction

calminstruction ins? dest*,src*
	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src
	check	@dest.size
	jyes	size_ok
	err	'operand size not specified'
	compute size, 0
    size_ok:
	check	@src.type = 'reg' & @src.size = 2 & @src.rm = 2 & @dest.type = 'mem' & @dest.mod = 0 & ( (x86.mode < 64 & @dest.mode = 16 & @dest.rm = 5) | (@dest.mode > 16 & @dest.rm = 7) ) & ( @dest.segment_prefix = 0 | @dest.segment_prefix = 26h | (x86.mode = 64 & @dest.segment_prefix < 64h))
	jno	invalid_combination_of_operands
	check	@dest.mode = x86.mode
	jyes	address_prefix_ok
	emit	1, 67h
    address_prefix_ok:
	check	@dest.size > 1
	jyes	ins_word
	emit	1, 6Ch
	exit
    ins_word:
	call	x86.store_operand_prefix, @dest.size
	emit	1, 6Dh
	exit
    invalid_combination_of_operands:
	err	'invalid combination of operands'
	exit
end calminstruction

calminstruction outs? dest*,src*
	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src
	check	@src.size
	jyes	size_ok
	err	'operand size not specified'
    size_ok:
	check	@dest.type = 'reg' & @dest.size = 2 & @dest.rm = 2 & @src.type = 'mem' & @src.mod = 0 & ( (x86.mode < 64 & @src.mode = 16 & @src.rm = 4) | (@src.mode > 16 & @src.rm = 6) )
	jno	invalid_combination_of_operands
	check	@src.segment_prefix = 0 | @src.segment_prefix = 3Eh | (x86.mode = 64 & @src.segment_prefix < 64h)
	jyes	segment_prefix_ok
	emit	1, @src.segment_prefix
    segment_prefix_ok:
	check	@src.mode = x86.mode
	jyes	address_prefix_ok
	emit	1, 67h
    address_prefix_ok:
	check	@src.size > 1
	jyes	outs_word
	emit	1, 6Eh
	exit
    outs_word:
	call	x86.store_operand_prefix, @src.size
	emit	1, 6Fh
	exit
    operand_size_not_specified:
	err	'operand size not specified'
	compute size, 0
	jump	size_ok
    operand_sizes_do_not_match:
	err	'operand sizes do not match'
	compute size, 0
	jump	size_ok
    invalid_combination_of_operands:
	err	'invalid combination of operands'
	exit
end calminstruction

calminstruction xlat? src*
	call	x86.parse_operand@src, src
	check	@src.size > 1
	jno	size_ok
	err	'invalid operand size'
    size_ok:
	check	@src.type = 'mem' & @src.mod = 0 & ( (x86.mode < 64 & @src.mode = 16 & @src.rm = 7) | (@src.mode > 16 & @src.rm = 3) )
	jno	invalid_operand
	check	@src.segment_prefix = 0 | @src.segment_prefix = 3Eh | (x86.mode = 64 & @src.segment_prefix < 64h)
	jyes	segment_prefix_ok
	emit	1, @src.segment_prefix
    segment_prefix_ok:
	check	@src.mode = x86.mode
	jyes	address_prefix_ok
	emit	1, 67h
    address_prefix_ok:
	emit	1, 0D7h
	exit
    invalid_operand:
	err	'invalid operand'
	exit
end calminstruction

calminstruction in? dest*,src*
	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src
	check	@dest.size = 8
	jyes	invalid_size
	check	@dest.size
	jyes	size_ok
	err	'operand size not specified'
	jump	size_ok
    invalid_size:
	err	'invalid_operand_size'
    size_ok:
	check	@src.type = 'reg' & @src.size = 2 & @src.rm = 2 & @dest.type = 'reg' & @dest.rm = 0
	jyes	in_ax_dx
	check	@src.type = 'imm' & @dest.type = 'reg' & @dest.rm = 0
	jyes	in_ax_imm
	err	'invalid combination of operands'
	exit
    in_ax_dx:
	check	@dest.size > 1
	jno	in_al_dx
	call	x86.store_operand_prefix, @dest.size
	emit	1, 0EDh
	exit
    in_al_dx:
	emit	1, 0ECh
	exit
    in_ax_imm:
	check	@dest.size > 1
	jno	in_al_imm
	call	x86.store_operand_prefix, @dest.size
	emit	1, 0E5h
	emit	1, @src.imm
	exit
    in_al_imm:
	emit	1, 0E4h
	emit	1, @src.imm
	exit
end calminstruction

calminstruction out? dest*,src*
	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src
	check	@src.size = 8
	jyes	invalid_size
	check	@src.size
	jyes	size_ok
	err	'operand size not specified'
	jump	size_ok
    invalid_size:
	err	'invalid_operand_size'
    size_ok:
	check	@dest.type = 'reg' & @dest.size = 2 & @dest.rm = 2 & @src.type = 'reg' & @src.rm = 0
	jyes	out_dx_ax
	check	@dest.type = 'imm' & @src.type = 'reg' & @src.rm = 0
	jyes	out_imm_ax
	err	'invalid combination of operands'
	exit
    out_dx_ax:
	check	@src.size > 1
	jno	out_dx_al
	call	x86.store_operand_prefix, @src.size
	emit	1, 0EFh
	exit
    out_dx_al:
	emit	1, 0EEh
	exit
    out_imm_ax:
	check	@src.size > 1
	jno	out_imm_al
	call	x86.store_operand_prefix, @src.size
	emit	1, 0E7h
	emit	1, @dest.imm
	exit
    out_imm_al:
	emit	1, 0E6h
	emit	1, @dest.imm
	exit
end calminstruction

calminstruction enter? alloc*,nesting*
	call	x86.parse_operand@src, alloc
	call	x86.parse_operand@aux, nesting
	check	(@src.size and not 2) | (@aux.size and not 1)
	jno	size_ok
	err	'invalid operand size'
    size_ok:
	check	@src.type = 'imm' & @aux.type = 'imm'
	jyes	operand_ok
	err	'invalid operand'
	exit
    operand_ok:
	emit	1, 0C8h
	call	word, @src.imm
	emit	1, @aux.imm
end calminstruction

calminstruction bound? dest*,src*
	check	x86.mode < 64
	jyes	allowed
	err	'illegal instruction'
	exit
    allowed:
	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src
	check	@src.type = 'mem' & @dest.type = 'reg'
	jno	invalid_combination_of_operands
	check	@src.size and not @dest.size
	jno	size_ok
	err	'operand sizes do not match'
    size_ok:
	xcall	x86.select_operand_prefix@src, @dest.size
	xcall	x86.store_instruction@src, (62h),@dest.rm
	exit
    invalid_combination_of_operands:
	err	'invalid combination of operands'
	exit
end calminstruction

calminstruction arpl? dest*,src*
	check	x86.mode < 64
	jyes	allowed
	err	'illegal instruction'
	exit
    allowed:
	call	x86.parse_operand@dest, dest
	call	x86.parse_operand@src, src
	check	@src.type = 'reg' & (@dest.type = 'mem' | @dest.type = 'reg')
	jno	invalid_combination_of_operands
	check	@src.size = 2
	jno	invalid_operand_size
	check	@dest.size and not @src.size
	jno	size_ok
	err	'operand sizes do not match'
	jump	size_ok
    invalid_operand_size:
	err	'invalid operand size'
    size_ok:
	xcall	x86.store_instruction@dest, (63h),@src.rm
	exit
    invalid_combination_of_operands:
	err	'invalid combination of operands'
	exit
	exit
end calminstruction

iterate <instr,ext,postbyte>, lldt,0,2, ltr,0,3, verr,0,4, verw,0,5, lmsw,1,6

	calminstruction instr? dest*
		call	x86.parse_operand@dest, dest
		check	@dest.size and not 2
		jno	size_ok
		err	'invalid operand size'
	    size_ok:
		check	@dest.type = 'reg' | @dest.type = 'mem'
		jyes	operand_ok
		err	'invalid operand'
		exit
	    operand_ok:
		xcall	x86.store_instruction@dest, <0Fh,ext>,(postbyte)
	end calminstruction

end iterate

iterate <instr,ext,postbyte>, sldt,0,0, str,0,1, smsw,1,4

	calminstruction instr? dest*
		call	x86.parse_operand@dest, dest
		check	@dest.type = 'reg'
		jyes	select_operand_prefix
		check	@dest.size and not 2
		jno	size_ok
		err	'invalid operand size'
	    size_ok:
		check	@dest.type = 'mem'
		jyes	store_instruction
		err	'invalid combination of operands'
		exit
	    select_operand_prefix:
		xcall	x86.select_operand_prefix@dest, @dest.size
	    store_instruction:
		xcall	x86.store_instruction@dest, <0Fh,ext>,(postbyte)
	end calminstruction

end iterate

iterate <instr,postbyte>, lgdt,2, lidt,3, sgdt,0, sidt,1

	calminstruction instr? dest*
		call	x86.parse_operand@dest, dest
		check	@dest.type = 'mem'
		jyes	operand_ok
		err	'invalid operand'
		exit
	    operand_ok:
		check	x86.mode = 64 & @dest.size = 10
		jyes	store_instruction
		check	x86.mode < 64 & @dest.size = 6
		jyes	o32
		check	x86.mode < 64 & @dest.size = 5
		jyes	o16
		check	@dest.size
		jno	store_instruction
		err	'invalid operand size'
		jump	store_instruction
	    o16:
		xcall	x86.select_operand_prefix@dest, (2)
		jump	store_instruction
	    o32:
		xcall	x86.select_operand_prefix@dest, (4)
	    store_instruction:
		xcall	x86.store_instruction@dest, <0Fh,1>,(postbyte)
	end calminstruction

end iterate

iterate <instr,ext>, lar,2, lsl,3

	calminstruction instr? dest*,src*
		call	x86.parse_operand@dest, dest
		call	x86.parse_operand@src, src
		check	@dest.type = 'reg' & (@src.type = 'mem' | @src.type = 'reg')
		jno	invalid_combination_of_operands
		check	@src.size and not 2
		jno	size_ok
		err	'invalid operand size'
	    size_ok:
		xcall	x86.select_operand_prefix@src, @dest.size
		xcall	x86.store_instruction@src, <0Fh,ext>,@dest.rm
		exit
	    invalid_combination_of_operands:
		err	'invalid combination of operands'
	end calminstruction

end iterate

iterate <instr,ext>, cmpxchg,0B0h, xadd,0C0h

	calminstruction instr? dest*,src*
		call	x86.parse_operand@dest, dest
		call	x86.parse_operand@src, src
		check	@src.type = 'reg' & ( @dest.type = 'reg' | @dest.type = 'mem' )
		jyes	xadd_rm_reg
		err	'invalid combination of operands'
		exit
	    xadd_rm_reg:
		check	@dest.size and not @src.size
		jno	size_ok
		err	'operand sizes do not match'
	      size_ok:
		check	@src.size > 1
		jno	xadd_rm_reg_8bit
		xcall	x86.select_operand_prefix@dest, @src.size
		xcall	x86.store_instruction@dest, <0Fh,ext+1>,@src.rm
		exit
	      xadd_rm_reg_8bit:
		xcall	x86.store_instruction@dest, <0Fh,ext>,@src.rm
	end calminstruction

end iterate

calminstruction cmpxchg8b? dest*
	call	x86.parse_operand@dest, dest
	check	@dest.type = 'mem'
	jyes	operand_ok
	err	'invalid operand'
	exit
    operand_ok:
	check	@dest.size and not 8
	jno	size_ok
	err	'invalid operand size'
    size_ok:
	xcall	x86.store_instruction@dest, <0Fh,0C7h>,(1)
end calminstruction

calminstruction cmpxchg16b? dest*
	call	x86.parse_operand@dest, dest
	check	@dest.type = 'mem'
	jyes	operand_ok
	err	'invalid operand'
	exit
    operand_ok:
	check	@dest.size and not 16
	jno	size_ok
	err	'invalid operand size'
    size_ok:
	xcall	x86.store_operand_prefix, (8)
	xcall	x86.store_instruction@dest, <0Fh,0C7h>,(1)
end calminstruction

calminstruction bswap? dest*
	call	x86.parse_operand@dest, dest
	check	@dest.type = 'reg' & @dest.size > 2
	jyes	operand_ok
	err	'invalid operand'
	exit
    operand_ok:
	call	x86.store_operand_prefix, @dest.size,@dest.rm
	emit	1, 0Fh
	emit	1, 0C8h + @dest.rm and 111b
end calminstruction

calminstruction invlpg? dest*
	call	x86.parse_operand@dest, dest
	check	@dest.type = 'mem'
	jyes	operand_ok
	err	'invalid operand'
	exit
    operand_ok:
	xcall	x86.store_instruction@dest, <0Fh,1>,(7)
end calminstruction

include '80387.inc'

iterate <instr,opcode,postbyte>, fcmovb,0DAh,0C0h, fcmove,0DAh,0C8h, fcmovbe,0DAh,0D0h, fcmovu,0DAh,0D8h, \
				 fcmovnb,0DBh,0C0h, fcmovne,0DBh,0C8h, fcmovnbe,0DBh,0D0h, fcmovnu,0DBh,0D8h

	calminstruction instr? dest*,src*
		call	x87.parse_operand@dest, dest
		call	x87.parse_operand@src, src
		check	@dest.type = 'streg' & @dest.rm = 0 & @src.type = 'streg'
		jyes	ok
		err	'invalid operand'
		exit
	    ok:
		emit	1, opcode
		emit	1, postbyte + @src.rm
	end calminstruction

end iterate

iterate <instr,opcode,postbyte>, fucomi,0DBh,5, fucomip,0DFh,5, fcomi,0DBh,6, fcomip,0DFh,6

	calminstruction instr? src:st1
		call	x87.parse_operand@src, src
		check	@src.type = 'streg'
		jyes	ok
		err	'invalid operand'
		exit
	    ok:
		emit	1, opcode
		emit	1, 11b shl 6 + postbyte shl 3 + @src.rm
	end calminstruction

end iterate

include 'ext/sse2.inc'

repeat 8, i:8
	element xmm#i? : SSE.reg + i
end repeat

iterate <instr,postbyte>, fxsave64,0, fxrstor64,1

	calminstruction instr? src*
		check	x86.mode = 64
		jyes	allowed
		err	'instruction requires long mode'
		exit
	    allowed:
		call	x86.parse_operand@src, src
		check	@src.type = 'mem'
		jno	invalid_operand
		check	@src.size and not 512
		jno	size_ok
		err	'invalid operand size'
	    size_ok:
		xcall	x86.select_operand_prefix@src, (8)
		xcall	x86.store_instruction@src, <0Fh,0AEh>,(postbyte)
		exit
	    invalid_operand:
		err	'invalid operand'
	end calminstruction

end iterate
