
define x86

x86.settings = 0

macro x86.require? options&
	iterate option, options
		match name+, option
			if x86.settings and x86.setting_#name scale 1 < x86.setting_#name scale 0
				err `name, ' required'
				break
			end if
		else
			if x86.settings and x86.setting_#option scale 1 <> x86.setting_#option scale 0
				err `option, ' required'
				break
			end if
		end match
	end iterate
end macro

macro use? options&
	iterate name, options
		x86.settings = (x86.settings and not x86.setting_#name scale 1) or x86.setting_#name scale 0
	end iterate
	if x86.mode > 32
		x86.require AMD64+
	else if x86.mode > 16
		x86.require 80386+
	end if
end macro

macro use16?
	use 16
end macro

macro use32?
	use 32
end macro

macro use64?
	use 64
end macro

use16

macro ? line&
	match {decorator} instruction, line
		macro ? any&
			purge ?
			x86.settings =: x86.settings
			use decorator
			instruction
			restore x86.settings
		end macro
	end match
	outscope line
end macro

element x86.mask

x86.mask_bits = 0

struc x86.mask bits
	. = (1 shl bits - 1) shl x86.mask_bits
	x86.mask_bits = x86.mask_bits + bits
end struc

x86.MODE x86.mask 2
x86.CPU x86.mask 4
x86.FPU x86.mask 3
x86.SIMD x86.mask 3
x86.IMM x86.mask 4
x86.RM x86.mask 2

x86.mode equ 1 shl (4 + x86.settings and x86.MODE) shr bsf x86.MODE
x86.cpu equ (x86.settings and x86.CPU) shr bsf x86.CPU
x86.fpu equ (x86.settings and x86.FPU) shr bsf x86.FPU
x86.immsize equ (x86.settings and x86.IMM) shr bsf x86.IMM
x86.rmposition equ (x86.settings and x86.RM) shr bsf x86.RM

macro ? line&
	match name == selections, line
		local v
		v = 0
		iterate sel, selections
			match X value, sel
				v = v + value shl bsf x86.X + x86.X * x86.mask
			end match
		end iterate
		x86.setting_#name? := v
	else
		line
	end match
end macro

16 = MODE 0
32 = MODE 1
64 = MODE 2

8086 = CPU 0
80186 = CPU 1
80286 = CPU 2
80386 = CPU 3
80486 = CPU 4
P5 = CPU 5
P6 = CPU 6
x64 = CPU 8

8087 = FPU 1
80187 = FPU 2
80287 = FPU 3
80387 = FPU 4

MMX = SIMD 1
SSE = SIMD 2
SSE2 = SIMD 3
SSE3 = SIMD 4

i86 = CPU 0, FPU 0, SIMD 0
i186 = CPU 1, FPU 0, SIMD 0
i286 = CPU 2, FPU 0, SIMD 0
i386 = CPU 3, FPU 0, SIMD 0
i486 = CPU 4, FPU 0, SIMD 0
Pentium = CPU 5, FPU 4, SIMD 0
PentiumPro = CPU 6, FPU 4, SIMD 0
PentiumMMX = CPU 5, FPU 4, SIMD 1
Pentium2 = CPU 6, FPU 4, SIMD 1
Pentium3 = CPU 6, FPU 4, SIMD 2
Pentium4 = CPU 6, FPU 4, SIMD 4
AMD64 = CPU 8, FPU 4, SIMD 3

immauto = IMM 0
imm8 = IMM 1
imm16 = IMM 2
imm32 = IMM 4
imm64 = IMM 8

rmauto = RM 0
rm1 = RM 1
rmdst = RM 1
rm2 = RM 2
rmsrc = RM 2

purge ?

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
element x86.trx : x86.creg + 4

repeat 16, i:0
	element cr#i? : x86.crx + i
	element dr#i? : x86.drx + i
	element tr#i? : x86.trx + i
end repeat

element st?

repeat 8, i:0
	element st#i? : st? + i
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
define x86.xword? :16
define x86.qqword? :32
define x86.yword? :32
define x86.dqqword? :64
define x86.zword? :64

x86.REX_REQUIRED = 100h
x86.REX_FORBIDDEN = 200h

macro x86.parse_operands args&
	match operands, args
		iterate op, operands
			x86.parse_operand x86.@#%,op
			x86.@count = %%
		end iterate
	else
		x86.@count = 0
	end match
end macro

macro x86.parse_operand ns,op
	match :sz value, x86.op
		ns.size = sz
		x86.parse_operand_value ns,value
	else
		ns.size = 0
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
	else match =st?(i), op
		ns.size = 0
		ns.type = 'streg'
		ns.mod = 11b
		ns.rm = +i
	else
		ns.type = 'imm'
		ns.imm = +op
		if defined op
			ns.unresolved = 0
		else
			ns.unresolved = 1
		end if
		ns.displacement_size = 0
		if ns.imm eq ns.imm element 1
			if (ns.imm metadata 1) metadata 1 relativeto x86.reg
				ns.type = 'reg'
				ns.mode = x86.mode
				ns.mod = 11b
				ns.rm = ns.imm metadata 1 - (ns.imm metadata 1) element 1
				if ns.size & ns.size <> (ns.imm metadata 1) metadata 1 - x86.reg
					err 'operand sizes do not match'
				else
					ns.size = (ns.imm metadata 1) metadata 1 - x86.reg
					if ns.rm < 0
						ns.rm = x86.REX_FORBIDDEN - ns.rm
					else if ns.size = 1 & ns.rm >= 4 & ns.rm < 8
						ns.rm = x86.REX_REQUIRED + ns.rm
					end if
				end if
			else if ns.imm metadata 1 relativeto x86.sreg
				ns.type = 'sreg'
				ns.rm = ns.imm metadata 1 - x86.sreg
				if ns.rm >= 4
					x86.require 80386+
				end if
				if ns.size <> 0 & ns.size <> 2 & ns.size <> 4
					err 'invalid operand size'
				end if
			else if ns.imm metadata 1 metadata 1 relativeto x86.creg
				ns.type = 'creg'
				ns.rm = ns.imm metadata 1 - ns.imm metadata 1 element 1
				ns.ext = ns.imm metadata 1 metadata 1 - x86.creg
				if ns.size & ( (x86.mode <> 64 & ns.size <> 4) | (x86.mode = 64 & ns.size <> 8) )
					err 'invalid operand size'
				end if
			else if ns.imm eq st?
				ns.type = 'streg'
				ns.mod = 11b
				ns.rm = 0
			else if ns.imm metadata 1 relativeto st?
				ns.type = 'streg'
				ns.mod = 11b
				ns.rm = ns.imm metadata 1 - st?
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
		if ns.address metadata % relativeto x86.r16 | ns.address metadata % relativeto x86.r32 | ns.address metadata % relativeto x86.r64 | ns.address metadata % relativeto x86.ip
			ns.address_registers = ns.address_registers + ns.address element % * ns.address scale %
		end if
	end repeat
	ns.displacement = ns.address - ns.address_registers
	ns.auto_relative = 0
	if ns.address_registers eq 0
		x86.encode_direct_address ns
	else
		if ns.mode = 0
			ns.mode = x86.mode
		else
			if ns.mode <> ns.address_registers metadata 1 metadata 1 scale 0 shl 3 & ~ ns.address_registers metadata 1 relativeto x86.ip
				err 'invalid address'
			end if
		end if
		if ns.address_registers metadata 1 relativeto x86.r64 | ns.address_registers metadata 1 relativeto x86.r32
			x86.encode_address_32_64 ns
		else if ns.address_registers metadata 1 relativeto x86.r16
			x86.encode_address_16 ns
		else if ns.address_registers eq rip | ns.address_registers eq eip
			ns.mode = (ns.address_registers metadata 1 scale 0) shl 3
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
		if ns.mode = 0 & ns.segment_prefix < 64h
			ns.mode = 64
			ns.rm = 5
			ns.auto_relative = 1
		else if ns.mode <> 16
			ns.rm = 4
			ns.base = 5
			ns.index = 4
			ns.scale = 1
			if ns.mode = 64
				ns.displacement_size = 8
			else if ns.mode = 32 & ns.displacement relativeto 0
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
	if ns.address_registers metadata 1 relativeto x86.r64
		ns.mode = 64
		ns.address_registers_type = x86.r64
	else
		ns.mode = 32
		ns.address_registers_type = x86.r32
	end if
	ns.index_only = 0
	if ns.address_registers scale 2 = 0
		ns.scale = ns.address_registers scale 1
		ns.base = ns.address_registers metadata 1 - ns.address_registers_type
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
	else if ns.address_registers scale 3 = 0 & ns.address_registers metadata 2 relativeto ns.address_registers_type
		ns.rm = 4
		if ns.address_registers scale 1 = 1
			ns.base = ns.address_registers metadata 1 - ns.address_registers_type
			ns.index = ns.address_registers metadata 2 - ns.address_registers_type
			ns.scale = ns.address_registers scale 2
		else if ns.address_registers scale 2 = 1
			ns.base = ns.address_registers metadata 2 - ns.address_registers_type
			ns.index = ns.address_registers metadata 1 - ns.address_registers_type
			ns.scale = ns.address_registers scale 1
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
	if size > 4
		x86.require x64+
	else if size > 2
		x86.require 80386+
	end if
	if (size = 2 & x86.mode <> 16) | (size = 4 & x86.mode = 16)
		rm_operand.prefix = 66h
	else if size = 8
		rm_operand.prefix = 48h
	else if size <> 0 & size <> 2 & size <> 4 & size <> 8
		err 'invalid operand size'
	end if
end macro

macro x86.store_operand_prefix size*,reg:0
	if size > 4
		x86.require x64+
	else if size > 2
		x86.require 80386+
	end if
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
	if x86.immsize & imm_size & ( (x86.immsize <> imm_size & imm_size < 8) | (x86.immsize <> 4 & imm_size = 8) )
		err 'immediate size mismatch'
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
	macro instr? args&
		x86.parse_operands args
		if x86.@count = 2
			x86.size = 0
			if x86.@1.size = 0 & x86.@2.size = 0
				err 'operand size not specified'
			else if x86.@1.size <> 0 & x86.@2.size <> 0 & x86.@1.size <> x86.@2.size
				err 'operand sizes do not match'
			else
				x86.size = x86.@1.size or x86.@2.size
			end if
			if x86.@2.type = 'reg' & ( x86.@1.type = 'mem' | (x86.@1.type = 'reg' & x86.rmposition <> 2) )
				if x86.size > 1
					x86.select_operand_prefix x86.@1,x86.size
					x86.store_instruction basecode+1,x86.@1,x86.@2.rm
				else
					x86.store_instruction basecode,x86.@1,x86.@2.rm
				end if
			else if x86.@1.type = 'reg' & (x86.@2.type = 'mem' | x86.@2.type = 'reg')
				if x86.size > 1
					x86.select_operand_prefix x86.@2,x86.size
					x86.store_instruction basecode+3,x86.@2,x86.@1.rm
				else
					x86.store_instruction basecode+2,x86.@2,x86.@1.rm
				end if
			else if x86.@2.type = 'imm' & ( x86.@1.type = 'reg' | x86.@1.type = 'mem' )
				if x86.size > 1
					x86.select_operand_prefix x86.@1,x86.size
					if x86.@2.imm eqtype 0.0
						virtual at 0
							emit x86.size:@2.imm
							load x86.@2.imm:size from 0
							x86.@2.imm = +@2.imm
						end virtual
					end if
					if x86.immsize < 2 & x86.@2.imm relativeto 0 & x86.@2.imm < 80h & x86.@2.imm >= -80h
						x86.store_instruction 83h,x86.@1,basecode shr 3,1,x86.@2.imm
					else if x86.immsize < 2 & x86.@2.imm relativeto 0 & x86.@2.imm - 1 shl (x86.size shl 3) >= -80h & x86.@2.imm < 1 shl (x86.size shl 3)
						x86.@2.imm = x86.@2.imm - 1 shl (x86.size shl 3)
						x86.store_instruction 83h,x86.@1,basecode shr 3,1,x86.@2.imm
					else if x86.immsize = 1
						x86.store_instruction 83h,x86.@1,basecode shr 3,1,x86.@2.imm
					else if x86.@1.type = 'reg' & x86.@1.rm = 0
						if x86.@1.prefix
							db x86.@1.prefix
						end if
						db basecode+5
						if x86.immsize & ( (x86.immsize <> x86.size & x86.size < 8) | (x86.immsize <> 4 & x86.size = 8) )
							err 'immediate size mismatch'
						end if
						if x86.size = 2
							dw x86.@2.imm
						else if x86.size = 4
							dd x86.@2.imm
						else
							simm32 x86.@2.imm
						end if
					else
						x86.store_instruction 81h,x86.@1,basecode shr 3,x86.size,x86.@2.imm
					end if
				else
					if x86.@1.type = 'reg' & x86.@1.rm = 0
						db basecode+4
						if x86.immsize > 1
							err 'immediate size mismatch'
						end if
						db x86.@2.imm
					else
						x86.store_instruction 80h,x86.@1,basecode shr 3,1,x86.@2.imm
					end if
				end if
			else
				err 'invalid combination of operands'
			end if
		else
			err 'invalid number of operands'
		end if
	end macro
end iterate

iterate <instr,postbyte>, not,2, neg,3, mul,4, div,6, idiv,7
	macro instr? args&
		x86.parse_operands args
		if x86.@count = 1
			if x86.@1.type = 'mem' | x86.@1.type = 'reg'
				if x86.@1.size = 0
					err 'operand size not specified'
				end if
				if x86.@1.size > 1
					x86.select_operand_prefix x86.@1,x86.@1.size
					x86.store_instruction 0F7h,x86.@1,postbyte
				else
					x86.store_instruction 0F6h,x86.@1,postbyte
				end if
			else
				err 'invalid operand'
			end if
		else
			err 'invalid numer of operands'
		end if
	end macro
end iterate

macro mov? args&
	x86.parse_operands args
	if x86.@count = 2
		x86.size = 0
		if x86.@1.size = 0 & x86.@2.size = 0 & x86.@2.type <> 'sreg' & x86.@1.type <> 'sreg'
			err 'operand size not specified'
		else if x86.@1.size <> 0 & x86.@2.size <> 0 & x86.@1.size <> x86.@2.size
			err 'operand sizes do not match'
		else
			x86.size = x86.@1.size or x86.@2.size
		end if
		if x86.@2.type = 'reg' & x86.@1.type = 'mem' & x86.@2.rm = 0 & x86.@1.address_registers eq 0 & ~ x86.@1.auto_relative
			if x86.mode = 64
				if x86.@1.segment_prefix & x86.@1.segment_prefix >= 64h
					db x86.@1.segment_prefix
				end if
				if x86.@1.mode = 16
					err 'illegal addressing x86.mode'
				end if
				if x86.@1.displacement_size < 8
					db 67h
				end if
			else
				if x86.@1.segment_prefix & x86.@1.segment_prefix <> 3Eh
					db x86.@1.segment_prefix
				end if
				if x86.@1.mode <> x86.mode
					db 67h
				end if
			end if
			if x86.size > 1
				x86.store_operand_prefix x86.size
				db 0A3h
			else
				db 0A2h
			end if
			if x86.@1.mode = 16
				dw x86.@1.address
			else if x86.@1.displacement_size < 8
				dd x86.@1.address
			else
				dq x86.@1.address
			end if
		else if x86.@2.type = 'mem' & x86.@1.type = 'reg' & x86.@1.rm = 0 & x86.@2.address_registers eq 0 & ~ x86.@2.auto_relative
			if x86.mode = 64
				if x86.@2.segment_prefix & x86.@2.segment_prefix >= 64h
					db x86.@2.segment_prefix
				end if
				if x86.@2.mode = 16
					err 'illegal addressing x86.mode'
				end if
				if x86.@2.displacement_size < 8
					db 67h
				end if
			else
				if x86.@2.segment_prefix & x86.@2.segment_prefix <> 3Eh
					db x86.@2.segment_prefix
				end if
				if x86.@2.mode <> x86.mode
					db 67h
				end if
			end if
			if x86.size > 1
				x86.store_operand_prefix x86.size
				db 0A1h
			else
				db 0A0h
			end if
			if x86.@2.mode = 16
				dw x86.@2.address
			else if x86.@2.displacement_size < 8
				dd x86.@2.address
			else
				dq x86.@2.address
			end if
		else if x86.@2.type = 'reg' & ( x86.@1.type = 'mem' | (x86.@1.type = 'reg' & x86.rmposition <> 2) )
			if x86.size > 1
				x86.select_operand_prefix x86.@1,x86.size
				x86.store_instruction 89h,x86.@1,x86.@2.rm
			else
				x86.store_instruction 88h,x86.@1,x86.@2.rm
			end if
		else if x86.@1.type = 'reg' & (x86.@2.type = 'mem' | x86.@2.type = 'reg')
			if x86.size > 1
				x86.select_operand_prefix x86.@2,x86.size
				x86.store_instruction 8Bh,x86.@2,x86.@1.rm
			else
				x86.store_instruction 8Ah,x86.@2,x86.@1.rm
			end if
		else if x86.@2.type = 'imm' & x86.@1.type = 'mem'
			if x86.size > 1
				x86.select_operand_prefix x86.@1,x86.size
				x86.store_instruction 0C7h,x86.@1,0,x86.size,x86.@2.imm
			else
				x86.store_instruction 0C6h,x86.@1,0,1,x86.@2.imm
			end if
		else if x86.@2.type = 'imm' & x86.@1.type = 'reg'
			if x86.size > 1
				if x86.@2.imm eqtype 0.0
					virtual at 0
						emit x86.size:@2.imm
						load x86.@2.imm:size from 0
						x86.@2.imm = +@2.imm
					end virtual
				end if
				if x86.size = 8 & x86.@2.imm relativeto 0 & x86.@2.imm < 80000000h & x86.@2.imm >= -80000000h & x86.immsize < 8
					x86.select_operand_prefix x86.@1,x86.size
					x86.store_instruction 0C7h,x86.@1,0,x86.size,x86.@2.imm
				else if x86.size = 8 & x86.@2.imm relativeto 0 & x86.@2.imm - 1 shl 64 < 80000000h & x86.@2.imm - 1 shl 64 >= -80000000h & x86.immsize < 8
					x86.@2.imm = x86.@2.imm - 1 shl 64
					x86.select_operand_prefix x86.@1,x86.size
					x86.store_instruction 0C7h,x86.@1,0,x86.size,x86.@2.imm
				else
					x86.store_operand_prefix x86.size,x86.@1.rm
					db 0B8h + x86.@1.rm and 111b
					if x86.immsize & x86.immsize <> x86.size
						err 'immediate size mismatch'
					end if
					if x86.size = 2
						dw x86.@2.imm
					else if x86.size = 4
						dd x86.@2.imm
					else
						dq x86.@2.imm
					end if
				end if
			else
				x86.store_operand_prefix 0,x86.@1.rm
				db 0B0h + x86.@1.rm and 111b
				if x86.immsize > 1
					err 'immediate size mismatch'
				end if
				db x86.@2.imm
			end if
		else if x86.@2.type = 'creg' & x86.@1.type = 'reg'
			if x86.@2.ext = 4
				x86.require 80386
			else
				x86.require 80386+
			end if
			x86.store_instruction <0Fh,20h+x86.@2.ext>,x86.@1,x86.@2.rm
		else if x86.@2.type = 'reg' & x86.@1.type = 'creg'
			if x86.@1.ext = 4
				x86.require 80386
			else
				x86.require 80386+
			end if
			x86.store_instruction <0Fh,22h+x86.@1.ext>,x86.@2,x86.@1.rm
		else if x86.@2.type = 'sreg' & x86.@1.type = 'reg'
			if x86.size > 1
				x86.select_operand_prefix x86.@1,x86.size
				x86.store_instruction 8Ch,x86.@1,x86.@2.rm
			else
				err 'invalid operand size'
			end if
		else if x86.@2.type = 'sreg' & x86.@1.type = 'mem'
			if x86.size = 2
				x86.store_instruction 8Ch,x86.@1,x86.@2.rm
			else
				err 'invalid operand size'
			end if
		else if x86.@1.type = 'sreg' & x86.@1.rm <> 1 & ( x86.@2.type = 'reg' | x86.@2.type = 'mem' )
			if x86.size <> 1
				x86.store_instruction 8Eh,x86.@2,x86.@1.rm
			else
				err 'invalid operand size'
			end if
		else
			err 'invalid combination of operands'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro test? args&
	x86.parse_operands args
	if x86.@count = 2
		x86.size = 0
		if x86.@1.size = 0 & x86.@2.size = 0
			err 'operand size not specified'
		else if x86.@1.size <> 0 & x86.@2.size <> 0 & x86.@1.size <> x86.@2.size
			err 'operand sizes do not match'
		else
			x86.size = x86.@1.size or x86.@2.size
		end if
		if x86.@2.type = 'reg' & ( x86.@1.type = 'mem' | (x86.@1.type = 'reg' & x86.rmposition <> 2) )
			if x86.size > 1
				x86.select_operand_prefix x86.@1,x86.size
				x86.store_instruction 85h,x86.@1,x86.@2.rm
			else
				x86.store_instruction 84h,x86.@1,x86.@2.rm
			end if
		else if x86.@1.type = 'reg' & (x86.@2.type = 'mem' | x86.@2.type = 'reg')
			if x86.size > 1
				x86.select_operand_prefix x86.@2,x86.size
				x86.store_instruction 85h,x86.@2,x86.@1.rm
			else
				x86.store_instruction 84h,x86.@2,x86.@1.rm
			end if
		else if x86.@2.type = 'imm' & ( x86.@1.type = 'reg' | x86.@1.type = 'mem' )
			if x86.size > 1
				if x86.@1.type = 'reg' & x86.@1.rm = 0
					x86.store_operand_prefix x86.size
					db 0A9h
					if x86.immsize & ( (x86.immsize <> x86.size & x86.size < 8) | (x86.immsize <> 4 & x86.size = 8) )
						err 'immediate size mismatch'
					end if
					if x86.size = 2
						dw x86.@2.imm
					else if x86.size = 4
						dd x86.@2.imm
					else
						simm32 x86.@2.imm
					end if
				else
					x86.select_operand_prefix x86.@1,x86.size
					x86.store_instruction 0F7h,x86.@1,0,x86.size,x86.@2.imm
				end if
			else
				if x86.@1.type = 'reg' & x86.@1.rm = 0
					db 0A8h
					if x86.immsize > 1
						err 'immediate size mismatch'
					end if
					db x86.@2.imm
				else
					x86.store_instruction 0F6h,x86.@1,0,1,x86.@2.imm
				end if
			end if
		else
			err 'invalid combination of operands'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro xchg? args&
	x86.parse_operands args
	if x86.@count = 2
		x86.size = 0
		if x86.@1.size = 0 & x86.@2.size = 0
			err 'operand size not specified'
		else if x86.@1.size <> 0 & x86.@2.size <> 0 & x86.@1.size <> x86.@2.size
			err 'operand sizes do not match'
		else
			x86.size = x86.@1.size or x86.@2.size
		end if
		if x86.@2.type = 'reg' & x86.@1.type = 'reg'
			if x86.size > 1
				if (x86.@2.rm & x86.@1.rm) | (x86.size = 4 & x86.@2.rm or x86.@1.rm = 0)
					x86.select_operand_prefix x86.@2,x86.size
					x86.store_instruction 87h,x86.@2,x86.@1.rm
				else
					x86.@2.rm = x86.@2.rm or x86.@1.rm
					x86.store_operand_prefix x86.size,x86.@2.rm
					db 90h + x86.@2.rm and 111b
				end if
			else
				x86.store_instruction 86h,x86.@2,x86.@1.rm
			end if
		else if x86.@2.type = 'reg' & x86.@1.type = 'mem'
			if x86.size > 1
				x86.select_operand_prefix x86.@1,x86.size
				x86.store_instruction 87h,x86.@1,x86.@2.rm
			else
				x86.store_instruction 86h,x86.@1,x86.@2.rm
			end if
		else if x86.@2.type = 'mem' & x86.@1.type = 'reg'
			if x86.size > 1
				x86.select_operand_prefix x86.@2,x86.size
				x86.store_instruction 87h,x86.@2,x86.@1.rm
			else
				x86.store_instruction 86h,x86.@2,x86.@1.rm
			end if
		else
			err 'invalid combination of operands'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

iterate <instr,postbyte>, inc,0 ,dec,1
	macro instr? args&
		x86.parse_operands args
		if x86.@1.size = 0
			err 'operand size not specified'
		end if
		if x86.@count = 1
			if x86.@1.type = 'mem' | (x86.mode = 64 & x86.@1.type = 'reg')
				if x86.@1.size > 1
					x86.select_operand_prefix x86.@1,x86.@1.size
					x86.store_instruction 0FFh,x86.@1,postbyte
				else
					x86.store_instruction 0FEh,x86.@1,postbyte
				end if
			else if x86.@1.type = 'reg'
				if x86.@1.size > 1
					x86.store_operand_prefix x86.@1.size
					db 40h + x86.@1.rm + postbyte shl 3
				else
					x86.store_instruction 0FEh,x86.@1,postbyte
				end if
			else
				err 'invalid operand'
			end if
		else
			err 'invalid number of operands'
		end if
	end macro
end iterate

macro imul? args&
	x86.parse_operands args
	if x86.@count = 1
		parse_operand x86.@1,args
		if x86.@1.size = 0
			err 'operand size not specified'
		end if
		if x86.@1.type = 'mem' | x86.@1.type = 'reg'
			if x86.@1.size > 1
				x86.select_operand_prefix x86.@1,x86.@1.size
				x86.store_instruction 0F7h,x86.@1,5
			else
				x86.store_instruction 0F6h,x86.@1,5
			end if
		else
			err 'invalid operand'
		end if
	else if x86.@count = 2
		x86.require 80186+
		if x86.@1.size = 0 & x86.@2.size = 0
			err 'operand size not specified'
		else if x86.@1.size <> 0 & x86.@2.size <> 0 & x86.@1.size <> x86.@2.size
			err 'operand sizes do not match'
		else
			x86.size = x86.@1.size or x86.@2.size
		end if
		if x86.@1.type = 'reg' & (x86.@2.type = 'reg' | x86.@2.type = 'mem')
			x86.select_operand_prefix x86.@2,x86.size
			x86.store_instruction <0Fh,0AFh>,x86.@2,x86.@1.rm
		else if x86.@2.type = 'imm' & x86.@1.type = 'reg'
			x86.select_operand_prefix x86.@1,x86.size
			if x86.@2.imm eqtype 0.0
				virtual at 0
					emit x86.size:@2.imm
					load x86.@2.imm:size from 0
					x86.@2.imm = +@2.imm
				end virtual
			end if
			if x86.@2.imm relativeto 0 & x86.@2.imm < 80h & x86.@2.imm >= -80h
				x86.store_instruction 6Bh,x86.@1,x86.@1.rm,1,x86.@2.imm
			else if x86.@2.imm relativeto 0 & x86.@2.imm - 1 shl (x86.size shl 3) >= -80h & x86.@2.imm < 1 shl (x86.size shl 3)
				x86.@2.imm = x86.@2.imm - 1 shl (x86.size shl 3)
				x86.store_instruction 6Bh,x86.@1,x86.@1.rm,1,x86.@2.imm
			else
				x86.store_instruction 69h,x86.@1,x86.@1.rm,x86.size,x86.@2.imm
			end if
		else
			err 'invalid operand'
		end if
	else if x86.@count = 3
		x86.require 80186+
		if x86.@1.size = 0 & x86.@2.size = 0 & x86.@3.size = 0
			err 'operand size not specified'
		else if x86.@1.size <> 0 & x86.@2.size <> 0 & x86.@1.size <> x86.@2.size
			err 'operand sizes do not match'
		else
			x86.size = x86.@1.size or x86.@2.size
			if x86.@3.size <> 0 & x86.@3.size <> x86.size
				err 'operand sizes do not match'
			end if
		end if
		if x86.@3.type = 'imm' & ( x86.@2.type = 'mem' | x86.@2.type = 'reg' ) & x86.@1.type = 'reg'
			x86.select_operand_prefix x86.@2,x86.size
			if x86.@3.imm eqtype 0.0
				virtual at 0
					emit x86.size:@3.imm
					load x86.@3.imm:size from 0
					x86.@3.imm = +@3.imm
				end virtual
			end if
			if x86.@3.imm relativeto 0 & x86.@3.imm < 80h & x86.@3.imm >= -80h
				x86.store_instruction 6Bh,x86.@2,x86.@1.rm,1,x86.@3.imm
			else if x86.@3.imm relativeto 0 & x86.@3.imm - 1 shl (x86.size shl 3) >= -80h & x86.@3.imm < 1 shl (x86.size shl 3)
				x86.@3.imm = x86.@3.imm - 1 shl (x86.size shl 3)
				x86.store_instruction 6Bh,x86.@2,x86.@1.rm,1,x86.@3.imm
			else
				x86.store_instruction 69h,x86.@2,x86.@1.rm,x86.size,x86.@3.imm
			end if
		else
			err 'invalid operand'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro x86.push_instruction operand_size,args&
	x86.parse_operands args
	if x86.@count = 1
		if x86.@1.size = 0
			x86.@1.size = operand_size
		else if (operand_size <> 0 & x86.@1.size <> operand_size) | (x86.@1.size <> 2 & ((x86.@1.size <> 4 & x86.mode < 64) | (x86.@1.size <> 8 & x86.mode = 64)))
			err 'invalid operand size'
		end if
		if (x86.mode <> 16 & x86.@1.size = 2) | (x86.mode = 16 & x86.@1.size = 4)
			x86.@1.prefix = 66h
		end if
		if x86.@1.type = 'mem'
			x86.store_instruction 0FFh,x86.@1,110b
		else if x86.@1.type = 'reg'
			if x86.@1.prefix
				db x86.@1.prefix
			end if
			if x86.@1.rm and 1000b
				db 41h
				x86.@1.rm = x86.@1.rm and 111b
			end if
			db 50h + x86.@1.rm
		else if x86.@1.type = 'sreg'
			if x86.@1.prefix
				db x86.@1.prefix
			end if
			if x86.@1.rm >= 4
				db 0Fh,0A0h + (x86.@1.rm-4) shl 3
			else if x86.mode <> 64
				db 6 + x86.@1.rm shl 3
			else
				err 'invalid operand'
			end if
		else if x86.@1.type = 'imm'
			x86.require 80186+
			if x86.@1.prefix
				db x86.@1.prefix
			end if
			if x86.@1.size = 0
				if x86.mode = 16
					x86.@1.size = 2
				else if x86.mode = 64
					x86.@1.size = 8
				else
					x86.@1.size = 4
				end if
			end if
			if x86.@1.imm eqtype 0.0
				virtual at 0
					emit x86.@1.size:@1.imm
					load x86.@1.imm:@1.size from 0
					x86.@1.imm = +@1.imm
				end virtual
			end if
			if x86.@1.size = 8 & x86.@1.imm relativeto 0
				if x86.@1.imm-10000000000000000h >= -80000000h & x86.@1.imm < 10000000000000000h
					x86.@1.imm = x86.@1.imm - 10000000000000000h
				else if x86.@1.imm >= 80000000h | x86.@1.imm < -80000000h
					err 'immediate value out of signed range'
				end if
			end if
			if x86.immsize < 2 & x86.@1.imm relativeto 0 & x86.@1.imm < 80h & x86.@1.imm >= -80h
				db 6Ah
				db x86.@1.imm
			else if x86.immsize < 2 & x86.@1.size = 2 & x86.@1.imm relativeto 0 & x86.@1.imm-10000h >= -80h & x86.@1.imm < 10000h
				x86.@1.imm = x86.@1.imm - 10000h
				db 6Ah
				db x86.@1.imm
			else if x86.immsize < 2 & x86.@1.size = 4 & x86.@1.imm relativeto 0 & x86.@1.imm-100000000h >= -80h & x86.@1.imm < 100000000h
				x86.@1.imm = x86.@1.imm - 100000000h
				db 6Ah
				db x86.@1.imm
			else if x86.immsize = 1
				db 6Ah
				db x86.@1.imm
			else
				db 68h
				if x86.@1.size = 2 | (x86.@1.size = 0 & x86.mode = 16)
					if x86.immsize & x86.immsize <> 2
						err 'immediate size mismatch'
					end if
					dw x86.@1.imm
				else
					if x86.immsize & x86.immsize <> 4
						err 'immediate size mismatch'
					end if
					dd x86.@1.imm
				end if
			end if
		else
			err 'invalid operand'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro x86.pop_instruction operand_size,args&
	x86.parse_operands args
	if x86.@count = 1
		if x86.@1.size = 0
			x86.@1.size = operand_size
		else if (operand_size <> 0 & x86.@1.size <> operand_size) | (x86.@1.size <> 2 & ((x86.@1.size <> 4 & x86.mode < 64) | (x86.@1.size <> 8 & x86.mode = 64)))
			err 'invalid operand size'
		end if
		if (x86.mode <> 16 & x86.@1.size = 2) | (x86.mode = 16 & x86.@1.size = 4)
			x86.@1.prefix = 66h
		end if
		if x86.@1.type = 'mem'
			x86.store_instruction 8Fh,x86.@1,0
		else if x86.@1.type = 'reg'
			if x86.@1.prefix
				db x86.@1.prefix
			end if
			if x86.@1.rm and 1000b
				db 41h
				x86.@1.rm = x86.@1.rm and 111b
			end if
			db 58h + x86.@1.rm
		else if x86.@1.type = 'sreg'
			if x86.@1.prefix
				db x86.@1.prefix
			end if
			if x86.@1.rm >= 4
				db 0Fh,0A1h + (x86.@1.rm-4) shl 3
			else if x86.@1.rm <> 1 & x86.mode <> 64
				db 7 + x86.@1.rm shl 3
			else
				err 'invalid operand'
			end if
		else
			err 'invalid operand'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro push? args&
	x86.push_instruction 0,args
end macro

macro pushw? args&
	x86.push_instruction 2,args
end macro

macro pushd? args&
	x86.push_instruction 4,args
end macro

macro pushq? args&
	x86.push_instruction 8,args
end macro

macro pop? args&
	x86.pop_instruction 0,args
end macro

macro popw? args&
	x86.pop_instruction 2,args
end macro

macro popd? args&
	x86.pop_instruction 4,args
end macro

macro popq? args&
	x86.pop_instruction 8,args
end macro

macro retn? args&
	x86.parse_operands args
	if x86.@count = 0
		db 0C3h
	else if x86.@count = 1 & x86.@1.type = 'imm'
		db 0C2h
		if x86.immsize & x86.immsize <> 2
			err 'immediate size mismatch'
		end if
		dw x86.@1.imm
	else
		err 'invalid operand'
	end if
end macro

macro retnw? args&
	x86.parse_operands args
	if x86.mode < 64
		x86.store_operand_prefix 2
		if x86.@count = 0
			db 0C3h
		else if x86.@count = 1 & x86.@1.type = 'imm'
			db 0C2h
			if x86.immsize & x86.immsize <> 2
				err 'immediate size mismatch'
			end if
			dw x86.@1.imm
		else
			err 'invalid args&'
		end if
	else
		err 'illegal instruction'
	end if
end macro

macro retnd? args&
	x86.parse_operands args
	if x86.mode < 64
		x86.store_operand_prefix 4
		if x86.@count = 0
			db 0C3h
		else if x86.@count = 1 & x86.@1.type = 'imm'
			db 0C2h
			if x86.immsize & x86.immsize <> 2
				err 'immediate size mismatch'
			end if
			dw x86.@1.imm
		else
			err 'invalid args&'
		end if
	else
		err 'illegal instruction'
	end if
end macro

macro retnq? args&
	x86.parse_operands args
	if x86.mode = 64
		if x86.@count = 0
			db 0C3h
		else if x86.@count = 1 & x86.@1.type = 'imm'
			db 0C2h
			if x86.immsize & x86.immsize <> 2
				err 'immediate size mismatch'
			end if
			dw x86.@1.imm
		else
			err 'invalid args&'
		end if
	else
		err 'instruction requires long x86.mode'
	end if
end macro

macro retf? args&
	x86.parse_operands args
	if x86.@count = 0
		db 0CBh
	else if x86.@count = 1 & x86.@1.type = 'imm'
		db 0CAh
		if x86.immsize & x86.immsize <> 2
			err 'immediate size mismatch'
		end if
		dw x86.@1.imm
	else
		err 'invalid args&'
	end if
end macro

macro retfw? args&
	x86.parse_operands args
	x86.store_operand_prefix 2
	if x86.@count = 0
		db 0CBh
	else if x86.@count = 1 & x86.@1.type = 'imm'
		db 0CAh
		if x86.immsize & x86.immsize <> 2
			err 'immediate size mismatch'
		end if
		dw x86.@1.imm
	else
		err 'invalid args&'
	end if
end macro

macro retfd? args&
	x86.parse_operands args
	x86.store_operand_prefix 4
	if x86.@count = 0
		db 0CBh
	else if x86.@count = 1 & x86.@1.type = 'imm'
		db 0CAh
		if x86.immsize & x86.immsize <> 2
			err 'immediate size mismatch'
		end if
		dw x86.@1.imm
	else
		err 'invalid args&'
	end if
end macro

macro retfq? args&
	x86.parse_operands args
	x86.store_operand_prefix 8
	if x86.@count = 0
		db 0CBh
	else if x86.@count = 1 & x86.@1.type = 'imm'
		db 0CAh
		if x86.immsize & x86.immsize <> 2
			err 'immediate size mismatch'
		end if
		dw x86.@1.imm
	else
		err 'invalid args&'
	end if
end macro

macro ret? args&
	retn args
end macro

macro retw? args&
	retnw args
end macro

macro retd? args&
	retnd args
end macro

macro retq? args&
	retnq args
end macro

macro lea? args&
	x86.parse_operands args
	if x86.@count = 2
		if x86.@1.type = 'reg' & x86.@2.type = 'mem'
			x86.select_operand_prefix x86.@2,x86.@1.size
			x86.store_instruction 8Dh,x86.@2,x86.@1.rm
		else
			err 'invalid combination of operands'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

iterate <instr,opcode>, les,0C4h, lds,0C5h
	macro instr? args&
		x86.parse_operands args
		if x86.mode < 64
			if x86.@count = 2
				if (x86.@1.size = 2 & (x86.@2.size <> 0 & x86.@2.size <> 4)) | (x86.@1.size = 4 & (x86.@2.size <> 0 & x86.@2.size <> 6))
					err 'invalid operand size'
				end if
				if x86.@2.type = 'mem' & x86.@1.type = 'reg'
					x86.select_operand_prefix x86.@2,x86.@1.size
					x86.store_instruction opcode,x86.@2,x86.@1.rm
				else
					err 'invalid combination of operands'
				end if
			else
				err 'invalid number of operands'
			end if
		else
			err 'illegal instruction'
		end if
	end macro
end iterate

iterate <instr,ext>, lss,0B2h, lfs,0B4h, lgs,0B5h
	macro instr? args&
		x86.parse_operands args
		if x86.@count = 2
			if (x86.@1.size = 2 & (x86.@2.size <> 0 & x86.@2.size <> 4)) | (x86.@1.size = 4 & (x86.@2.size <> 0 & x86.@2.size <> 6))
				err 'invalid operand size'
			end if
			if x86.@2.type = 'mem' & x86.@1.type = 'reg'
				x86.select_operand_prefix x86.@2,x86.@1.size
				x86.store_instruction <0Fh,ext>,x86.@2,x86.@1.rm
			else
				err 'invalid combination of operands'
			end if
		else
			err 'invalid number of operands'
		end if
	end macro
end iterate

macro x86.shift_instruction opcode,args&
	x86.parse_operands args
	if x86.@count = 2
		if x86.@1.size = 0
			err 'operand size not specified'
		end if
		if x86.@2.size and not 1
			err 'invalid operand size'
		end if
		if x86.@2.type = 'reg' & x86.@2.size = 1 & x86.@2.rm = 1 & ( x86.@1.type = 'reg' | x86.@1.type = 'mem' )
			if x86.@1.size > 1
				x86.select_operand_prefix x86.@1,x86.@1.size
				x86.store_instruction 0D3h,x86.@1,opcode
			else
				x86.require 80186+
				x86.store_instruction 0D2h,x86.@1,opcode
			end if
		else if x86.@2.type = 'imm' & ( x86.@1.type = 'reg' | x86.@1.type = 'mem' )
			if x86.@1.size > 1
				x86.select_operand_prefix x86.@1,x86.@1.size
				if x86.@2.imm = 1
					x86.store_instruction 0D1h,x86.@1,opcode
				else
					x86.require 80186+
					x86.store_instruction 0C1h,x86.@1,opcode,1,x86.@2.imm
				end if
			else
				if x86.@2.imm = 1
					x86.store_instruction 0D0h,x86.@1,opcode
				else
					x86.require 80186+
					x86.store_instruction 0C0h,x86.@1,opcode,1,x86.@2.imm
				end if
			end if
		else
			err 'invalid combination of operands'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro rol? args&
	x86.shift_instruction 0,args
end macro

macro ror? args&
	x86.shift_instruction 1,args
end macro

macro rcl? args&
	x86.shift_instruction 2,args
end macro

macro rcr? args&
	x86.shift_instruction 3,args
end macro

macro shl? args&
	x86.shift_instruction 4,args
end macro

macro sal? args&
	x86.shift_instruction 4,args
end macro

macro shr? args&
	x86.shift_instruction 5,args
end macro

macro sar? args&
	x86.shift_instruction 7,args
end macro

iterate <instr,ext>, movzx,0B6h, movsx,0BEh
	macro instr? args&
		x86.parse_operands args
		x86.require 80386+
		if x86.@count = 2
			if x86.@1.size <= x86.@2.size
				err 'operand sizes do not match'
			end if
			if x86.@1.type = 'reg' & (x86.@2.type = 'mem' | x86.@2.type = 'reg')
				if x86.@2.size = 2
					x86.select_operand_prefix x86.@2,x86.@1.size
					x86.store_instruction <0Fh,ext+1>,x86.@2,x86.@1.rm
				else if x86.@2.size = 1
					x86.select_operand_prefix x86.@2,x86.@1.size
					x86.store_instruction <0Fh,ext>,x86.@2,x86.@1.rm
				else if x86.@2.size <> 0
					err 'invalid operand size'
				else
					err 'operand size not specified'
				end if
			else
				err 'invalid combination of operands'
			end if
		else
			err 'invalid number of operands'
		end if
	end macro
end iterate

macro movsxd? args&
	x86.parse_operands args
	x86.require x64+
	if x86.@count = 2
		if x86.@1.size <= x86.@2.size
			err 'operand sizes do not match'
		end if
		if x86.@1.type = 'reg' & (x86.@2.type = 'mem' | x86.@2.type = 'reg')
			if x86.@2.size = 0 | x86.@2.size = 4
				x86.select_operand_prefix x86.@2,x86.@1.size
				x86.store_instruction 63h,x86.@2,x86.@1.rm
			else
				err 'invalid operand size'
			end if
		else
			err 'invalid combination of operands'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

iterate <instr,postbyte>, bt,4, bts,5, btr,6, btc,7
	macro instr? args&
		x86.parse_operands args
		x86.require 80386+
		if x86.@count = 2
			x86.size = 0
			if x86.@2.type = 'reg' & (x86.@1.type = 'mem' | x86.@1.type = 'reg')
				if x86.@1.size <> 0 & x86.@2.size <> 0 & x86.@1.size <> x86.@2.size
					err 'operand sizes do not match'
				else
					x86.size = x86.@1.size or x86.@2.size
				end if
				x86.select_operand_prefix x86.@1,x86.size
				x86.store_instruction <0Fh,0A3h+(postbyte-4) shl 3>,x86.@1,x86.@2.rm
			else if x86.@2.type = 'imm' & (x86.@1.type = 'mem' | x86.@1.type = 'reg')
				if x86.@2.size <> 0 & x86.@2.size <> 1
					err 'invalid operand size'
				end if
				if x86.@1.size
					x86.select_operand_prefix x86.@1,x86.@1.size
					x86.store_instruction <0Fh,0BAh>,x86.@1,postbyte,1,x86.@2.imm
				else
					err 'operand size not specified'
				end if
			else
				err 'invalid combination of operands'
			end if
		else
			err 'invalid number of operands'
		end if
	end macro
end iterate

iterate <instr,ext>, bsf,0BCh, bsr,0BDh
	macro instr? args&
		x86.parse_operands args
		x86.require 80386+
		if x86.@count = 2
			if x86.@1.size <> 0 & x86.@2.size <> 0 & x86.@1.size <> x86.@2.size
				err 'operand sizes do not match'
			end if
			if x86.@1.type = 'reg' & (x86.@2.type = 'mem' | x86.@2.type = 'reg')
				x86.select_operand_prefix x86.@2,x86.@1.size
				x86.store_instruction <0Fh,ext>,x86.@2,x86.@1.rm
			else
				err 'invalid combination of operands'
			end if
		else
			err 'invalid number of operands'
		end if
	end macro
end iterate

iterate <instr,ext>, shld,0A4h, shrd,0ACh
	macro instr? args&
		x86.parse_operands args
		x86.require 80386+
		if x86.@count = 3
			x86.size = 0
			if x86.@1.size <> 0 & x86.@2.size <> 0 & x86.@1.size <> x86.@2.size
				err 'operand sizes do not match'
			else
				x86.size = x86.@1.size or x86.@2.size
			end if
			if x86.@3.size <> 0 & x86.@3.size <> 1
				err 'invalid operand size'
			end if
			if x86.@3.type = 'reg' & x86.@3.size = 1 & x86.@3.rm = 1 & x86.@2.type = 'reg' & ( x86.@1.type = 'reg' | x86.@1.type = 'mem' )
				x86.select_operand_prefix x86.@1,x86.size
				x86.store_instruction <0Fh,ext+1>,x86.@1,x86.@2.rm
			else if x86.@3.type = 'imm' & x86.@2.type = 'reg' & ( x86.@1.type = 'reg' | x86.@1.type = 'mem' )
				x86.select_operand_prefix x86.@1,x86.size
				x86.store_instruction <0Fh,ext>,x86.@1,x86.@2.rm,1,x86.@3.imm
			else
				err 'invalid combination of operands'
			end if
		else
			err 'invalid number of operands'
		end if
	end macro
end iterate

iterate <instr,ext>, cmpxchg,0B0h, xadd,0C0h
	macro instr? args&
		x86.parse_operands args
		x86.require 80486+
		if x86.@count = 2
			x86.size = 0
			if x86.@1.size = 0 & x86.@2.size = 0
				err 'operand size not specified'
			else if x86.@1.size <> 0 & x86.@2.size <> 0 & x86.@1.size <> x86.@2.size
				err 'operand sizes do not match'
			else
				x86.size = x86.@1.size or x86.@2.size
			end if
			if x86.@2.type = 'reg' & ( x86.@1.type = 'reg' | x86.@1.type = 'mem' )
				if x86.size > 1
					x86.select_operand_prefix x86.@1,x86.size
					x86.store_instruction <0Fh,ext+1>,x86.@1,x86.@2.rm
				else
					x86.store_instruction <0Fh,ext>,x86.@1,x86.@2.rm
				end if
			else
				err 'invalid combination of operands'
			end if
		else
			err 'invalid number of operands'
		end if
	end macro
end iterate

macro bswap? args&
	x86.parse_operands args
	x86.require 80486+
	if x86.@count = 1
		if x86.@1.type = 'reg' & x86.@1.size > 2
			x86.store_operand_prefix x86.@1.size,x86.@1.rm
			db 0Fh,0C8h + x86.@1.rm and 111b
		else
			err 'invalid operand'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro cmpxchg8b? args&
	x86.parse_operands args
	x86.require P5+
	if x86.@count = 1
		if x86.@1.size <> 0 & x86.@1.size <> 8
			err 'invalid operand size'
		end if
		if x86.@1.type = 'mem'
			x86.store_instruction <0Fh,0C7h>,x86.@1,1
		else
			err 'invalid operand'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro cmpxchg16b? args&
	x86.parse_operands args
	x86.require x64+
	if x86.@count = 1
		if x86.@1.size <> 0 & x86.@1.size <> 16
			err 'invalid operand size'
		end if
		if x86.@1.type = 'mem'
			x86.store_operand_prefix 8
			x86.store_instruction <0Fh,0C7h>,x86.@1,1
		else
			err 'invalid operand'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro x86.parse_jump_operands args&
	match =far? dest, args
		x86.parse_operands dest
		x86.@1.jump_type = 'far'
	else match =near? dest, args
		x86.parse_operands dest
		x86.@1.jump_type = 'near'
	else match =short? dest, args
		x86.parse_operands dest
		x86.@1.jump_type = 'short'
	else
		x86.parse_operands args
		x86.@1.jump_type = ''
	end match
	if x86.@1.type = 'imm'
		if x86.@1.size = 0
			x86.@1.size = x86.mode shr 3
		end if
		if x86.@1.imm relativeto 0 & (x86.@1.imm < 0 | x86.@1.imm >= 1 shl (x86.@1.size*8))
			err 'value out of range'
		end if
	end if
end macro


macro call? args&
	x86.parse_jump_operands args
	if x86.@count = 1
		if x86.@1.type = 'far'
			if x86.@1.jump_type & x86.@1.jump_type <> 'far'
				err 'invalid operand'
			else if x86.mode < 64
				if x86.@1.size = 0
					if x86.mode = 16
						db 9Ah
						dw x86.@1.offset,x86.@1.segment
					else
						db 9Ah
						dd x86.@1.offset
						dw x86.@1.segment
					end if
				else if x86.@1.size = 4 | x86.@1.size = 6
					x86.store_operand_prefix (x86.@1.size-2)
					db 9Ah
					if x86.@1.size = 4
						dw x86.@1.offset,x86.@1.segment
					else
						dd x86.@1.offset
						dw x86.@1.segment
					end if
				else
					err 'invalid operand size'
				end if
			else
				err 'illegal instruction'
			end if
		else if x86.@1.type = 'mem' | x86.@1.type = 'reg'
			if x86.@1.size = 6 | x86.@1.size = 10
				if x86.@1.jump_type & x86.@1.jump_type <> 'far'
					err 'invalid operand'
				end if
				x86.select_operand_prefix x86.@1,(x86.@1.size-2)
				x86.store_instruction 0FFh,x86.@1,11b
			else if x86.@1.size = 8 & x86.mode = 64
				if x86.@1.jump_type & x86.@1.jump_type <> 'near'
					err 'invalid operand'
				end if
				x86.store_instruction 0FFh,x86.@1,10b
			else if x86.@1.size = 4 & x86.mode < 64
				if x86.@1.jump_type | x86.@1.type = 'reg'
					if x86.@1.jump_type = 'far'
						x86.select_operand_prefix x86.@1,2
						x86.store_instruction 0FFh,x86.@1,11b
					else
						x86.select_operand_prefix x86.@1,4
						x86.store_instruction 0FFh,x86.@1,10b
					end if
				else
					if x86.mode = 16
						x86.select_operand_prefix x86.@1,2
						x86.store_instruction 0FFh,x86.@1,11b
					else
						x86.select_operand_prefix x86.@1,4
						x86.store_instruction 0FFh,x86.@1,10b
					end if
				end if
			else if x86.@1.size = 2 & x86.mode < 64
				if x86.@1.jump_type & x86.@1.jump_type <> 'near'
					err 'invalid operand'
				end if
				x86.select_operand_prefix x86.@1,2
				x86.store_instruction 0FFh,x86.@1,10b
			else if x86.@1.size = 0
				if x86.mode = 64
					x86.@1.prefix = 48h
				end if
				if x86.@1.jump_type = 'far'
					x86.store_instruction 0FFh,x86.@1,11b
				else if x86.@1.jump_type = 'near'
					x86.store_instruction 0FFh,x86.@1,10b
				else
					err 'operand size not specified'
				end if
			else
				err 'invalid operand'
			end if
		else if x86.@1.type = 'imm'
			if x86.@1.jump_type & x86.@1.jump_type <> 'near'
				err 'invalid operand'
			end if
			if x86.@1.size = 8 & x86.mode = 64
				if x86.@1.imm relativeto $ & (x86.@1.imm-($+4) < -80000000h | x86.@1.imm-($+4) >= 80000000h)
					err 'relative jump out of range'
				else
					db 0E8h
					dd x86.@1.imm-($+4)
				end if
			else if x86.@1.size = 2 & x86.mode <> 64
				x86.store_operand_prefix 2
				db 0E8h
				dw x86.@1.imm-($+2)
			else if x86.@1.size = 4 & x86.mode <> 64
				x86.store_operand_prefix 4
				db 0E8h
				dd x86.@1.imm-($+4)
			else
				err 'invalid operand size'
			end if
		else
			err 'invalid operand'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro jmp? args&
	x86.parse_jump_operands args
	if x86.@count = 1
		if x86.@1.type = 'far'
			if x86.@1.jump_type & x86.@1.jump_type <> 'far'
				err 'invalid operand'
			else if x86.mode < 64
				if x86.@1.size = 0
					db 0EAh
					if x86.mode = 16
						dw x86.@1.offset,x86.@1.segment
					else
						dd x86.@1.offset
						dw x86.@1.segment
					end if
				else if x86.@1.size = 4 | x86.@1.size = 6
					x86.store_operand_prefix (x86.@1.size-2)
					db 0EAh
					if x86.@1.size = 4
						dw x86.@1.offset,x86.@1.segment
					else
						dd x86.@1.offset
						dw x86.@1.segment
					end if
				else
					err 'invalid operand size'
				end if
			else
				err 'illegal instruction'
			end if
		else if x86.@1.type = 'mem' | x86.@1.type = 'reg'
			if x86.@1.size = 6 | x86.@1.size = 10
				if x86.@1.jump_type & x86.@1.jump_type <> 'far'
					err 'invalid operand'
				end if
				x86.select_operand_prefix x86.@1,(x86.@1.size-2)
				x86.store_instruction 0FFh,x86.@1,101b
			else if x86.@1.size = 8 & x86.mode = 64
				if x86.@1.jump_type & x86.@1.jump_type <> 'near'
					err 'invalid operand'
				end if
				x86.store_instruction 0FFh,x86.@1,100b
			else if x86.@1.size = 4 & x86.mode < 64
				if x86.@1.jump_type | x86.@1.type = 'reg'
					if x86.@1.jump_type = 'far'
						x86.select_operand_prefix x86.@1,2
						x86.store_instruction 0FFh,x86.@1,101b
					else
						x86.select_operand_prefix x86.@1,4
						x86.store_instruction 0FFh,x86.@1,100b
					end if
				else
					if x86.mode = 16
						x86.select_operand_prefix x86.@1,2
						x86.store_instruction 0FFh,x86.@1,101b
					else
						x86.select_operand_prefix x86.@1,4
						x86.store_instruction 0FFh,x86.@1,100b
					end if
				end if
			else if x86.@1.size = 2 & x86.mode < 64
				if x86.@1.jump_type & x86.@1.jump_type <> 'near'
					err 'invalid operand'
				end if
				x86.select_operand_prefix x86.@1,2
				x86.store_instruction 0FFh,x86.@1,100b
			else if x86.@1.size = 0
				if x86.@1.jump_type = 'far'
					x86.store_instruction 0FFh,x86.@1,101b
				else if x86.@1.jump_type = 'near'
					x86.store_instruction 0FFh,x86.@1,100b
				else
					err 'operand size not specified'
				end if
			else
				err 'invalid operand size'
			end if
		else if x86.@1.type = 'imm'
			if x86.@1.size = 8 & x86.mode = 64
				if x86.@1.jump_type = 'short'
					db 0EBh
					if x86.@1.imm-($+1) < 80h & x86.@1.imm-($+1) >= -80h
						db x86.@1.imm-($+1)
					else
						err 'relative jump out of range'
						db ?
					end if
				else if x86.@1.jump_type = 'near'
					if x86.@1.imm relativeto $ & (x86.@1.imm-($+4) < -80000000h | x86.@1.imm-($+4) >= 80000000h)
						err 'relative jump out of range'
					else
						db 0E9h
						dd x86.@1.imm-($+4)
					end if
				else if ~ x86.@1.jump_type
					if ~ $ relativeto 0 & x86.@1.imm relativeto 0
						x86.@1.imm = x86.@1.imm + $ - $ scale 0
						err 'invalid address'
					end if
					if ( x86.@1.imm relativeto $ & x86.@1.imm-($+2) < 80h & x86.@1.imm-($+2) >= -80h ) | x86.@1.unresolved
						db 0EBh
						db x86.@1.imm-($+1)
					else if x86.@1.imm relativeto $ & (x86.@1.imm-($+4) < -80000000h | x86.@1.imm-($+4) >= 80000000h)
						err 'relative jump out of range'
					else
						db 0E9h
						dd x86.@1.imm-($+4)
					end if
				else
					err 'invalid operand'
				end if
			else if x86.@1.size = 2 & x86.mode <> 64
				x86.store_operand_prefix 2
				if x86.@1.jump_type = 'near'
					db 0E9h
					dw x86.@1.imm-($+2)
				else if x86.@1.jump_type = 'short'
					db 0EBh
					if (x86.@1.imm-($+2)) and 0FFFFh < 80h | (x86.@1.imm-($+2)) and 0FFFFh >= 0FF80h
						db (x86.@1.imm-($+1)) and 0FFh
					else
						err 'relative jump out of range'
						db ?
					end if
				else if ~ x86.@1.jump_type
					if ~ $ relativeto 0 & x86.@1.imm relativeto 0
						x86.@1.imm = x86.@1.imm + $ - $ scale 0
						err 'invalid address'
					end if
					if x86.@1.unresolved | ( x86.@1.imm relativeto $ & ( (x86.@1.imm-($+2)) and 0FFFFh < 80h | (x86.@1.imm-($+2)) and 0FFFFh >= 0FF80h ) )
						db 0EBh
						db (x86.@1.imm-($+1)) and 0FFh
					else

						db 0E9h
						if x86.@1.imm relativeto $
							dw (x86.@1.imm-($+2)) and 0FFFFh
						else
							dw x86.@1.imm-($+2)
						end if
					end if
				else
					err 'invalid operand'
				end if
			else if x86.@1.size = 4 & x86.mode <> 64
				x86.store_operand_prefix 4
				if x86.@1.jump_type = 'near'
					db 0E9h
					dd x86.@1.imm-($+4)
				else if x86.@1.jump_type = 'short'
					db 0EBh
					if x86.@1.imm-($+1) < 80h & x86.@1.imm-($+1) >= -80h
						db x86.@1.imm-($+1)
					else
						err 'relative jump out of range'
						db ?
					end if
				else if ~ x86.@1.jump_type
					if ~ $ relativeto 0 & x86.@1.imm relativeto 0
						x86.@1.imm = x86.@1.imm + $ - $ scale 0
						err 'invalid address'
					end if
					if x86.@1.unresolved | ( x86.@1.imm relativeto $ & x86.@1.imm-($+2) < 80h & x86.@1.imm-($+2) >= -80h )
						db 0EBh
						db x86.@1.imm-($+1)
					else
						db 0E9h
						dd x86.@1.imm-($+4)
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
	else
		err 'invalid number of operands'
	end if
end macro

iterate <instr,opcode>, jo,70h, jno,71h, jc,72h, jb,72h, jnae,72h, jnc,73h, jnb,73h, jae,73h, jz,74h, je,74h, jnz,75h, jne,75h, jna,76h, jbe,76h, ja,77h, jnbe,77h, \
			js,78h, jns,79h, jp,7Ah, jpe,7Ah, jnp,7Bh, jpo,7Bh, jl,7Ch, jnge,7Ch, jnl,7Dh, jge,7Dh, jng,7Eh, jle,7Eh, jg,7Fh, jnle,7Fh
	macro instr? args&
		x86.parse_jump_operands args
		if x86.@count = 1
			if x86.@1.type = 'imm' & x86.@1.jump_type <> 'far'
				if x86.mode <> 64 & x86.@1.size <> 8
					x86.store_operand_prefix x86.@1.size
				else if x86.mode = 64 & x86.@1.size <> 8
					err 'invalid operand size'
				end if
				if ~ $ relativeto 0 & x86.@1.imm relativeto 0
					x86.@1.imm = x86.@1.imm + $ - $ scale 0
					err 'invalid address'
				end if
				if x86.@1.jump_type <> 'near' & ( x86.@1.unresolved | ( x86.@1.imm relativeto $ & x86.@1.imm-($+2) < 80h & x86.@1.imm-($+2) >= -80h ) )
					db opcode
					db x86.@1.imm-($+1)
				else if x86.@1.jump_type <> 'near' & x86.@1.size = 2 & x86.@1.imm relativeto $ & ( (x86.@1.imm-($+2)) and 0FFFFh < 80h | (x86.@1.imm-($+2)) and 0FFFFh >= 0FF80h )
					db opcode
					db (x86.@1.imm-($+1)) and 0FFh
				else if x86.@1.jump_type = 'short' | x86.cpu < 3
					err 'relative jump out of range'
					db ?,?
				else
					db 0Fh,10h+opcode
					if x86.@1.size = 2
						if x86.@1.imm relativeto $
							dw (x86.@1.imm-($+2)) and 0FFFFh
						else
							dw x86.@1.imm-($+2)
						end if
					else
						dd x86.@1.imm-($+4)
					end if
				end if
			else
				err 'invalid operand'
			end if
		else
			err 'invalid number of operands'
		end if
	end macro
end iterate

iterate <instr,opcode,len>, loopnz,0E0h,0, loopne,0E0h,0, loopz,0E1h,0, loope,0E1h,0, loop,0E2h,0, \
			    loopnzw,0E0h,2, loopnew,0E0h,2, loopzw,0E1h,2, loopew,0E1h,2, loopw,0E2h,2, \
			    loopnzd,0E0h,4, loopned,0E0h,4, loopzd,0E1h,4, looped,0E1h,4, loopd,0E2h,4, \
			    loopnzq,0E0h,8, loopneq,0E0h,8, loopzq,0E1h,8, loopeq,0E1h,8, loopq,0E2h,8, \
			    jcxz,0E3h,2, jecxz,0E3h,4, jrcxz,0E3h,8
	macro instr? args&
		x86.parse_jump_operands args
		if x86.@count = 1
			if x86.@1.type = 'imm' & ( x86.@1.jump_type = 'short' | ~ x86.@1.jump_type )
				if len & len shl 3 <> x86.mode
					if len = 8 | (len = 2 & x86.mode = 64)
						err 'illegal instruction'
					end if
					db 67h
					x86.require 80386+
				end if
				if x86.@1.size shl 3 <> x86.mode
					if x86.@1.size = 8 | x86.mode = 64
						err 'invalid operand size'
					end if
					db 66h
					x86.require 80386+
				end if
				db opcode
				if x86.@1.imm-($+1) < 80h & x86.@1.imm-($+1) >= -80h
					db x86.@1.imm-($+1)
				else if x86.@1.size = 2 & ( (x86.@1.imm-($+2)) and 0FFFFh < 80h | (x86.@1.imm-($+2)) and 0FFFFh >= 0FF80h )
					db (x86.@1.imm-($+1)) and 0FFh
				else
					err 'relative jump out of range'
					db ?
				end if
			else
				err 'invalid operand'
			end if
		else
			err 'invalid number of operands'
		end if
	end macro
end iterate

iterate <instr,opcode>, daa,27h, das,2Fh, aaa,37h, aas,3Fh
	macro instr? args&
		x86.parse_operands args
		if x86.mode < 64
			if x86.@count = 0
				db opcode
			else
				err 'unexpected operands'
			end if
		else
			err 'illegal instruction'
		end if
	end macro
end iterate

iterate <instr,opcode>, aam,0D4h, aad,0D5h
	macro instr? args&
		x86.parse_operands args
		if x86.mode < 64
			if x86.@count = 0
				db opcode,10
			else if x86.@count = 1
				if x86.@1.type = 'imm'
					if x86.@1.size & x86.@1.size <> 1
						err 'invalid operand size'
					else if x86.immsize & x86.immsize <> 1
						err 'immediate size mismatch'
					end if
					db opcode,x86.@1.imm
				else
					err 'invalid operand'
				end if
			else
				err 'invalid number of operands'
			end if
		else
			err 'illegal instruction'
		end if
	end macro
end iterate

macro nop? args&
	x86.parse_operands args
	if x86.@count = 0
		db 90h
	else
		err 'unexpected operands'
	end if
end macro

iterate <instr,opcode>, int3,0CCh, into,0CEh, salc,0D6h, hlt,0F4h, cmc,0F5h, clc,0F8h, stc,0F9h, cli,0FAh, sti,0FBh, cld,0FCh, std,0FDh, \
			pushf,9Ch, popf,9Dh, sahf,9Eh, lahf,9Fh, xlatb,0D7h, \
			movsb,0A4h, cmpsb,0A6h, stosb,0AAh, lodsb,0ACh, scasb,0AEh
	macro instr? args&
		x86.parse_operands args
		if x86.@count = 0
			db opcode
		else
			err 'unexpected operands'
		end if
	end macro
end iterate

macro int? args&
	x86.parse_operands args
	if x86.@count = 1
		if x86.@1.type = 'imm'
			if x86.@1.size & x86.@1.size <> 1
				err 'invalid operand size'
			else if x86.immsize & x86.immsize <> 1
				err 'immediate size mismatch'
			end if
			db 0CDh,x86.@1.imm
		else
			err 'invalid operand'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro iret? args&
	x86.parse_operands args
	if x86.@count = 0
		if x86.mode = 64
			db 48h
		end if
		db 0CFh
	else
		err 'unexpected operands'
	end if
end macro

iterate <instr,size,opcode>, cbw,2,98h, cwd,2,99h, cwde,4,98h, cdq,4,99h, cdqe,8,98h, cqo,8,99h, \
			     iretw,2,0CFh, iretd,4,0CFh, iretq,8,0CFh, pushfw,2,9Ch, popfw,2,9Dh, \
			     movsw,2,0A5h, cmpsw,2,0A7h, stosw,2,0ABh, lodsw,2,0ADh, scasw,2,0AFh, \
			     movsd,4,0A5h, cmpsd,4,0A7h, stosd,4,0ABh, lodsd,4,0ADh, scasd,4,0AFh, \
			     movsq,8,0A5h, cmpsq,8,0A7h, stosq,8,0ABh, lodsq,8,0ADh, scasq,8,0AFh
	macro instr? args&
		x86.parse_operands args
		if x86.@count = 0
			x86.store_operand_prefix size
			db opcode
		else
			err 'unexpected operands'
		end if
	end macro
end iterate

iterate <instr,opcode>, pusha,60h, popa,61h
	macro instr? args&
		x86.parse_operands args
		if x86.mode < 64
			x86.require 80186+
			if x86.@count = 0
				db opcode
			else
				err 'unexpected operands'
			end if
		else
			err 'illegal instruction'
		end if
	end macro
end iterate

iterate <instr,size,opcode>, pushaw,2,60h, popaw,2,61h, pushad,4,60h, popad,4,61h, pushfd,4,9Ch, popfd,4,9Dh
	macro instr? args&
		x86.parse_operands args
		if x86.mode < 64
			x86.require 80186+
			if x86.@count = 0
				x86.store_operand_prefix size
				db opcode
			else
				err 'unexpected operands'
			end if
		else
			err 'illegal instruction'
		end if
	end macro
end iterate

iterate <instr,opcode>, pushfd,9Ch, popfd,9Dh
	macro instr? args&
		x86.parse_operands args
		if x86.mode < 64
			if x86.@count = 0
				x86.store_operand_prefix 4
				db opcode
			else
				err 'unexpected operands'
			end if
		else
			err 'illegal instruction'
		end if
	end macro
end iterate

iterate <instr,opcode>, pushfq,9Ch, popfq,9Dh
	macro instr? args&
		x86.parse_operands args
		if x86.mode = 64
			if x86.@count = 0
				x86.store_operand_prefix 4
				db opcode
			else
				err 'unexpected operands'
			end if
		else
			err 'instruction requires long mode'
		end if
	end macro
end iterate

iterate <prefix,code>, lock,0F0h, repnz,0F2h, repne,0F2h, rep,0F3h, repz,0F3h, repe,0F3h
	macro prefix? instr&
		db code
		instr
	end macro
end iterate

iterate <instr,opcode>, insb,06Ch, outsb,06Eh
	macro instr? args&
		x86.parse_operands args
		x86.require 80186+
		if x86.@count = 0
			db opcode
		else
			err 'unexpected operands'
		end if
	end macro
end iterate

iterate <instr,size,opcode>, insw,2,06Dh, outsw,2,06Fh, insd,4,06Dh, outsd,4,06Fh
	macro instr? args&
		x86.parse_operands args
		x86.require 80186+
		if x86.@count = 0
			x86.store_operand_prefix size
			db opcode
		else
			err 'unexpected operands'
		end if
	end macro
end iterate

macro movs? args&
	x86.parse_operands args
	if x86.@count = 2
		x86.size = 0
		if x86.@1.size = 0 & x86.@2.size = 0
			err 'operand size not specified'
		else if x86.@1.size <> 0 & x86.@2.size <> 0 & x86.@1.size <> x86.@2.size
			err 'operand sizes do not match'
		else
			x86.size = 0 = x86.@1.size or x86.@2.size
		end if
		if x86.@2.type = 'mem' & x86.@2.mod = 0 & x86.@1.type = 'mem' & x86.@1.mod = 0 & ( (x86.mode < 64 & x86.@2.mode = 16 & x86.@2.rm = 4 & x86.@1.mode = 16 & x86.@1.rm = 5) | (@2.mode > 16 & x86.@2.rm = 6 & x86.@1.mode = x86.@2.mode & x86.@1.rm = 7) ) & ( x86.@1.segment_prefix = 0 | x86.@1.segment_prefix = 26h | (x86.mode = 64 & x86.@1.segment_prefix < 64h))
			if x86.@2.segment_prefix & x86.@2.segment_prefix <> 3Eh & (x86.mode < 64 | x86.@2.segment_prefix >= 64h)
				db x86.@2.segment_prefix
			end if
			if x86.size > 1
				if x86.@1.mode <> x86.mode
					db 67h
				end if
				x86.store_operand_prefix x86.size
				db 0A5h
			else
				if x86.@1.mode <> x86.mode
					db 67h
				end if
				db 0A4h
			end if
		else
			err 'invalid operand'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro cmps? args&
	x86.parse_operands args
	if x86.@count = 2
		x86.size = 0
		if x86.@2.size = 0 & x86.@1.size = 0
			err 'operand size not specified'
		else if x86.@2.size <> 0 & x86.@1.size <> 0 & x86.@2.size <> x86.@1.size
			err 'operand sizes do not match'
		else
			x86.size = x86.@2.size or x86.@1.size
		end if
		if x86.@1.type = 'mem' & x86.@1.mod = 0 & x86.@2.type = 'mem' & x86.@2.mod = 0 & ( (x86.mode < 64 & x86.@1.mode = 16 & x86.@1.rm = 4 & x86.@2.mode = 16 & x86.@2.rm = 5) | (@1.mode > 16 & x86.@1.rm = 6 & x86.@2.mode = x86.@1.mode & x86.@2.rm = 7) ) & ( x86.@2.segment_prefix = 0 | x86.@2.segment_prefix = 26h | (x86.mode = 64 & x86.@2.segment_prefix < 64h))
			if x86.@1.segment_prefix & x86.@1.segment_prefix <> 3Eh & (x86.mode < 64 | x86.@1.segment_prefix >= 64h)
				db x86.@1.segment_prefix
			end if
			if x86.size > 1
				if x86.@2.mode <> x86.mode
					db 67h
				end if
				x86.store_operand_prefix x86.size
				db 0A7h
			else
				if x86.@2.mode <> x86.mode
					db 67h
				end if
				db 0A6h
			end if
		else
			err 'invalid operand'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro stos? args&
	x86.parse_operands args
	if x86.@count = 1
		if x86.@1.size = 0
			err 'operand size not specified'
		end if
		if x86.@1.type = 'mem' & x86.@1.mod = 0 & ( (x86.mode < 64 & x86.@1.mode = 16 & x86.@1.rm = 5) | (@1.mode > 16 & x86.@1.rm = 7) ) & ( x86.@1.segment_prefix = 0 | x86.@1.segment_prefix = 26h | (x86.mode = 64 & x86.@1.segment_prefix < 64h))
			if x86.@1.size > 1
				if x86.@1.mode <> x86.mode
					db 67h
				end if
				x86.store_operand_prefix x86.@1.size
				db 0ABh
			else
				if x86.@1.mode <> x86.mode
					db 67h
				end if
				db 0AAh
			end if
		else
			err 'invalid operand'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro lods? src*
	x86.parse_operands args
	if x86.@count = 1
		if x86.@1.size = 0
			err 'operand size not specified'
		end if
		if x86.@1.type = 'mem' & x86.@1.mod = 0 & ( (x86.mode < 64 & x86.@1.mode = 16 & x86.@1.rm = 4) | (@1.mode > 16 & x86.@1.rm = 6) )
			if x86.@1.segment_prefix & x86.@1.segment_prefix <> 3Eh & (x86.mode < 64 | x86.@1.segment_prefix >= 64h)
				db x86.@1.segment_prefix
			end if
			if x86.@1.size > 1
				if x86.@1.mode <> x86.mode
					db 67h
				end if
				x86.store_operand_prefix x86.@1.size
				db 0ADh
			else
				if x86.@1.mode <> x86.mode
					db 67h
				end if
				db 0ACh
			end if
		else
			err 'invalid operand'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro scas? args&
	x86.parse_operands args
	if x86.@count = 1
		if x86.@1.size = 0
			err 'operand size not specified'
		end if
		if x86.@1.type = 'mem' & x86.@1.mod = 0 & ( (x86.mode < 64 & x86.@1.mode = 16 & x86.@1.rm = 5) | (@1.mode > 16 & x86.@1.rm = 7) ) & ( x86.@1.segment_prefix = 0 | x86.@1.segment_prefix = 26h | (x86.mode = 64 & x86.@1.segment_prefix < 64h))
			if x86.@1.size > 1
				if x86.@1.mode <> x86.mode
					db 67h
				end if
				x86.store_operand_prefix x86.@1.size
				db 0AFh
			else
				if x86.@1.mode <> x86.mode
					db 67h
				end if
				db 0AEh
			end if
		else
			err 'invalid operand'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro ins? args&
	x86.parse_operands args
	x86.require 80186+
	if x86.@count = 2
		if x86.@1.size = 0
			err 'operand size not specified'
		else if x86.@1.size = 8
			err 'invalid operand size'
		end if
		if x86.@2.type = 'reg' & x86.@2.size = 2 & x86.@2.rm = 2 & x86.@1.type = 'mem' & x86.@1.mod = 0 & ( (x86.mode < 64 & x86.@1.mode = 16 & x86.@1.rm = 5) | (@1.mode > 16 & x86.@1.rm = 7) ) & ( x86.@1.segment_prefix = 0 | x86.@1.segment_prefix = 26h | (x86.mode = 64 & x86.@1.segment_prefix < 64h))
			if x86.@1.size > 1
				if x86.@1.mode <> x86.mode
					db 67h
				end if
				x86.store_operand_prefix x86.@1.size
				db 06Dh
			else
				if x86.@1.mode <> x86.mode
					db 67h
				end if
				db 06Ch
			end if
		else
			err 'invalid operand'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro outs? args&
	x86.parse_operands args
	x86.require 80186+
	if x86.@count = 2
		if x86.@2.size = 0
			err 'operand size not specified'
		else if x86.@2.size = 8
			err 'invalid operand size'
		end if
		if x86.@1.type = 'reg' & x86.@1.size = 2 & x86.@1.rm = 2 & x86.@2.type = 'mem' & x86.@2.mod = 0 & ( (x86.mode < 64 & x86.@2.mode = 16 & x86.@2.rm = 4) | (@2.mode > 16 & x86.@2.rm = 6) )
			if x86.@2.segment_prefix & x86.@2.segment_prefix <> 3Eh & (x86.mode < 64 | x86.@2.segment_prefix >= 64h)
				db x86.@2.segment_prefix
			end if
			if x86.@2.size > 1
				if x86.@2.mode <> x86.mode
					db 67h
				end if
				x86.store_operand_prefix x86.@2.size
				db 06Fh
			else
				if x86.@2.mode <> x86.mode
					db 67h
				end if
				db 06Eh
			end if
		else
			err 'invalid operand'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro xlat? src*
	x86.parse_operands args
	if x86.@count = 1
		if x86.@1.size > 1
			err 'invalid operand size'
		end if
		if x86.@1.type = 'mem' & x86.@1.mod = 0 & ( (x86.mode < 64 & x86.@1.mode = 16 & x86.@1.rm = 7) | (x86.@1.mode > 16 & x86.@1.rm = 3) )
			if x86.@1.segment_prefix & x86.@1.segment_prefix <> 3Eh & (x86.mode < 64 | x86.@1.segment_prefix >= 64h)
				db x86.@1.segment_prefix
			end if
			if x86.@1.mode <> x86.mode
				db 67h
			end if
			db 0D7h
		else
			err 'invalid operand'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro in? args&
	x86.parse_operands args
	if x86.@count = 2
		if x86.@1.size = 0
			err 'operand size not specified'
		else if x86.@1.size = 8
			err 'invalid operand size'
		end if
		if x86.@2.type = 'reg' & x86.@2.size = 2 & x86.@2.rm = 2 & x86.@1.type = 'reg' & x86.@1.rm = 0
			if x86.@1.size > 1
				x86.store_operand_prefix x86.@1.size
				db 0EDh
			else
				db 0ECh
			end if
		else if x86.@2.type = 'imm' & x86.@1.type = 'reg' & x86.@1.rm = 0
			if x86.@1.size > 1
				x86.store_operand_prefix x86.@1.size
				db 0E5h,x86.@2.imm
			else
				db 0E4h,x86.@2.imm
			end if
		else
			err 'invalid combination of operands'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro out? args&
	x86.parse_operands args
	if x86.@count = 2
		if x86.@2.size = 0
			err 'operand size not specified'
		else if x86.@2.size = 8
			err 'invalid operand size'
		end if
		if x86.@1.type = 'reg' & x86.@1.size = 2 & x86.@1.rm = 2 & x86.@2.type = 'reg' & x86.@2.rm = 0
			if x86.@2.size > 1
				x86.store_operand_prefix x86.@2.size
				db 0EFh
			else
				db 0EEh
			end if
		else if x86.@1.type = 'imm' & x86.@2.type = 'reg' & x86.@2.rm = 0
			if x86.@2.size > 1
				x86.store_operand_prefix x86.@2.size
				db 0E7h,x86.@1.imm
			else
				db 0E6h,x86.@1.imm
			end if
		else
			err 'invalid combination of operands'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro bound? dest*,src*
	x86.parse_operands args
	if x86.mode < 64
		x86.require 80186+
		if x86.@count = 2
				x86.size = 0
				if x86.@1.size <> 0 & x86.@2.size <> 0 & x86.@1.size <> x86.@2.size
					err 'operand sizes do not match'
				else
					x86.size = x86.@1.size or x86.@2.size
				end if
				if x86.@2.type = 'mem' & x86.@1.type = 'reg'
					x86.select_operand_prefix x86.@2,x86.size
					x86.store_instruction 62h,x86.@2,x86.@1.rm
				else
					err 'invalid combination of operands'
				end if
		else
			err 'invalid number of operands'
		end if
	else
		err 'illegal instruction'
	end if
end macro

macro enter? args&
	x86.parse_operands args
	x86.require 80186+
	if (x86.@1.size <> 0 & x86.@1.size <> 2) | (x86.@2.size <> 0 & x86.@2.size <> 1)
		err 'invalid operand size'
	else if x86.immsize
		err 'immediate size mismatch'
	end if
	if x86.@1.type = 'imm' & x86.@2.type = 'imm'
		db 0C8h
		dw x86.@1.imm
		db x86.@2.imm
	else
		err 'invalid operand'
	end if
end macro

macro leave? args&
	x86.parse_operands args
	x86.require 80186+
	if x86.@count = 0
		db 0C9h
	else
		err 'unexpected operands'
	end if
end macro

macro x86.setcc_instruction ext,args&
	x86.parse_operands args
	x86.require 80386+
	if x86.@count = 1
		if x86.@1.size > 1
			err 'invalid operand size'
		end if
		if x86.@1.type = 'reg' | x86.@1.type = 'mem'
			x86.store_instruction <0Fh,ext>,x86.@1,0
		else
			err 'invalid operand'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro seto? args&
	x86.setcc_instruction 90h,args
end macro

macro setno? args&
	x86.setcc_instruction 91h,args
end macro

macro setc? args&
	x86.setcc_instruction 92h,args
end macro

macro setb? args&
	x86.setcc_instruction 92h,args
end macro

macro setnae? args&
	x86.setcc_instruction 92h,args
end macro

macro setnc? args&
	x86.setcc_instruction 93h,args
end macro

macro setnb? args&
	x86.setcc_instruction 93h,args
end macro

macro setae? args&
	x86.setcc_instruction 93h,args
end macro

macro setz? args&
	x86.setcc_instruction 94h,args
end macro

macro sete? args&
	x86.setcc_instruction 94h,args
end macro

macro setnz? args&
	x86.setcc_instruction 95h,args
end macro

macro setne? args&
	x86.setcc_instruction 95h,args
end macro

macro setna? args&
	x86.setcc_instruction 96h,args
end macro

macro setbe? args&
	x86.setcc_instruction 96h,args
end macro

macro seta? args&
	x86.setcc_instruction 97h,args
end macro

macro setnbe? args&
	x86.setcc_instruction 97h,args
end macro

macro sets? args&
	x86.setcc_instruction 98h,args
end macro

macro setns? args&
	x86.setcc_instruction 99h,args
end macro

macro setp? args&
	x86.setcc_instruction 9Ah,args
end macro

macro setpe? args&
	x86.setcc_instruction 9Ah,args
end macro

macro setnp? args&
	x86.setcc_instruction 9Bh,args
end macro

macro setpo? args&
	x86.setcc_instruction 9Bh,args
end macro

macro setl? args&
	x86.setcc_instruction 9Ch,args
end macro

macro setnge? args&
	x86.setcc_instruction 9Ch,args
end macro

macro setnl? args&
	x86.setcc_instruction 9Dh,args
end macro

macro setge? args&
	x86.setcc_instruction 9Dh,args
end macro

macro setng? args&
	x86.setcc_instruction 9Eh,args
end macro

macro setle? args&
	x86.setcc_instruction 9Eh,args
end macro

macro setg? args&
	x86.setcc_instruction 9Fh,args
end macro

macro setnle? args&
	x86.setcc_instruction 9Fh,args
end macro

macro x86.conditional_move ext,args&
	x86.parse_operands args
	x86.require P6+
	if x86.@count = 2
		if x86.@1.size <> 0 & x86.@2.size <> 0 & x86.@1.size <> x86.@2.size
			err 'operand sizes do not match'
		end if
		if x86.@1.type = 'reg' & (x86.@2.type = 'mem' | x86.@2.type = 'reg')
			x86.select_operand_prefix x86.@2,x86.@1.size
			x86.store_instruction <0Fh,ext>,x86.@2,x86.@1.rm
		else
			err 'invalid combination of operands'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro cmovo? args&
	x86.conditional_move 40h,args
end macro

macro cmovno? args&
	x86.conditional_move 41h,args
end macro

macro cmovb? args&
	x86.conditional_move 42h,args
end macro

macro cmovc? args&
	x86.conditional_move 42h,args
end macro

macro cmovnae? args&
	x86.conditional_move 42h,args
end macro

macro cmovae? args&
	x86.conditional_move 43h,args
end macro

macro cmovnb? args&
	x86.conditional_move 43h,args
end macro

macro cmovnc? args&
	x86.conditional_move 43h,args
end macro

macro cmove? args&
	x86.conditional_move 44h,args
end macro

macro cmovz? args&
	x86.conditional_move 44h,args
end macro

macro cmovne? args&
	x86.conditional_move 45h,args
end macro

macro cmovnz? args&
	x86.conditional_move 45h,args
end macro

macro cmovbe? args&
	x86.conditional_move 46h,args
end macro

macro cmovna? args&
	x86.conditional_move 46h,args
end macro

macro cmova? args&
	x86.conditional_move 47h,args
end macro

macro cmovnbe? args&
	x86.conditional_move 47h,args
end macro

macro cmovs? args&
	x86.conditional_move 48h,args
end macro

macro cmovns? args&
	x86.conditional_move 49h,args
end macro

macro cmovp? args&
	x86.conditional_move 4Ah,args
end macro

macro cmovpe? args&
	x86.conditional_move 4Ah,args
end macro

macro cmovnp? args&
	x86.conditional_move 4Bh,args
end macro

macro cmovpo? args&
	x86.conditional_move 4Bh,args
end macro

macro cmovl? args&
	x86.conditional_move 4Ch,args
end macro

macro cmovnge? args&
	x86.conditional_move 4Ch,args
end macro

macro cmovge? args&
	x86.conditional_move 4Dh,args
end macro

macro cmovnl? args&
	x86.conditional_move 4Dh,args
end macro

macro cmovle? args&
	x86.conditional_move 4Eh,args
end macro

macro cmovng? args&
	x86.conditional_move 4Eh,args
end macro

macro cmovg? args&
	x86.conditional_move 4Fh,args
end macro

macro cmovnle? args&
	x86.conditional_move 4Fh,args
end macro

iterate <instr,opcode>, swapgs,<0Fh,1,0F8h>, syscall,<0Fh,5>, sysret,<0Fh,7>, sysretq,<48h,0Fh,7>
	macro instr? args&
		x86.parse_operands args
		if x86.mode = 64
			if x86.@count = 0
				db opcode
			else
				err 'unexpected operands'
			end if
		else
			err 'instruction requires long mode'
		end if
	end macro
end iterate

iterate <instr,mincpu,opcode>, clts,80286,<0Fh,06h>, invd,80486,<0Fh,8>, wbinvd,80486,<0Fh,9>, cpuid,P5,<0Fh,0A2h>, \
			       wrmsr,P5,<0Fh,30h>, rdtsc,P5,<0Fh,31h>, rdmsr,P5,<0Fh,32h>, rdpmc,P5,<0Fh,33h>, rsm,P5,<0Fh,0AAh>, \
			       sysenter,P6,<0Fh,34h>, sysexit,P6,<0Fh,35h>
	macro instr? args&
		x86.parse_operands args
		x86.require mincpu+
		if x86.@count = 0
			db opcode
		else
			err 'unexpected operands'
		end if
	end macro
end iterate

iterate <instr,opcode>, wrmsrq,<48h,0Fh,30h>, rdmsrq,<48h,0Fh,32h>, sysexitq,<48h,0Fh,35h>
	macro instr? args&
		x86.parse_operands args
		if x86.mode = 64
			if x86.@count = 0
				db opcode
			else
				err 'unexpected operands'
			end if
		else
			err 'instruction requires long mode'
		end if
	end macro
end iterate

macro loadall? args&
	if x86.cpu = 2
		x86.parse_operands args
		if x86.@count = 0
			db 0Fh,05h
		else
			err 'unexpected operands'
		end if
	else if x86.cpu = 3
		x86.parse_operands args
		if x86.@count = 0
			db 0Fh,07h
		else
			err 'unexpected operands'
		end if
	else
		err 'illegal instruction'
	end if
end macro

macro arpl? args&
	x86.parse_operands args
	if x86.mode < 64
		x86.require 80286+
		if x86.@count = 2
			x86.size = 0
			if x86.@1.size <> 0 & x86.@2.size <> 0 & x86.@1.size <> x86.@2.size
				err 'operand sizes do not match'
			else
				x86.size = x86.@1.size or x86.@2.size
			end if
			if x86.@2.type = 'reg' & (x86.@1.type = 'mem' | x86.@1.type = 'reg')
				x86.select_operand_prefix x86.@1,x86.size
				x86.store_instruction <63h>,x86.@1,x86.@2.rm
			else
				err 'invalid combination of operands'
			end if
		else
			err 'invalid number of operands'
		end if
	else
		err 'illegal instruction'
	end if
end macro

iterate <instr,ext,postbyte>, lldt,0,2, ltr,0,3, verr,0,4, verw,0,5, lmsw,1,6
	macro instr? args&
		x86.parse_operands args
		x86.require 80286+
		if x86.@count = 1
			if x86.@1.size <> 0 & x86.@1.size <> 2
				err 'invalid operand size'
			end if
			if x86.@1.type = 'reg' | x86.@1.type = 'mem'
				x86.store_instruction <0Fh,ext>,x86.@1,postbyte
			else
				err 'invalid operand'
			end if
		else
			err 'invalid number of operands'
		end if
	end macro
end iterate

iterate <instr,ext,postbyte>, sldt,0,0, str,0,1, smsw,1,4
	macro instr? args&
		x86.parse_operands args
		x86.require 80286+
		if x86.@count = 1
			if x86.@1.type = 'reg'
				x86.select_operand_prefix x86.@1,x86.@1.size
				x86.store_instruction <0Fh,ext>,x86.@1,postbyte
			else if x86.@1.type = 'mem'
				if x86.@1.size <> 0 & x86.@1.size <> 2
					err 'invalid operand size'
				end if
				x86.store_instruction <0Fh,ext>,x86.@1,postbyte
			else
				err 'invalid operand'
			end if
		else
			err 'invalid number of operands'
		end if
	end macro
end iterate

iterate <instr,postbyte>, lgdt,2, lidt,3, sgdt,0, sidt,1
	macro instr? args&
		x86.parse_operands args
		x86.require 80286+
		if x86.@count = 1
			if x86.@1.size <> 0 & ( (x86.mode < 64 & x86.@1.size <> 5 & x86.@1.size <> 6) | (x86.mode = 64 & x86.@1.size <> 10) )
				err 'invalid operand size'
			end if
			if x86.@1.type = 'mem'
				if x86.@1.size = 6
					x86.select_operand_prefix x86.@1,4
				else if x86.@1.size = 5
					x86.select_operand_prefix x86.@1,2
				end if
				x86.store_instruction <0Fh,1>,x86.@1,postbyte
			else
				err 'invalid operand'
			end if
		else
			err 'invalid number of operands'
		end if
	end macro
end iterate

iterate <instr,ext>, lar,2, lsl,3
	macro instr? args&
		x86.parse_operands args
		x86.require 80286+
		if x86.@count = 2
			if x86.@1.type = 'reg' & (x86.@2.type = 'mem' | x86.@2.type = 'reg')
				if x86.@2.size and not 2
					err 'invalid operand size'
				end if
				x86.select_operand_prefix x86.@2,x86.@1.size
				x86.store_instruction <0Fh,ext>,x86.@2,x86.@1.rm
			else
				err 'invalid combination of operands'
			end if
		else
			err 'invalid number of operands'
		end if
	end macro
end iterate

macro invlpg? args&
	x86.parse_operands args
	x86.require 80486+
	if x86.@count = 1
		if x86.@1.type = 'mem'
			x86.store_instruction <0Fh,1>,x86.@1,7
		else
			err 'invalid operand'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

iterate <instr,opcode>, fwait,9Bh, wait,9Bh, fnop,<0D9h,0D0h>, fchs,<0D9h,0E0h>, fabs,<0D9h,0E1h>, ftst,<0D9h,0E4h>, fxam,<0D9h,0E5h>, \
			fld1,<0D9h,0E8h>, fldl2t,<0D9h,0E9h>, fldl2e,<0D9h,0EAh>, fldpi,<0D9h,0EBh>, fldlg2,<0D9h,0ECh>, fldln2,<0D9h,0EDh>, fldz,<0D9h,0EEh>, \
			f2xm1,<0D9h,0F0h>, fyl2x,<0D9h,0F1h>, fptan,<0D9h,0F2h>, fpatan,<0D9h,0F3h>, fxtract,<0D9h,0F4h>, fdecstp,<0D9h,0F6h>, fincstp,<0D9h,0F7h>, \
			fprem,<0D9h,0F8h>, fyl2xp1,<0D9h,0F9h>, fsqrt,<0D9h,0FAh>, frndint,<0D9h,0FCh>, fscale,<0D9h,0FDh>, \
			fnclex,<0DBh,0E2h>, fclex,<9Bh,0DBh,0E2h>, fninit,<0DBh,0E3h>, finit,<9Bh,0DBh,0E3h>, \
			fcompp,<0DEh,0D9h>
	macro instr? args&
		x86.parse_operands args
		x86.require 8087+
		if x86.@count = 0
			db opcode
		else
			err 'unexpected operands'
		end if
	end macro
end iterate

iterate <instr,opcode>, fneni,<0DBh,0E0h>, feni,<9Bh,0DBh,0E0h>, fndisi,<0DBh,0E1h>, fdisi,<9Bh,0DBh,0E1h>
	macro instr? args&
		x86.parse_operands args
		x86.require 8087
		if x86.@count = 0
			db opcode
		else
			err 'unexpected operands'
		end if
	end macro
end iterate

macro fsetpm? args&
	x86.parse_operands args
	x86.require 80287
	if x86.@count = 0
		db 0DBh,0E4h
	else
		err 'unexpected operands'
	end if
end macro

iterate <instr,opcode>, fprem1,<0D9h,0F5h>, fsincos,<0D9h,0FBh>, fsin,<0D9h,0FEh>, fcos,<0D9h,0FFh>, fucompp,<0DAh,0E9h>
	macro instr? args&
		x86.parse_operands args
		x86.require 80387+
		if x86.@count = 0
			db opcode
		else
			err 'unexpected operands'
		end if
	end macro
end iterate

iterate <instr,postbyte>, fadd,0, fmul,1, fsub,4, fsubr,5, fdiv,6, fdivr,7
	macro instr? args&
		x86.parse_operands args
		x86.require 8087+
		if x86.@count = 1 & x86.@1.type = 'mem'
			if x86.@1.size = 4
				x86.store_instruction 0D8h,x86.@1,postbyte
			else if x86.@1.size = 8
				x86.store_instruction 0DCh,x86.@1,postbyte
			else if x86.@1.size
				err 'invalid operand size'
			else
				err 'operand size not specified'
			end if
		else if x86.@count = 2 & x86.@1.type = 'streg' & x86.@2.type = 'streg'
			if x86.@1.rm = 0
				db 0D8h, 11b shl 6 + postbyte shl 3 + x86.@2.rm
			else if x86.@2.rm = 0
				if postbyte < 4
					db 0DCh, 11b shl 6 + postbyte shl 3 + x86.@1.rm
				else
					db 0DCh, 11b shl 6 + (postbyte xor 1) shl 3 + x86.@1.rm
				end if
			else
				err 'invalid combination of operands'
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro
end iterate

iterate <instr,postbyte>, faddp,0, fmulp,1, fsubrp,4, fsubp,5, fdivrp,6, fdivp,7
	macro instr? args&
		x86.parse_operands args
		x86.require 8087+
		if x86.@count = 0
			db 0DEh, 11b shl 6 + postbyte shl 3 + 1
		else if x86.@count = 2 & x86.@1.type = 'streg' & x86.@2.type = 'streg' & x86.@2.rm = 0
			db 0DEh, 11b shl 6 + postbyte shl 3 + x86.@1.rm
		else
			err 'invalid combination of operands'
		end if
	end macro
end iterate

iterate <instr,postbyte>, fcom,2, fcomp,3
	macro instr? args&
		x86.parse_operands args
		x86.require 8087+
		if x86.@count = 0
			db 0D8h, 11b shl 6 + postbyte shl 3 + 1
		else if x86.@count = 1 & x86.@1.type = 'streg'
			db 0D8h, 11b shl 6 + postbyte shl 3 + x86.@1.rm
		else if x86.@count = 1 & x86.@1.type = 'mem'
			if x86.@1.size = 4
				x86.store_instruction 0D8h,x86.@1,postbyte
			else if x86.@1.size = 8
				x86.store_instruction 0DCh,x86.@1,postbyte
			else if x86.@1.size
				err 'invalid operand size'
			else
				err 'operand size not specified'
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro
end iterate

iterate <instr,postbyte>, fiadd,0, fimul,1, ficom,2, ficomp,3, fisub,4, fisubr,5, fidiv,6, fidivr,7
	macro instr? args&
		x86.parse_operand args
		x86.require 8087+
		if x86.@count = 1 & x86.@1.type = 'mem'
			if x86.@1.size = 2
				x86.store_instruction 0DEh,x86.@1,postbyte
			else if x86.@1.size = 4
				x86.store_instruction 0DAh,x86.@1,postbyte
			else if x86.@1.size
				err 'invalid operand size'
			else
				err 'operand size not specified'
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro
end iterate

macro fld? args&
	x86.parse_operands args
	x86.require 8087+
	if x86.@count = 1 & x86.@1.type = 'mem'
		if x86.@1.size = 4
			x86.store_instruction 0D9h,x86.@1,0
		else if x86.@1.size = 8
			x86.store_instruction 0DDh,x86.@1,0
		else if x86.@1.size = 10
			x86.store_instruction 0DBh,x86.@1,5
		else if x86.@1.size
			err 'invalid operand size'
		else
			err 'operand size not specified'
		end if
	else if x86.@count = 1 & x86.@1.type = 'streg'
		x86.store_instruction 0D9h,x86.@1,0
	else
		err 'invalid combination of operands'
	end if
end macro

macro fst? args&
	x86.parse_operands args
	x86.require 8087+
	if x86.@count = 1 & x86.@1.type = 'mem'
		if x86.@1.size = 4
			x86.store_instruction 0D9h,x86.@1,2
		else if x86.@1.size = 8
			x86.store_instruction 0DDh,x86.@1,2
		else if x86.@1.size
			err 'invalid operand size'
		else
			err 'operand size not specified'
		end if
	else if x86.@count = 1 & x86.@1.type = 'streg'
		x86.store_instruction 0DDh,x86.@1,2
	else
		err 'invalid combination of operands'
	end if
end macro

macro fstp? args&
	x86.parse_operands args
	x86.require 8087+
	if x86.@count = 1 & x86.@1.type = 'mem'
		if x86.@1.size = 4
			x86.store_instruction 0D9h,x86.@1,3
		else if x86.@1.size = 8
			x86.store_instruction 0DDh,x86.@1,3
		else if x86.@1.size = 10
			x86.store_instruction 0DBh,x86.@1,7
		else if x86.@1.size
			err 'invalid operand size'
		else
			err 'operand size not specified'
		end if
	else if x86.@count = 1 & x86.@1.type = 'streg'
		x86.store_instruction 0DDh,x86.@1,3
	else
		err 'invalid combination of operands'
	end if
end macro

macro fild? args&
	x86.parse_operands args
	x86.require 8087+
	if x86.@count = 1 & x86.@1.type = 'mem'
		if x86.@1.size = 2
			x86.store_instruction 0DFh,x86.@1,0
		else if x86.@1.size = 4
			x86.store_instruction 0DBh,x86.@1,0
		else if x86.@1.size = 8
			x86.store_instruction 0DFh,x86.@1,5
		else if x86.@1.size
			err 'invalid operand size'
		else
			err 'operand size not specified'
		end if
	else
		err 'invalid combination of operands'
	end if
end macro

macro fist? args&
	x86.parse_operands args
	x86.require 8087+
	if x86.@count = 1 & x86.@1.type = 'mem'
		if x86.@1.size = 2
			x86.store_instruction 0DFh,x86.@1,2
		else if x86.@1.size = 4
			x86.store_instruction 0DBh,x86.@1,2
		else if x86.@1.size
			err 'invalid operand size'
		else
			err 'operand size not specified'
		end if
	else
		err 'invalid combination of operands'
	end if
end macro

macro fistp? args&
	x86.parse_operands args
	x86.require 8087+
	if x86.@count = 1 & x86.@1.type = 'mem'
		if x86.@1.size = 2
			x86.store_instruction 0DFh,x86.@1,3
		else if x86.@1.size = 4
			x86.store_instruction 0DBh,x86.@1,3
		else if x86.@1.size = 8
			x86.store_instruction 0DFh,x86.@1,7
		else if x86.@1.size
			err 'invalid operand size'
		else
			err 'operand size not specified'
		end if
	else
		err 'invalid combination of operands'
	end if
end macro

macro fisttp? args&
	x86.parse_operands args
	x86.require 8087+
	if x86.@count = 1 & x86.@1.type = 'mem'
		if x86.@1.size = 2
			x86.store_instruction 0DFh,x86.@1,1
		else if x86.@1.size = 4
			x86.store_instruction 0DBh,x86.@1,1
		else if x86.@1.size = 8
			x86.store_instruction 0DDh,x86.@1,1
		else if x86.@1.size
			err 'invalid operand size'
		else
			err 'operand size not specified'
		end if
	else
		err 'invalid combination of operands'
	end if
end macro

iterate <instr,postbyte>, fbld,4, fbstp,6
	macro instr? args&
		x86.parse_operands args
		x86.require 8087+
		if x86.@count = 1 & x86.@1.type = 'mem'
			if x86.@1.size = 0 | x86.@1.size = 10
				x86.store_instruction 0DFh,x86.@1,postbyte
			else
				err 'invalid operand size'
			end if
		else
			err 'invalid combination of operands'
		end if
	end macro
end iterate

macro fxch? args&
	x86.parse_operands args
	x86.require 8087+
	if x86.@count = 0
		db 0D9h, 11b shl 6 + 1 shl 3 + 1
	else if x86.@count = 1 & x86.@1.type = 'streg'
		db 0D9h, 11b shl 6 + 1 shl 3 + x86.@1.rm
	else
		err 'invalid operand'
	end if
end macro

macro ffree? args&
	x86.parse_operands args
	x86.require 8087+
	if x86.@count = 1 & x86.@1.type = 'streg'
		db 0DDh, 11b shl 6 + x86.@1.rm
	else
		err 'invalid operand'
	end if
end macro

macro ffreep? args&
	x86.parse_operands args
	x86.require 8087+
	if x86.@count = 1 & x86.@1.type = 'streg'
		db 0DFh, 11b shl 6 + x86.@1.rm
	else
		err 'invalid operand'
	end if
end macro

macro fnstsw? args&
	x86.parse_operands args
	x86.require 8087+
	if x86.@count = 1
		if x86.@1.size & x86.@1.size <> 2
			err 'invalid operand size'
		end if
		if x86.@1.type = 'reg' & x86.@1.rm = 0
			db 0DFh,0E0h
		else if x86.@1.type = 'mem'
			x86.store_instruction 0DDh,x86.@1,7
		else
			err 'invalid operand'
		end if
	else
		err 'invalid number of operands'
	end if
end macro

macro fstsw args&
	fwait
	fnstsw args
end macro

iterate <instr,postbyte>, fldcw,5, fnstcw,7
	macro instr? args&
		x86.parse_operands args
		x86.require 8087+
		if x86.@count = 1
			if x86.@1.size & x86.@1.size <> 2
				err 'invalid operand size'
			end if
			if x86.@1.type = 'mem'
				x86.store_instruction 0D9h,x86.@1,postbyte
			else
				err 'invalid operand'
			end if
		else
			err 'invalid number of operands'
		end if
	end macro
end iterate

macro fstcw args&
	fwait
	fnstcw args
end macro

iterate <instr,postbyte>, fldenv,4, fnstenv,6
	macro instr? args&
		x86.parse_operands args
		x86.require 8087+
		if x86.@count = 1
			if x86.@1.size & x86.@1.size <> 14
				err 'invalid operand size'
			end if
			if x86.@1.type = 'mem'
				x86.store_instruction 0D9h,x86.@1,postbyte
			else
				err 'invalid operand'
			end if
		else
			err 'invalid number of operands'
		end if
	end macro
end iterate

macro fstenv args&
	fwait
	fnstenv args
end macro

iterate <instr,postbyte>, frstor,4, fnsave,6
	macro instr? args&
		x86.parse_operands args
		x86.require 8087+
		if x86.@count = 1
			if x86.@1.size & x86.@1.size <> 94
				err 'invalid operand size'
			end if
			if x86.@1.type = 'mem'
				x86.store_instruction 0DDh,x86.@1,postbyte
			else
				err 'invalid operand'
			end if
		else
			err 'invalid number of operands'
		end if
	end macro
end iterate

macro fsave args&
	fwait
	fnsave args
end macro

iterate <instr,postbyte>, fucom,4, fucomp,5
	macro instr? args&
		x86.parse_operands args
		x86.require 80387+
		if x86.@count = 0
			db 0DDh, 11b shl 6 + postbyte shl 3 + 1
		else if x86.@count = 1 & x86.@1.type = 'streg'
			db 0DDh, 11b shl 6 + postbyte shl 3 + x86.@1.rm
		else
			err 'invalid operand'
		end if
	end macro
end iterate

iterate <instr,postbyte>, fldenv,4, fnstenv,6
	macro instr? args&
		x86.parse_operands args
		x86.require 80387+
		if x86.@count = 1
			if x86.@1.size & ( ( x86.mode = 16 & x86.@1.size <> 14 ) | ( x86.mode = 32 & x86.@1.size <> 28 ) )
				err 'invalid operand size'
			end if
			if x86.@1.type = 'mem'
				x86.store_instruction 0D9h,x86.@1,postbyte
			else
				err 'invalid operand'
			end if
		else
			err 'invalid number of operands'
		end if
	end macro
end iterate

iterate <instr,postbyte>, fldenvw,4, fnstenvw,6
	macro instr? args&
		x86.parse_operands args
		x86.require 80387+
		if x86.@count = 1
			if x86.@1.size & x86.@1.size <> 14
				err 'invalid operand size'
			end if
			if x86.@1.type = 'mem'
				x86.store_operand_prefix 2
				x86.store_instruction 0D9h,x86.@1,postbyte
			else
				err 'invalid operand'
			end if
		else
			err 'invalid number of operands'
		end if
	end macro
end iterate

macro fstenvw args&
	fwait
	fnstenvw dest
end macro

iterate <instr,postbyte>, fldenvd,4, fnstenvd,6
	macro instr? args&
		x86.parse_operands args
		x86.require 80387+
		if x86.@count = 1
			if x86.@1.size & x86.@1.size <> 28
				err 'invalid operand size'
			end if
			if x86.@1.type = 'mem'
				x86.store_operand_prefix 4
				x86.store_instruction 0D9h,x86.@1,postbyte
			else
				err 'invalid operand'
			end if
		else
			err 'invalid number of operands'
		end if
	end macro
end iterate

macro fstenvd args&
	fwait
	fnstenvd dest
end macro

iterate <instr,postbyte>, frstor,4, fnsave,6
	macro instr? args&
		x86.parse_operands args
		x86.require 80387+
		if x86.@count = 1
			if x86.@1.size & ( ( x86.mode = 16 & x86.@1.size <> 94 ) | ( x86.mode = 32 & x86.@1.size <> 108 ) )
				err 'invalid operand size'
			end if
			if x86.@1.type = 'mem'
				x86.store_operand_prefix 2
				x86.store_instruction 0DDh,x86.@1,postbyte
			else
				err 'invalid operand'
			end if
		else
			err 'invalid number of operands'
		end if
	end macro
end iterate

iterate <instr,postbyte>, frstorw,4, fnsavew,6
	macro instr? args&
		x86.parse_operands args
		x86.require 80387+
		if x86.@count = 1
			if x86.@1.size & x86.@1.size <> 94
				err 'invalid operand size'
			end if
			if x86.@1.type = 'mem'
				x86.store_operand_prefix 2
				x86.store_instruction 0DDh,x86.@1,postbyte
			else
				err 'invalid operand'
			end if
		else
			err 'invalid number of operands'
		end if
	end macro
end iterate

macro fsavew args&
	fwait
	fnsavew dest
end macro

iterate <instr,postbyte>, frstord,4, fnsaved,6
	macro instr? args&
		x86.parse_operands args
		x86.require 80387+
		if x86.@count = 1
			if x86.@1.size & x86.@1.size <> 108
				err 'invalid operand size'
			end if
			if x86.@1.type = 'mem'
				x86.store_operand_prefix 4
				x86.store_instruction 0DDh,x86.@1,postbyte
			else
				err 'invalid operand'
			end if
		else
			err 'invalid number of operands'
		end if
	end macro
end iterate

macro fsaved args&
	fwait
	fnsaved dest
end macro
