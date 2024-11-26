;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; encoding immediates
;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; determine if x can be encoded as a wide immediate
macro aarch64.parse_uimm64 x
    assert x relativeto 0
    @imm.number = x and 0xFFFFFFFFFFFFFFFF
    @imm.success = 1
    @imm.shift = 0
    @imm.inv = 0
    if @imm.number < 0x10000
	@imm.shift = 0
    else if (@imm.number and 0xFFFF) = 0 & (@imm.number shr 16) < 0x10000
	@imm.shift = 1
    else if (@imm.number and 0xFFFFFFFF) = 0 & (@imm.number shr 32) < 0x10000
	@imm.shift = 2
    else if (@imm.number and 0xFFFFFFFFFFFF) = 0 & (@imm.number shr 48) < 0x10000
	@imm.shift = 3
    else
	@imm.number = @imm.number xor 0xFFFFFFFFFFFFFFFF
	@imm.inv = 1
	if @imm.number < 0x10000
	    @imm.shift = 0
	else if (@imm.number and 0xFFFF) = 0 & (@imm.number shr 16) < 0x10000
	    @imm.shift = 1
	else if (@imm.number and 0xFFFFFFFF) = 0 & (@imm.number shr 32) < 0x10000
	    @imm.shift = 2
	else if (@imm.number and 0xFFFFFFFFFFFF) = 0 & (@imm.number shr 48) < 0x10000
	    @imm.shift = 3
	else
	    @imm.success = 0
	end if
    end if
    if x > 0xFFFFFFFFFFFFFFFF
	@imm.success = 0
    else if x < -0x8000000000000000
	@imm.success = 0
    end if
    @imm.imm16 = @imm.number shr (16*@imm.shift)
end macro

macro aarch64.parse_uimm32 x
    assert x relativeto 0
    @imm.number = +x and 0xFFFFFFFF
    @imm.success = 1
    @imm.shift = 0
    @imm.inv = 0
    if @imm.number < 0x10000
	@imm.shift = 0
    else if (@imm.number and 0xFFFF) = 0 & (@imm.number shr 16) < 0x10000
	@imm.shift = 1
    else
	@imm.number = @imm.number xor 0xFFFFFFFF
	@imm.inv = 1
	if @imm.number < 0x10000
	    @imm.shift = 0
	else if (@imm.number and 0xFFFF) = 0 & (@imm.number shr 16) < 0x10000
	    @imm.shift = 1
	else
	    @imm.success = 0
	end if
    end if
    if x > 0xFFFFFFFF
	@imm.success = 0
    else if x < -0x80000000
	@imm.success = 0
    end if
    @imm.imm16 = @imm.number shr (16*@imm.shift)
end macro

; determine if x can be encoded as a logical immediate
macro aarch64.parse_bimm64 x, ec:0
    local value, negate, a, b, c, d, clz_a, clz_b, clz_c
    assert x relativeto 0
    value = x and 0xFFFFFFFFFFFFFFFF
    negate = 0
    @imm.success = 0
    if value and 1 = 1
	negate = 1
	value = value xor 0xFFFFFFFFFFFFFFFF
    end if
    a = value and -value
    b = (value + a) and -(value + a)
    c = (value + a - b) and -(value + a - b)
    if c <> 0 | a <> 0
	if a = 0
	    clz_a = 64
	else 
	    clz_a = 63 - bsr a	  
	end if
	if c <> 0
	    clz_c = 63 - bsr c
	    d = clz_a - clz_c
	    @imm.n = 0
	else
	    d = 64
	    @imm.n = 1
	end if
	mask = 1 shl d - 1
	mult = 0
	if d = 2
	    mult = 0x5555555555555555
	else if d = 4
	    mult = 0x1111111111111111
	else if d = 8
	    mult = 0x0101010101010101
	else if d = 16
	    mult = 0x0001000100010001
	else if d = 32
	    mult = 0x0000000100000001
	else if d = 64
	    mult = 0x0000000000000001
	end if
	if mult <> 0 
	  if (b - a) and not mask = 0
	    if value = (b - a)*mult
		if b = 0
		    clz_b = -1
		else
		    clz_b = 63 - bsr b
		end if
		@imm.s = clz_a - clz_b
		if negate
		    @imm.s = d - @imm.s
		    @imm.r = (clz_b + 1) and (d - 1)
		else
		    @imm.r = (clz_a + 1) and (d - 1)
		end if
		@imm.success = 1
		@imm.s = ((-d shl 1) or (@imm.s - 1)) and 0x3F
	    end if
	  end if
	end if
    end if
    if x > 0xFFFFFFFFFFFFFFFF
	@imm.success = 0
    else if x < -0x8000000000000000
	@imm.success = 0
    end if
    if ec & @imm.success = 0
	err 'logical immediate cannot be encoded'
    end if
end macro

macro aarch64.parse_bimm32 x, ec:
    aarch64.parse_bimm64 0x100000001*(x and 0xFFFFFFFF), ec
    if x > 0xFFFFFFFF
	@imm.success = 0
    else if x < -0x80000000
	@imm.success = 0
    end if
end macro


; err if x cannot be encoded as a arithmetic immediate
macro aarch64.parse_aimm x, ec:0
    @imm.number = +x
    @imm.success = 0
    @imm.shift = 0
    @imm.imm12 = 0
    assert @imm.number relativeto 0
    if @shf.empty
	if @imm.number >= 0
	    @imm.inv = 0
	    if @imm.number < 0x1000
		@imm.shift = 0
		@imm.imm12 = @imm.number
	    else if (@imm.number and 0x0FFF) = 0 & (@imm.number shr 12) < 0x1000
		@imm.shift = 1
		@imm.imm12 = @imm.number shr 12
	    else if ec
		err 'immediate out of range'
	    else
		@imm.success = 0
	    end if	  
	else
	    @imm.inv = 1
	    @imm.number = -@imm.number
	    if @imm.number < 0x1000
		@imm.shift = 0
		@imm.imm12 = @imm.number
	    else if (@imm.number and 0x0FFF) = 0 & (@imm.number shr 12) < 0x1000
		@imm.shift = 1
		@imm.imm12 = @imm.number shr 12
	    else if ec
		err 'immediate out of range'
	    else
		@imm.success = 0
	    end if
	end if
    else if @shf.kind = 0 & @shf.shift = 12
	if @imm.number >= 0
	    @imm.inv = 0
	    if @imm.number < 0x1000
		@imm.shift = 1
		@imm.imm12 = @imm.number
	    else if ec
		err 'immediate out of range'
	    else
		@imm.success = 0
	    end if	  
	else
	    @imm.inv = 1
	    @imm.number = -@imm.number
	    if @imm.number < 0x1000
		@imm.shift = 1
		@imm.imm12 = @imm.number
	    else if ec
		err 'immediate out of range'
	    else
		@imm.success = 0
	    end if
	end if
    else if ec
	err 'invalid immediate and shift'
    else
	@imm.success = 0
    end if
end macro

; err if ns.data cannot be encoded as an 8 bit float
macro aarch64.parse_float ns
    local f, t, msb, lsb
    ns.type = @type_immf
    if ns.data > 0
	f = 128*ns.data
	ns.data = 0x00
    else if ns.data < 0
	f = -128*ns.data
	ns.data = 0x80
    else
	err 'float cannot be encoded'
    end if
    t = trunc f
    if f - t > 0.999999999
	t = t + 1
    end if
    if (-0.000000001 < f - float t) & (f - float t < 0.000000001) & (t <> 0)
	msb = bsr t
	lsb = bsf t
	if (4 <= msb) & (msb <= 11) & (msb - lsb <= 4)
	    ns.data = ns.data + (((msb - 8) and 7) shl 4) + ((t shr (msb - 4)) and 15)
	else
	    err 'float cannot be encoded'
	end if
    else
	err 'float cannot be encoded'
    end if
end macro


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; parsing registers and immediates ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

macro aarch64.parse_colon ns, y, z
    ns.data = +z
    match =lo12, y
	ns.type = @type_immc_lo12
    else match =g0_nc, y
	ns.type = @type_immc_g0_nc
    else match =g1_nc, y
	ns.type = @type_immc_g1_nc
    else match =g2_nc, y
	ns.type = @type_immc_g2_nc
    else match =g0, y
	ns.type = @type_immc_g0
    else match =g1, y
	ns.type = @type_immc_g1
    else match =g2, y
	ns.type = @type_immc_g2
    else match =g3, y
	ns.type = @type_immc_g3
    else match =pg_hi21, y
	ns.type = @type_immc_pg_hi21
    end match
    if ~(ns.data relativeto 0)
	ns.type = ns.type shl 1
    end if
end macro

macro aarch64.parse_bracket ns, y, z
    ns.idx = +z
    ns.data = +y
    if (elementsof ns.data = 1 & ns.data eq ns.data element 1) & (ns.data metadata 1 relativeto aarch64.reg)
	ns.data = ns.data metadata 1 - aarch64.reg
	ns.vfull = 0
	ns.vsize = (ns.data shr 16) and 255
	ns.type  = 1 shl ((ns.data shr 8) and 255)
	if ns.data and @type_vn_all
	    ns.type = @type_vn_bi shl ns.vsize
	else
	    err 'invalid index register'
	end if
	ns.data = ns.data and 255
	aarch64.restrict ns.idx, 0, (1 shl (4 - ns.vsize)) - 1
    else
	err 'invalid index register'
    end if
end macro

macro aarch64.parse_operand ns, x
    ns.type = 0
    match , x
	ns.type = @type_empty
    else match :y:z, x
	aarch64.parse_colon ns, y, z	; uncommon case
    else match y[z], x
	aarch64.parse_bracket ns, y, z	; uncommon case
    else
	ns.data = +x
	if (elementsof ns.data = 1 & ns.data eq ns.data element 1) & (ns.data metadata 1 relativeto aarch64.reg)
	    ns.data = ns.data metadata 1 - aarch64.reg
	    ns.vfull = (ns.data shr 24) and 255
	    ns.vsize = (ns.data shr 16) and 255
	    ns.type  = 1 shl ((ns.data shr 8) and 255)
	    ns.data = ns.data and 255
	else
	    if ns.data eqtype 0.0
		if ns.data = 0.0
		    ns.type = @type_immc
		    ns.data = 0
		else
		    aarch64.parse_float ns
		end if
	    else if ns.data relativeto 0
		ns.type = @type_immc
	    else
		ns.type = @type_immr
	    end if
	end if
    end match
end macro

;err if x is not a register
macro aarch64.parse_register ns, x
    ns.data = +x
    if (elementsof ns.data = 1 & ns.data eq ns.data element 1) & (ns.data metadata 1 relativeto aarch64.reg)
	ns.data = ns.data metadata 1 - aarch64.reg
	ns.vfull = (ns.data shr 24) and 255
	ns.vsize = (ns.data shr 16) and 255
	ns.type  = 1 shl ((ns.data shr 8) and 255)
	ns.data = ns.data and 255
    else
	err 'invalid operand'
    end if
end macro


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; parsing extensions, shifts, and conditions
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

macro aarch64.parse_ext x
    @ext.empty = 0
    @ext.success = 1
    @ext.kind = 3
    @ext.shift = 0
    match , x
	@ext.empty = 1
    else match =lsl a, x
	@ext.shift = +a
    else match =uxtb a, x
	@ext.kind = 0
	@ext.shift = +a
    else match =uxth a, x
	@ext.kind = 1
	@ext.shift = +a
    else match =uxtw a, x
	@ext.kind = 2
	@ext.shift = +a
    else match =uxtx a, x
	@ext.kind = 3
	@ext.shift = +a
    else match =sxtb a, x
	@ext.kind = 4
	@ext.shift = +a
    else match =sxth a, x
	@ext.kind = 5
	@ext.shift = +a
    else match =sxtw a, x
	@ext.kind = 6
	@ext.shift = +a
    else match =sxtx a, x
	@ext.kind = 7
	@ext.shift = +a
    else match =uxtb, x
	@ext.kind = 0
    else match =uxth, x
	@ext.kind = 1
    else match =uxtw, x
	@ext.kind = 2
    else match =uxtx, x
	@ext.kind = 3
    else match =sxtb, x
	@ext.kind = 4
    else match =sxth, x
	@ext.kind = 5
    else match =sxtw, x
	@ext.kind = 6
    else match =sxtx, x
	@ext.kind = 7
    else
	@ext.success = 0
    end match
    if @ext.success & (~(@ext.shift relativeto 0) | @ext.shift < 0 | 4 < @ext.shift)
	@ext.success = 0
    end if
end macro

macro aarch64.parse_shf x
    @shf.empty = 0
    @shf.success = 1
    @shf.kind = 0
    @shf.shift = 0
    match , x
	@shf.empty = 1
    else match =lsl a, x
	@shf.kind = 0
	@shf.shift = +a
    else match =lsr a, x
	@shf.kind = 1
	@shf.shift = +a
    else match =asr a, x
	@shf.kind = 2
	@shf.shift = +a
    else match =ror a, x
	@shf.kind = 3
	@shf.shift = +a
    else
	@shf.success = 0
    end match
    if @shf.success & (~(@shf.shift relativeto 0) | @shf.shift < 0 | 64 <= @shf.shift)
	@shf.success = 0
    end if
end macro

macro aarch64.parse_con x
    @con.success = 1
    @cond.kind = -1
    match , x
	@con.success = 0
    else match =eq?, x
	@con.kind = 0
    else match =ne?, x
	@con.kind = 1
    else match =hs?, x
	@con.kind = 2
    else match =cs?, x
	@con.kind = 2
    else match =lo?, x
	@con.kind = 3
    else match =cc?, x
	@con.kind = 3
    else match =mi?, x
	@con.kind = 4
    else match =pl?, x
	@con.kind = 5
    else match =vs?, x
	@con.kind = 6
    else match =vc?, x
	@con.kind = 7
    else match =hi?, x
	@con.kind = 8
    else match =ls?, x
	@con.kind = 9
    else match =ge?, x
	@con.kind = 10
    else match =lt?, x
	@con.kind = 11
    else match =gt?, x
	@con.kind = 12
    else match =le?, x
	@con.kind = 13
    else match =al?, x
	@con.kind = 14
    else match =nv?, x
	@con.kind = 15
    else
	@con.success = 0
    end match	 
end macro
