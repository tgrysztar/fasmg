
; 5.2.1 Load-Store Single Register
; 5.2.3 Load Single Register (pc-relative, literal load)
;  - Scaled, 12-bit, unsigned immediate offset, without pre- and post-index options.
;  - Unscaled, 9-bit, signed immediate offset with pre- or post-index writeback.
;  - Scaled or unscaled 64-bit register offset.
;  - Scaled or unscaled 32-bit extended register offset.
;  - label

macro aarch64.parse_adr_f13_abc a*, b*, c*, size*
	@adr.type = @atype_register
	aarch64.parse_register @opA, a
	aarch64.parse_register @opB, b	      
	if (@opA.type and @type_xn_xsp) & (@opB.type and @type_wn_wzr_xn_xzr)
	    @adr.shift = -1
	    match =lsl d, c
		@adr.kind = 1
		@adr.shift = +d
	    else match =sxtx, c
		@adr.kind = 5
	    else match =uxtw, c
		@adr.kind = 0
	    else match =sxtw, c
		@adr.kind = 4
	    else match =uxtw d, c
		@adr.kind = 0
		@adr.shift = +d
	    else match =sxtw d, c
		@adr.kind = 4
		@adr.shift = +d
	    else match =sxtx d, c
		@adr.kind = 5
		@adr.shift = +d
	    else
		err 'invalid extension in address'
	    end match
	    if @adr.shift = size
		@adr.encode = 0x00205800 + (@adr.kind shl 13) + (@opB.data shl 16) + (@opA.data shl 5)
	    else if @adr.shift = -1
		@adr.encode = 0x00204800 + (@adr.kind shl 13) + (@opB.data shl 16) + (@opA.data shl 5)
	    else
		err 'invalid shift amount'
	    end if
	else
	    err 'invalid address'
	end if
end macro


macro aarch64.parse_adr_f13_ab a*, b*, size*
	aarch64.parse_register @opA, a
	aarch64.parse_operand @opB, b
	if (@opA.type and @type_xn_xsp) & (@opB.type and @type_immc)
	    @adr.type = @atype_simple
	    if (@opB.data and @adr.mask) = 0 & (0 <= @opB.data) & ((@opB.data shr size) <= 0xFFF)
		@adr.encode = 0x01000000 + (((@opB.data shr size) and 0xFFF) shl 10) + (@opA.data shl 5)
	    else
		aarch64.restrict @opB.data, -0x100, 0xFF
		@adr.encode = 0x00000000 + ((@opB.data and 0x1FF) shl 12) + (@opA.data shl 5)
	    end if
	else if (@opA.type and @type_xn_xsp) & (@opB.type and @type_xn_xzr)
	    @adr.type = @atype_register
	    @adr.encode = 0x00206800 + (@opB.data shl 16) + (@opA.data shl 5)
	else if (@opA.type and @type_xn_xsp) & (@opB.type and @type_immc_lo12)
	    @adr.type = @atype_simple
	    @opB.data = @opB.data and 0xFFF
	    aarch64.restrict @opB.data, 0, 0xFFF, @adr.mask
	    @adr.encode = 0x01000000 + ((@opB.data shr size) shl 10) + (@opA.data shl 5)
	else
	    err 'invalid address'
	end if
end macro



macro aarch64.parse_adr_f13 lit, size, x&
    @adr.mask = (1 shl size) - 1
    match [a=,b=,c], x
	aarch64.parse_adr_f13_abc a, b, c, size
    else match [a=,b], x
	aarch64.parse_adr_f13_ab a, b, size
    else match [a=,b]!, x
	@adr.type = @atype_preincr
	aarch64.parse_register @opA, a
	aarch64.parse_operand @opB, b
	if (@opA.type and @type_xn_xsp) & (@opB.type and @type_immc)
	    aarch64.restrict @opB.data, -256, 255
	    @adr.encode = 0x00000c00 + ((@opB.data and 0x1FF) shl 12) + (@opA.data shl 5)
	else
	    err 'invalid address'
	end if
    else match [a]=,b, x
	@adr.type = @atype_postincr
	aarch64.parse_register @opA, a
	aarch64.parse_operand @opB, b
	if (@opA.type and @type_xn_xsp) & (@opB.type and @type_immc)
	    aarch64.restrict @opB.data, -256, 255
	    @adr.encode = 0x00000400 + ((@opB.data and 0x1FF) shl 12) + (@opA.data shl 5)
	else
	    err 'invalid address'
	end if
    else match [a], x
	@adr.type = @atype_simple
	aarch64.parse_register @opA, a
	if (@opA.type and @type_xn_xsp)
	    @adr.encode = 0x01000000 + (@opA.data shl 5)
	else
	    err 'invalid address'
	end if
    else
	@adr.type = @atype_literal
	@adr.imm = +x
	if lit = -1
	    err 'literal address not allowed'
	else if @adr.imm relativeto $
	    @adr.imm = @adr.imm - $
	    aarch64.restrict @adr.imm, -0x100000, 0xFFFFF, 0x03
	    @adr.encode = lit + (((@adr.imm shr 2) and 0x7FFFF) shl 5)
	else
	    err 'invalid literal'
	end if
    end match
end macro

macro ldrb a*, b&
    aarch64.parse_operand @op0, a
    if (@op0.type and @type_wn_wzr)
	aarch64.parse_adr_f13 -1, 0, b
	dd 0x38400000 + @adr.encode + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro ldrh a*, b&
    aarch64.parse_operand @op0, a
    aarch64.parse_adr_f13 -1, 1, b
    if (@op0.type and @type_wn_wzr)
	dd 0x78400000 + @adr.encode + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro ldrsb a*, b&
    aarch64.parse_operand @op0, a
    aarch64.parse_adr_f13 -1, 0, b
    if (@op0.type and @type_wn_wzr)
	dd 0x38c00000 + @adr.encode + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xzr)
	dd 0x38800000 + @adr.encode + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro ldrsh a*, b&
    aarch64.parse_operand @op0, a
    aarch64.parse_adr_f13 -1, 1, b
    if (@op0.type and @type_wn_wzr)
	dd 0x78c00000 + @adr.encode + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xzr)
	dd 0x78800000 + @adr.encode + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro




macro str a*, b&
    aarch64.parse_operand @op0, a
    if @op0.type and @type_xn_xzr
	aarch64.parse_adr_f13 -1, 3, b
	dd 0xf8000000 + @adr.encode + (@op0.data shl 0)
    else if @op0.type and @type_wn_wzr
	aarch64.parse_adr_f13 -1, 2, b
	dd 0xb8000000 + @adr.encode + (@op0.data shl 0)
    else if @op0.type and @type_dn
	aarch64.parse_adr_f13 -1, 3, b
	dd 0xfc000000 + @adr.encode + (@op0.data shl 0)
    else if @op0.type and @type_qn
	aarch64.parse_adr_f13 -1, 4, b
	dd 0x3c800000 + @adr.encode + (@op0.data shl 0)
    else if @op0.type and @type_sn
	aarch64.parse_adr_f13 -1, 2, b
	dd 0xbc000000 + @adr.encode + (@op0.data shl 0)
    else if @op0.type and @type_bn
	aarch64.parse_adr_f13 -1, 0, b
	dd 0x3c000000 + @adr.encode + (@op0.data shl 0)
    else if @op0.type and @type_hn
	aarch64.parse_adr_f13 -1, 1, b
	dd 0x7c000000 + @adr.encode + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro



macro strb a*, b&
    aarch64.parse_operand @op0, a
    if (@op0.type and @type_wn_wzr)
	aarch64.parse_adr_f13 -1, 0, b
	dd 0x38000000 + @adr.encode + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro strh a*, b&
    aarch64.parse_operand @op0, a
    if (@op0.type and @type_wn_wzr)
	aarch64.parse_adr_f13 -1, 1, b
	dd 0x78000000 + @adr.encode + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro ldr a*, b&
    aarch64.parse_operand @op0, a
    if @op0.type and @type_xn_xzr
	aarch64.parse_adr_f13 -0xa0400000, 3, b
	dd 0xf8400000 + @adr.encode + (@op0.data shl 0)
    else if @op0.type and @type_wn_wzr
	aarch64.parse_adr_f13 -0xa0400000, 2, b
	dd 0xb8400000 + @adr.encode + (@op0.data shl 0)
    else if @op0.type and @type_dn
	aarch64.parse_adr_f13 -0xa0400000, 3, b
	dd 0xfc400000 + @adr.encode + (@op0.data shl 0)
    else if @op0.type and @type_qn
	aarch64.parse_adr_f13 -0xa0c00000 , 4, b
	dd 0x3cc00000 + @adr.encode + (@op0.data shl 0)
    else if @op0.type and @type_sn
	aarch64.parse_adr_f13 -0xa0400000 , 2, b
	dd 0xbc400000 + @adr.encode + (@op0.data shl 0)
    else if @op0.type and @type_bn
	aarch64.parse_adr_f13 -1, 0, b
	dd 0x3c400000 + @adr.encode + (@op0.data shl 0)
    else if @op0.type and @type_hn
	aarch64.parse_adr_f13 -1, 1, b
	dd 0x7c400000 + @adr.encode + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro prfm a*, b&
    match =PLDL1KEEP?, a
	@imm.data = 0
    else match =PLDL1STRM?, a
	@imm.data = 1
    else match =PLDL2KEEP?, a
	@imm.data = 2
    else match =PLDL2STRM?, a
	@imm.data = 3
    else match =PLDL3KEEP?, a
	@imm.data = 4
    else match =PLDL3STRM?, a
	@imm.data = 5
    else match =PSTL1KEEP?, a
	@imm.data = 16
    else match =PSTL1STRM?, a
	@imm.data = 17
    else match =PSTL2KEEP?, a
	@imm.data = 18
    else match =PSTL2STRM?, a
	@imm.data = 19
    else match =PSTL3KEEP?, a
	@imm.data = 20
    else match =PSTL3STRM?, a
	@imm.data = 21
    else
	@imm.data = +a
	if @imm.data relativeto 0
	    aarch64.restrict @imm.data, 0, 31
	else
	    err 'invalid prefetch hint'
	end if
    end match
    aarch64.parse_adr_f13 -0x20800000, 3, b
    if @adr.type and @atype_incr = 0
	dd 0xf8800000 + @adr.encode + @imm.data
    else
	err 'invalid post/pre increment'
    end if
end macro

macro ldrsw a*, b&
    aarch64.parse_operand @op0, a
    aarch64.parse_adr_f13 -0x20800000, 2, b
    if (@op0.type and @type_xn_xzr)
	dd 0xb8800000 + @adr.encode + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro



; simple address parse with pre and post index
macro aarch64.parse_adr x&
    match [a=,b], x
	aarch64.parse_operand @opA, a
	aarch64.parse_operand @opB, b
	if (@opA.type and @type_xn_xsp) & (@opB.type and @type_immc)
	    @adr.type = @atype_simple
	else
	    err 'invalid address'
	end if
    else match [a], x
	aarch64.parse_operand @opA, a
	if (@opA.type and @type_xn_xsp)
	    @adr.type = @atype_simple
	    @opB.data = 0
	else
	    err 'invalid address'
	end if
    else match [a=,b]!, x
	aarch64.parse_operand @opA, a
	aarch64.parse_operand @opB, b
	if (@opA.type and @type_xn_xsp) & (@opB.type and @type_immc)
	    @adr.type = @atype_preincr
	else
	    err 'invalid address'
	end if
    else match [a]=,b, x
	aarch64.parse_operand @opA, a
	aarch64.parse_operand @opB, b
	if (@opA.type and @type_xn_xsp) & (@opB.type and @type_immc)
	    @adr.type = @atype_postincr
	else
	    err 'invalid address'
	end if
    else
	err 'invalid address'
    end match
end macro

; 5.2.2 Load-Store Single Register (unscaled offset)
; 5.2.6 Load-Store Unprivileged
;  - Unscaled, 9-bit, signed immediate offset, without pre- and post-index options
macro aarch64.parse_adr_f26 x&
    aarch64.parse_adr x
    if @adr.type = @atype_simple
	aarch64.restrict @opB.data, -0x100, 0xFF
	@adr.encode = 0x00000000 + ((@opB.data and 0x1FF) shl 12) + (@opA.data shl 5)
    else
	err 'invalid address'
    end if
end macro

iterate <instr, code>, ldurb, 0x38400000,\
		       ldurh, 0x78400000,\
		       sturb, 0x38000000,\
		       sturh, 0x78000000,\
		       ldtrb, 0x38400800,\
		       ldtrh, 0x78400800,\
		       sttrb, 0x38000800,\
		       sttrh, 0x78000800
    macro instr a*, b&
	aarch64.parse_adr_f26	    b
	aarch64.parse_operand @op0, a
	if (@op0.type and @type_wn_wzr)
	    dd code + @adr.encode + (@op0.data shl 0)
	else
	    err 'invalid operand'
	end if
    end macro
end iterate

iterate <instr, code>, ldursw, 0xb8800000,\
		       ldtrsw, 0xb8800800
    macro instr a*, b&
	aarch64.parse_adr_f26	    b
	aarch64.parse_operand @op0, a
	if (@op0.type and @type_xn_xzr)
	    dd code + @adr.encode + (@op0.data shl 0)
	else
	    err 'invalid operand'
	end if
    end macro
end iterate

iterate <instr, code1, code2>, ldursb, 0x38c00000, 0x38800000,\
			       ldursh, 0x78c00000, 0x78800000,\
			       sttr,   0xb8000800, 0xf8000800,\
			       ldtr,   0xb8400800, 0xf8400800,\
			       ldtrsb, 0x38c00800, 0x00000000,\
			       ldtrsh, 0x78c00800, 0x00000000
    macro instr a*, b&
	aarch64.parse_adr_f26	    b
	aarch64.parse_operand @op0, a
	if (@op0.type and @type_wn_wzr)
	    dd code1 + @adr.encode + (@op0.data shl 0)
	else if (@op0.type and @type_xn_xzr)
	    dd code2 + @adr.encode + (@op0.data shl 0)
	else
	    err 'invalid operand'
	end if
    end macro
end iterate				 

iterate <instr, code1, code2, code3, code4, code5, code6, code7>,\
	 stur, 0xb8000000, 0xf8000000, 0x3c800000, 0xfc000000, 0xbc000000, 0x7c000000, 0x3c000000,\
	 ldur, 0xb8400000, 0xf8400000, 0x3cc00000, 0xfc400000, 0xbc400000, 0x7c400000, 0x3c400000
    macro instr a*, b&
	aarch64.parse_adr_f26	    b
	aarch64.parse_operand @op0, a
	if (@op0.type and @type_wn_wzr)
	    dd code1 + @adr.encode + (@op0.data shl 0)
	else if (@op0.type and @type_xn_xzr)
	    dd code2 + @adr.encode + (@op0.data shl 0)
	else if (@op0.type and @type_qn)
	    dd code3 + @adr.encode + (@op0.data shl 0)
	else if (@op0.type and @type_dn)
	    dd code4 + @adr.encode + (@op0.data shl 0)
	else if (@op0.type and @type_sn)
	    dd code5 + @adr.encode + (@op0.data shl 0)
	else if (@op0.type and @type_hn)
	    dd code6 + @adr.encode + (@op0.data shl 0)
	else if (@op0.type and @type_bn)
	    dd code7 + @adr.encode + (@op0.data shl 0)
	else
	    err 'invalid operand'
	end if
    end macro
end iterate				 


; 5.2.4 Load-Store Pair
;  - Scaled 7-bit signed immediate offset, with pre- and post-index writeback options
macro aarch64.parse_adr_f4 size, x&
    @adr.mask = (1 shl size) - 1
    aarch64.parse_adr x
    aarch64.restrict @opB.data, -0x200, 0x1FF, @adr.mask
    if @adr.type = @atype_simple
	@adr.encode = 0x01000000 + (((@opB.data shr size) and 0x7F) shl 15) + (@opA.data shl 5)
    else if @adr.type = @atype_postincr
	@adr.encode = 0x00800000 + (((@opB.data shr size) and 0x7F) shl 15) + (@opA.data shl 5)
    else if @adr.type = @atype_preincr
	@adr.encode = 0x01800000 + (((@opB.data shr size) and 0x7F) shl 15) + (@opA.data shl 5)
    else
	err 'internal error'
    end if
end macro

macro ldpsw a*, b*, c&
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr)
	aarch64.parse_adr_f4 2, c
	dd 0x68400000 + @adr.encode + (@op1.data shl 10) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

iterate <instr, code1, code2, code3, code4, code5>, ldp, 0xa8400000, 0x6c400000, 0x28400000, 0x2c400000, 0xac400000,\
						    stp, 0xa8000000, 0x6c000000, 0x28000000, 0x2c000000, 0xac000000
    macro instr a*, b*, c&
	aarch64.parse_operand @op0, a
	aarch64.parse_operand @op1, b
	if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr)
	    aarch64.parse_adr_f4 3, c
	    dd code1 + @adr.encode + (@op1.data shl 10) + (@op0.data shl 0)
	else if (@op0.type and @type_dn) & (@op1.type and @type_dn)
	    aarch64.parse_adr_f4 3, c
	    dd code2 + @adr.encode + (@op1.data shl 10) + (@op0.data shl 0)
	else if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr)
; FIXME: this case seems to be broken
	    aarch64.parse_adr_f4 2, c
	    dd code3 + @adr.encode + (@op1.data shl 10) + (@op0.data shl 0)
	else if (@op0.type and @type_sn) & (@op1.type and @type_sn)
	    aarch64.parse_adr_f4 2, c
	    dd code4 + @adr.encode + (@op1.data shl 10) + (@op0.data shl 0)
	else if (@op0.type and @type_qn) & (@op1.type and @type_qn)
	    aarch64.parse_adr_f4 4, c
	    dd code5 + @adr.encode + (@op1.data shl 10) + (@op0.data shl 0)
	else
	    err 'invalid operand'
	end if
    end macro
end iterate




; 5.2.5 Load-Store Non-temporal Pair
;  - Scaled 7-bit signed immediate offset, without pre- and post-index options
macro aarch64.parse_adr_f5 size, x&
    aarch64.parse_adr x
    if (@adr.type = @atype_simple)
	aarch64.restrict @opB.data, (-256 shl size), (255 shl size), ((1 shl size) - 1)
	@adr.encode = 0x00000000 + (((@opB.data and 0x3FF) shr size) shl 15) + (@opA.data shl 5)
    else
	err 'invalid address'
    end if
end macro

iterate <instr, code1, code2, code3, code4, code5>, ldnp, 0x28400000, 0xa8400000, 0x2c400000, 0x6c400000, 0xac400000,\
						    stnp, 0x28000000, 0xa8000000, 0x2c000000, 0x6c000000, 0xac000000
    macro instr a*, b*, c&
	aarch64.parse_operand @op0, a
	aarch64.parse_operand @op1, b
	if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr)
	    aarch64.parse_adr_f5 2, c
	    dd code1 + @adr.encode + (@op1.data shl 10) + (@op0.data shl 0)
	else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr)
	    aarch64.parse_adr_f5 3, c
	    dd code2 + @adr.encode + (@op1.data shl 10) + (@op0.data shl 0)
	else if (@op0.type and @type_sn) & (@op1.type and @type_sn)
	    aarch64.parse_adr_f5 2, c
	    dd code3 + @adr.encode + (@op1.data shl 10) + (@op0.data shl 0)
	else if (@op0.type and @type_dn) & (@op1.type and @type_dn)
	    aarch64.parse_adr_f5 3, c
	    dd code4 + @adr.encode + (@op1.data shl 10) + (@op0.data shl 0)
	else if (@op0.type and @type_qn) & (@op1.type and @type_qn)
	    aarch64.parse_adr_f5 4, c
	    dd code5 + @adr.encode + (@op1.data shl 10) + (@op0.data shl 0)
	else
	    err 'invalid operand'
	end if		  
    end macro
end iterate

; 5.2.7 Load-Store Exclusive
; 5.2.8 Load-Acquire / Store-Release
macro aarch64.parse_adr_f78 x&
    aarch64.parse_adr x
    if (@adr.type = @atype_simple) & (@opB.data = 0)
	@adr.encode = @opA.data shl 5
    else
	err 'invalid address'
    end if
end macro

iterate <instr, code>, ldarb,  0x08dffc00,\
		       ldaxrb, 0x085ffc00,\
		       ldxrb,  0x085f7c00,\
		       stlrb,  0x089ffc00,\
		       ldarh,  0x48dffc00,\
		       ldxrh,  0x485f7c00,\
		       ldaxrh, 0x485ffc00,\
		       stlrh,  0x489ffc00
    macro instr a*, b&
	aarch64.parse_adr_f78	    b
	aarch64.parse_operand @op0, a
	if (@op0.type and @type_wn_wzr)
	    dd code + @adr.encode + (@op0.data shl 0)
	else
	    err 'invalid operand'
	end if
    end macro
end iterate

iterate <instr, code1, code2>, ldar,  0x88dffc00, 0xc8dffc00,\
			       ldaxr, 0x885ffc00, 0xc85ffc00,\
			       ldxr,  0x885f7c00, 0xc85f7c00,\
			       stlr,  0x889ffc00, 0xc89ffc00
    macro instr a*, b&
	aarch64.parse_adr_f78	    b
	aarch64.parse_operand @op0, a
	if (@op0.type and @type_wn_wzr)
	    dd code1 + @adr.encode + (@op0.data shl 0)
	else if (@op0.type and @type_xn_xzr)
	    dd code2 + @adr.encode + (@op0.data shl 0)
	else
	    err 'invalid operand'
	end if
    end macro
end iterate

iterate <instr, code>, stlxrb, 0x0800fc00,\
		       stxrb,  0x08007c00,\
		       stlxrh, 0x4800fc00,\
		       stxrh,  0x48007c00
    macro instr a*, b*, c&
	aarch64.parse_adr_f78	    c
	aarch64.parse_operand @op0, a
	aarch64.parse_operand @op1, b
	if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr)
	    dd code + @adr.encode + (@op0.data shl 16) + (@op1.data shl 0)
	else
	    err 'invalid operand'
	end if
    end macro
end iterate

iterate <instr, code1, code2>, stxr,  0x88007c00, 0xc8007c00,\
			       stlxr, 0x8800fc00, 0xc800fc00
    macro instr a*, b*, c&
	aarch64.parse_adr_f78	    c
	aarch64.parse_operand @op0, a
	aarch64.parse_operand @op1, b
	if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr)
	    dd code1 + @adr.encode + (@op0.data shl 16) + (@op1.data shl 0)
	else if (@op0.type and @type_wn_wzr) & (@op1.type and @type_xn_xzr)
	    dd code2 + @adr.encode + (@op0.data shl 16) + (@op1.data shl 0)
	else
	    err 'invalid operand'
	end if
    end macro
end iterate

iterate <instr, code1, code2>, ldxp,  0x887f0000, 0xc87f0000,\
			       ldaxp, 0x887f8000, 0xc87f8000
    macro instr a*, b*, c&
	aarch64.parse_adr_f78	    c
	aarch64.parse_operand @op0, a
	aarch64.parse_operand @op1, b
	if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr)
	    dd code1 + @adr.encode + (@op1.data shl 10) + (@op0.data shl 0)
	else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr)
	    dd code2 + @adr.encode + (@op1.data shl 10) + (@op0.data shl 0)
	else
	    err 'invalid operand'
	end if
    end macro
end iterate

iterate <instr, code1, code2>, stxp,  0x88200000, 0xc8200000,\
			       stlxp, 0x88208000, 0xc8208000
    macro instr a*, b*, c*, d&
	aarch64.parse_adr_f78	    d
	aarch64.parse_operand @op0, a
	aarch64.parse_operand @op1, b
	aarch64.parse_operand @op2, c
	if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr)
	    dd code1 + @adr.encode + (@op0.data shl 16) + (@op2.data shl 10) + (@op1.data shl 0)
	else if (@op0.type and @type_wn_wzr) & (@op1.type and @type_xn_xzr)
	    dd code2 + @adr.encode + (@op0.data shl 16) + (@op2.data shl 10) + (@op1.data shl 0)
	else
	    err 'invalid operand'
	end if
    end macro
end iterate




macro aarch64.parse_vstr3 x&
    match =[a=], x
	aarch64.parse_operand @op0, a
	if (@op0.type and @type_xn_xsp)
	    @adr.encode = @adr.encode + (@op0.data shl 5)
	else
	    err 'invalid address A'
	end if
    else match =[a=]=,b , x
	aarch64.parse_operand @op0, a
	aarch64.parse_operand @op1, b
	if (@op0.type and @type_xn_xsp) & (@op1.type and @type_xn)
	    @adr.encode = 0x00800000 + @adr.encode + (@op1.data shl 16) + (@op0.data shl 5)
	else if (@op0.type and @type_xn_xsp) & (@op1.type and @type_immc)
	    @adr.encode = 0x00800000 + @adr.encode + (	     31 shl 16) + (@op0.data shl 5)
	else
	    err 'invalid address C'
	end if
    else
	err 'invalid address B'
    end match
end macro

macro aarch64.parse_vstr2 e&
    @adr.encode = @adr.encode + (@op0.data shl 0)
    if @adr.expectindex
	match =[i=]=,a , e
	    @adr.number = +i
	    if @adr.type and @type_vn_b
		@adr.encode = @adr.encode + 0x00000000
	    else if @adr.type and @type_vn_h
		@adr.encode = @adr.encode + 0x00004000
		@adr.number = @adr.number shl 1
	    else if @adr.type and @type_vn_s
		@adr.encode = @adr.encode + 0x00008000
		@adr.number = @adr.number shl 2
	    else if @adr.type and @type_vn_d
		@adr.encode = @adr.encode + 0x00008400
		@adr.number = @adr.number shl 3
	    else
		err 'internal error'
	    end if
	    @adr.encode = @adr.encode + ((@adr.number and 8) shl 27) + ((@adr.number and 7) shl 10)
	    aarch64.parse_vstr3 a
	else
	    err 'no index found'
	end match
    else
	@adr.number = (bsf @adr.type) - (bsf @type_vn_8b)
	@adr.encode = @adr.encode + ((@adr.number and 4) shl 28) + ((@adr.number and 3) shl 10)
	match =,a , e
	    aarch64.parse_vstr3 a
	else
	    err 'no index found'
	end match
    end if
end macro

macro aarch64.parse_vstr x&
    match ={a=,b=,c=,d=}e , x
	@adr.length = 4
	aarch64.parse_operand @op0, a
	aarch64.parse_operand @op1, b
	aarch64.parse_operand @op2, c
	aarch64.parse_operand @op3, d
	if @op0.type <> @op1.type | @op0.type <> @op2.type | @op0.type <> @op3.type
	    err 'vector type mismatch'
	else if (@op0.type and @type_vn_16b_etc)
	    @adr.expectindex = 0
	    @adr.encode = @adr.encode4
	else if (@op0.type and @type_vn_b_etc)
	    @adr.expectindex = 1
	    @adr.encode = @adr.encode4i
	else
	    err 'arguments are not vectors'
	end if
	@adr.type = @op0.type
	if (((@op1.data-@op0.data) and 31) <> 1) | (((@op2.data-@op0.data) and 31) <> 2) | (((@op3.data-@op0.data) and 31) <> 3)
	    err 'arguments are not consecutive'
	end if
	aarch64.parse_vstr2 e
    else match ={a=,b=,c=}e , x
	@adr.length = 3
	aarch64.parse_operand @op0, a
	aarch64.parse_operand @op1, b
	aarch64.parse_operand @op2, c
	if @op0.type <> @op1.type | @op0.type <> @op2.type
	    err 'vector type mismatch'
	else if (@op0.type and @type_vn_16b_etc)
	    @adr.expectindex = 0
	    @adr.encode = @adr.encode3
	else if (@op0.type and @type_vn_b_etc)
	    @adr.expectindex = 1
	    @adr.encode = @adr.encode3i
	else
	    err 'arguments are not vectors'
	end if
	@adr.type = @op0.type
	if (((@op1.data-@op0.data) and 31) <> 1) | (((@op2.data-@op0.data) and 31) <> 2)
	    err 'arguments are not consecutive'
	end if
	aarch64.parse_vstr2 e
    else match ={a=,b=}e , x
	@adr.length = 2
	aarch64.parse_operand @op0, a
	aarch64.parse_operand @op1, b
	if @op0.type <> @op1.type
	    err 'vector type mismatch'
	else if (@op0.type and @type_vn_16b_etc)
	    @adr.expectindex = 0
	    @adr.encode = @adr.encode2
	else if (@op0.type and @type_vn_b_etc)
	    @adr.expectindex = 1
	    @adr.encode = @adr.encode2i
	else
	    err 'arguments are not vectors'
	end if
	if (((@op1.data-@op0.data) and 31) <> 1)
	    err 'arguments are not consecutive'
	end if
	@adr.type = @op0.type
	aarch64.parse_vstr2 e
    else match ={a=}e , x
	@adr.length = 1
	aarch64.parse_operand @op0, a
	if (@op0.type and @type_vn_16b_etc)
	    @adr.expectindex = 0
	    @adr.encode = @adr.encode1
	else if (@op0.type and @type_vn_b_etc)
	    @adr.expectindex = 1
	    @adr.encode = @adr.encode1i
	else
	    err 'arguments are not vectors'
	end if
	@adr.type = @op0.type
	aarch64.parse_vstr2 e
    else
	err 'invalid structure'
    end match	     
end macro

macro st1 a&
    @adr.encode1  = 0x0c007000
    @adr.encode2  = 0x0c00a000
    @adr.encode3  = 0x0c006000
    @adr.encode4  = 0x0c002000
    @adr.encode1i = 0x0d000000
    @adr.encode2i = -1
    @adr.encode3i = -1
    @adr.encode4i = -1
    aarch64.parse_vstr a
    dd @adr.encode
end macro

macro st2 a&
    @adr.encode1  = -1
    @adr.encode2  = 0x0c008000
    @adr.encode3  = -1
    @adr.encode4  = -1
    @adr.encode1i = -1
    @adr.encode2i = 0x0d200000
    @adr.encode3i = -1
    @adr.encode4i = -1
    aarch64.parse_vstr a
    dd @adr.encode
end macro

macro st3 a&
    @adr.encode1  = -1
    @adr.encode2  = -1
    @adr.encode3  = 0x0c004000
    @adr.encode4  = -1
    @adr.encode1i = -1
    @adr.encode2i = -1
    @adr.encode3i = 0x0d002000
    @adr.encode4i = -1
    aarch64.parse_vstr a
    dd @adr.encode
end macro

macro st4 a&
    @adr.encode1  = -1
    @adr.encode2  = -1
    @adr.encode3  = -1
    @adr.encode4  = 0x0c000000
    @adr.encode1i = -1
    @adr.encode2i = -1
    @adr.encode3i = -1
    @adr.encode4i = 0x0d202000
    aarch64.parse_vstr a
    dd @adr.encode
end macro


macro ld1 a&
    @adr.encode1  = 0x0c407000
    @adr.encode2  = 0x0c40a000
    @adr.encode3  = 0x0c406000
    @adr.encode4  = 0x0c402000
    @adr.encode1i = 0x0d400000
    @adr.encode2i = -1
    @adr.encode3i = -1
    @adr.encode4i = -1
    aarch64.parse_vstr a
    dd @adr.encode
end macro

macro ld2 a&
    @adr.encode1  = -1
    @adr.encode2  = 0x0c408000
    @adr.encode3  = -1
    @adr.encode4  = -1
    @adr.encode1i = -1
    @adr.encode2i = 0x0d600000
    @adr.encode3i = -1
    @adr.encode4i = -1
    aarch64.parse_vstr a
    dd @adr.encode
end macro

macro ld3 a&
    @adr.encode1  = -1
    @adr.encode2  = -1
    @adr.encode3  = 0x0c404000
    @adr.encode4  = -1
    @adr.encode1i = -1
    @adr.encode2i = -1
    @adr.encode3i = 0x0d402000
    @adr.encode4i = -1
    aarch64.parse_vstr a
    dd @adr.encode
end macro

macro ld4 a&
    @adr.encode1  = -1
    @adr.encode2  = -1
    @adr.encode3  = -1
    @adr.encode4  = 0x0c400000
    @adr.encode1i = -1
    @adr.encode2i = -1
    @adr.encode3i = -1
    @adr.encode4i = 0x0d602000
    aarch64.parse_vstr a
    dd @adr.encode
end macro

macro ld1r a&
    @adr.encode1  = 0x0d40c000
    @adr.encode2  = -1
    @adr.encode3  = -1
    @adr.encode4  = -1
    @adr.encode1i = -1
    @adr.encode2i = -1
    @adr.encode3i = -1
    @adr.encode4i = -1
    aarch64.parse_vstr a
    dd @adr.encode
end macro

macro ld2r a&
    @adr.encode1  = -1
    @adr.encode2  = 0x0d60c000
    @adr.encode3  = -1
    @adr.encode4  = -1
    @adr.encode1i = -1
    @adr.encode2i = -1
    @adr.encode3i = -1
    @adr.encode4i = -1
    aarch64.parse_vstr a
    dd @adr.encode
end macro

macro ld3r a&
    @adr.encode1  = -1
    @adr.encode2  = -1
    @adr.encode3  = 0x0d40e000
    @adr.encode4  = -1
    @adr.encode1i = -1
    @adr.encode2i = -1
    @adr.encode3i = -1
    @adr.encode4i = -1
    aarch64.parse_vstr a
    dd @adr.encode
end macro

macro ld4r a&
    @adr.encode1  = -1
    @adr.encode2  = -1
    @adr.encode3  = -1
    @adr.encode4  = 0x0d60e000
    @adr.encode1i = -1
    @adr.encode2i = -1
    @adr.encode3i = -1
    @adr.encode4i = -1
    aarch64.parse_vstr a
    dd @adr.encode
end macro



macro aarch64.parse_vtb x&
    match f=,={a=,b=,c=,d=}=,g , x
	aarch64.parse_operand @op0, a
	aarch64.parse_operand @op1, b
	aarch64.parse_operand @op2, c
	aarch64.parse_operand @op3, d
	if @op0.type <> @op1.type | @op0.type <> @op2.type | @op0.type <> @op3.type
	    err 'vector type mismatch'
	else if @op0.type <> @type_vn_16b
	    err 'invalid vector types'
	else if (((@op1.data-@op0.data) and 31) <> 1) | (((@op2.data-@op0.data) and 31) <> 2) | (((@op3.data-@op0.data) and 31) <> 3)
	    err 'arguments are not consecutive'
	end if
	@adr.length = 3
	@adr.type = @op0.type
	@adr.start = @op0.data
	aarch64.parse_operand @op0, f
	aarch64.parse_operand @op1, g
    else match f=,={a=,b=,c=}=,g , x
	aarch64.parse_operand @op0, a
	aarch64.parse_operand @op1, b
	aarch64.parse_operand @op2, c
	if @op0.type <> @op1.type | @op0.type <> @op2.type
	    err 'vector type mismatch'
	else if @op0.type <> @type_vn_16b
	    err 'invalid vector types'
	else if (((@op1.data-@op0.data) and 31) <> 1) | (((@op2.data-@op0.data) and 31) <> 2)
	    err 'arguments are not consecutive'
	end if
	@adr.length = 2
	@adr.type = @op0.type
	@adr.start = @op0.data
	aarch64.parse_operand @op0, f
	aarch64.parse_operand @op1, g
    else match f=,={a=,b=}=,g , x
	aarch64.parse_operand @op0, a
	aarch64.parse_operand @op1, b
	if @op0.type <> @op1.type
	    err 'vector type mismatch'
	else if @op0.type <> @type_vn_16b
	    err 'invalid vector types'
	else if ((@op1.data-@op0.data) and 31) <> 1
	    err 'arguments are not consecutive'
	end if
	@adr.length = 1
	@adr.type = @op0.type
	@adr.start = @op0.data
	aarch64.parse_operand @op0, f
	aarch64.parse_operand @op1, g
    else match f=,={a=}=,g , x
	aarch64.parse_operand @op0, a
	if @op0.type <> @type_vn_16b
	    err 'invalid vector types'
	end if
	@adr.length = 0
	@adr.type = @op0.type
	@adr.start = @op0.data
	aarch64.parse_operand @op0, f
	aarch64.parse_operand @op1, g
    else
	err 'invalid operand'
    end match	
    if @op0.type <> @op1.type
	err 'invalid operand'
    end if
    if @op0.type = @type_vn_8b
	@adr.encode = 0x00000000 
    else if @op0.type = @type_vn_16b
	@adr.encode = 0x40000000 
    else
	err 'invalid operand'
    end if
    @adr.encode = @adr.encode + (@op1.data shl 16) +  (@adr.length shl 13) + (@adr.start shl 5) + (@op0.data shl 0)
end macro

macro tbl x&
    aarch64.parse_vtb x
    dd 0x0e000000 + @adr.encode
end macro

macro tbx x&
    aarch64.parse_vtb x
    dd 0x0e001000 + @adr.encode
end macro
