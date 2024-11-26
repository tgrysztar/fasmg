macro adrp a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type and @type_xn_xzr) & (@op1.type and @type_immr_immc) & (@op1.data relativeto $)
	@imm.number = (@op1.data scale 0 shr 12) - ($ scale 0 shr 12)
	aarch64.restrict @imm.number, -0x100000, 0xFFFFF 
	dd 0x90000000 + ((@imm.number and 0x3) shl 29) + (((@imm.number and 0x1FFFFC) shr 2) shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_immr)
	aarch64.store_reloc @op1.data - $, -1, R_AARCH64_ADR_PREL_PG_HI21
	dd 0x90000000 + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro adr a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type and @type_xn_xzr) & (@op1.type and @type_immr_immc) & (@op1.data relativeto $)
	@imm.number = @op1.data - $
	aarch64.restrict @imm.number, -0x100000, 0xFFFFF
	dd 0x10000000 + (@imm.number and 0x3) shl 29 + (((@imm.number shr 2) and 0x7FFFF) shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_immr)
	aarch64.store_reloc @op1.data - $, -1, R_AARCH64_ADR_PREL_LO21
	dd 0x10000000 + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro bl a*
    aarch64.parse_operand @op0, a
    if (@op0.type and @type_immr_immc)
	if @op0.data relativeto $
	    @imm.number = @op0.data - $
	    aarch64.restrict @imm.number, -0x8000000, 0x7FFFFFF, 0x3
	    dd 0x94000000 + (((@imm.number shr 2) and 0x3FFFFFF) shl 0)
	else
	    aarch64.store_reloc @op0.data - $, -1, R_AARCH64_CALL26
	    dd 0x94000000 + ((			0 and 0x3FFFFFF) shl 0)
	end if
    else
	err 'invalid operand'
    end if
end macro

macro b a*
    aarch64.parse_operand @op0, a
    if (@op0.type and @type_immr_immc)
	if @op0.data relativeto $
	    @imm.number = @op0.data - $
	    aarch64.restrict @imm.number, -0x8000000, 0x7FFFFFF, 0x3
	    dd 0x14000000 + (((@imm.number shr 2) and 0x3FFFFFF) shl 0)
	else
	    aarch64.store_reloc @op1.data - $, -1, R_AARCH64_JUMP26
	    dd 0x94000000 + ((			0 and 0x3FFFFFF) shl 0)
	end if
    else
	err 'invalid operand'
    end if
end macro

iterate <instr,code>, beq,0x54000000, bne,0x54000001, bhs,0x54000002, blo,0x54000003,\
		      bmi,0x54000004, bpl,0x54000005, bvs,0x54000006, bvc,0x54000007,\
		      bhi,0x54000008, bls,0x54000009, bge,0x5400000a, blt,0x5400000b,\
		      bgt,0x5400000c, ble,0x5400000d, bal,0x5400000e, bnv,0x5400000f,\
		      bcs,0x54000002, bcc,0x54000003
	macro instr a*
	aarch64.parse_operand @op0, a
	if (@op0.type and @type_immr_immc)
	    if @op0.data relativeto $
		@imm.number = @op0.data - $
		aarch64.restrict @imm.number, -0x800000, 0x7FFFFF, 0x3
		dd code + (((@imm.number shr 2) and 0x7FFFF) shl 5)
	    else
		aarch64.store_reloc @op1.data - $, -1, R_AARCH64_CONDBR19
		dd code + (((	       0 shr 2) and 0x7FFFF) shl 5)
	    end if
	else
	    err 'invalid operand'
	end if
	end macro
end iterate

macro cbnz a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type and @type_wn_wzr_xn_xzr) & (@op1.type and @type_immr_immc)
	if @op1.data relativeto $
	    @imm.number = @op1.data - $
	    aarch64.restrict @imm.number, -0x800000, 0x7FFFFF, 0x3
	    dd 0x35000000 + (@op0.vfull shl 31) + (((@imm.number shr 2) and 0x7FFFF) shl 5) + (@op0.data shl 0)
	else
	    aarch64.store_reloc @op1.data - $, -1, R_AARCH64_CONDBR19
	    dd 0x35000000 + (@op0.vfull shl 31) + (((	       0 shr 2) and 0x7FFFF) shl 5) + (@op0.data shl 0)
	end if
    else
	err 'invalid operand'
    end if
end macro

macro cbz a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type and @type_wn_wzr_xn_xzr) & (@op1.type and @type_immr_immc)
	if @op1.data relativeto $
	    @imm.number = @op1.data - $
	    aarch64.restrict @imm.number, -0x100000, 0x7FFFFF, 0x3
	    dd 0x34000000 + (@op0.vfull shl 31) + (((@imm.number shr 2) and 0x7FFFF) shl 5) + (@op0.data shl 0)
	else
	    aarch64.store_reloc @op1.data - $, -1, R_AARCH64_CONDBR19
	    dd 0x34000000 + (@op0.vfull shl 31) + (((	       0 shr 2) and 0x7FFFF) shl 5) + (@op0.data shl 0)
	end if
    else
	err 'invalid operand'
    end if
end macro

macro tbnz a*, b*, c*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if (@op0.type and @type_wn_wzr_xn_xzr) & (@op1.type and @type_immc) & (@op2.type and @type_immr_immc)
	if @op2.data relativeto $
	    @imm.number = @op2.data - $
	    aarch64.restrict @imm.number, -0x8000, 0x7FFF, 0x3
	    dd 0x37000000 + ((@op1.data and 0x20) shl 26) + ((@op1.data and 0x1F) shl 19) + (((@imm.number shr 2) and 0x3FFF) shl 5) + (@op0.data shl 0)
	else
	    err 'invalid target'
	end if
    else
	err 'invalid operand
    end if
end macro

macro tbz a*, b*, c*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if (@op0.type and @type_wn_wzr_xn_xzr) & (@op1.type and @type_immc) & (@op2.type and @type_immr_immc)
	if @op2.data relativeto $
	    @imm.number = @op2.data - $
	    aarch64.restrict @imm.number, -0x8000, 0x7FFF, 0x3
	    dd 0x36000000 + ((@op1.data and 0x20) shl 26) + ((@op1.data and 0x1F) shl 19) + (((@imm.number shr 2) and 0x3FFF) shl 5) + (@op0.data shl 0)
	else
	    err 'invalid target'
	end if
    else
	err 'invalid operand
    end if
end macro

macro movk a*, b*, c
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_shf		c
    if (@op0.type and @type_wn_wzr_xn_xzr)
	@adr.encode = 0x72800000 + (@op0.vfull shl 31) + (@op0.data shl 0)
	if (@op1.type and @type_immc) & (@shf.success) & (@shf.kind = 0)
	    aarch64.restrict @shf.shift, 0, 48, 0x0f
	    aarch64.restrict @op1.data, 0, 0xFFFF
	    dd @adr.encode + (@shf.shift shl 17) + (@op1.data shl 5)
	else if @shf.empty = 0
	    err 'invalid operand'
	else if @op1.type = @type_immc_g0_nc
	    dd @adr.encode + (	       0 shl 17) + (((@op1.data shr  0) and 0xFFFF) shl 5)
	else if @op1.type = @type_immc_g1_nc
	    dd @adr.encode + (	      16 shl 17) + (((@op1.data shr 16) and 0xFFFF) shl 5)
	else if @op1.type = @type_immc_g2_nc
	    dd @adr.encode + (	      32 shl 17) + (((@op1.data shr 32) and 0xFFFF) shl 5)
	else if @op1.type = @type_immc_g3
	    dd @adr.encode + (	      48 shl 17) + (((@op1.data shr 48) and 0xFFFF) shl 5)
	else if @op1.type = @type_immr_g0_nc
	    aarch64.store_reloc @op1.data, R_AARCH64_MOVW_UABS_G0_NC, R_AARCH64_MOVW_PREL_G0_NC
	    dd @adr.encode + (	       0 shl 17) + (((	      0 shr  0) and 0xFFFF) shl 5)
	else if @op1.type = @type_immr_g1_nc
	    aarch64.store_reloc @op1.data, R_AARCH64_MOVW_UABS_G1_NC, R_AARCH64_MOVW_PREL_G1_NC
	    dd @adr.encode + (	      16 shl 17) + (((	      0 shr 16) and 0xFFFF) shl 5)
	else if @op1.type = @type_immr_g2_nc
	    aarch64.store_reloc @op1.data, R_AARCH64_MOVW_UABS_G2_NC, R_AARCH64_MOVW_PREL_G1_NC
	    dd @adr.encode + (	      32 shl 17) + (((	      0 shr 32) and 0xFFFF) shl 5)
	else if @op1.type = @type_immr_g3
	    aarch64.store_reloc @op1.data, R_AARCH64_MOVW_UABS_G3,    -1
	    dd @adr.encode + (	      48 shl 17) + (((	      0 shr 48) and 0xFFFF) shl 5)
	else
	    err 'invalid operand'
	end if
    else
	err 'invalid operand'
    end if
end macro

macro movz a*, b*, c
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_shf		c
    if (@op0.type and @type_wn_wzr_xn_xzr)
	@adr.encode = 0x52800000 + (@op0.vfull shl 31) + (@op0.data shl 0)
	if (@op1.type and @type_immc) & (@shf.success) & (@shf.kind = 0)
	    aarch64.restrict @shf.shift, 0, 48, 0x0f
	    aarch64.restrict @op1.data, 0, 0xFFFF
	    dd @adr.encode + (@shf.shift shl 17) + (@op1.data shl 5)
	else if @shf.empty = 0
	    err 'invalid operand'
	else if @op1.type = @type_immc_g0
	    aarch64.restrict @op1.data, 0, 0xFFFF
	    dd @adr.encode + (	       0 shl 17) + (((@op1.data shr ( 0)) and 0xFFFF) shl 5)
	else if @op1.type = @type_immc_g1
	    aarch64.restrict @op1.data, 0, 0xFFFFFFFF
	    dd @adr.encode + (	      16 shl 17) + (((@op1.data shr (16)) and 0xFFFF) shl 5)
	else if @op1.type = @type_immc_g2
	    aarch64.restrict @op1.data, 0, 0xFFFFFFFFFFFF
	    dd @adr.encode + (	      32 shl 17) + (((@op1.data shr (32)) and 0xFFFF) shl 5)
	else if @op1.type = @type_immc_g3
	    dd @adr.encode + (	      48 shl 17) + (((@op1.data shr (48)) and 0xFFFF) shl 5)

	else if @op1.type = @type_immr_g0
	    aarch64.store_reloc @op1.data, R_AARCH64_MOVW_SABS_G0, R_AARCH64_MOVW_PREL_G0
	    dd @adr.encode + (	       0 shl 17) + (((	      0 shr ( 0)) and 0xFFFF) shl 5)
	else if @op1.type = @type_immr_g1
	    aarch64.store_reloc @op1.data, R_AARCH64_MOVW_SABS_G1, R_AARCH64_MOVW_PREL_G1
	    dd @adr.encode + (	      16 shl 17) + (((	      0 shr (16)) and 0xFFFF) shl 5)
	else if @op1.type = @type_immr_g2
	    aarch64.store_reloc @op1.data, R_AARCH64_MOVW_SABS_G2, R_AARCH64_MOVW_PREL_G2
	    dd @adr.encode + (	      32 shl 17) + (((	      0 shr (32)) and 0xFFFF) shl 5)
	else if @op1.type = @type_immr_g3
	    aarch64.store_reloc @op1.data, R_AARCH64_MOVW_UABS_G3, R_AARCH64_MOVW_PREL_G3
	    dd @adr.encode + (	      48 shl 17) + (((	      0 shr (48)) and 0xFFFF) shl 5)

	else
	    err 'invalid operand'
	end if
    else
	err 'invalid operand'
    end if
end macro

macro movn a*, b*, c
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_shf		c
    if (@op0.type and @type_wn_wzr_xn_xzr)
	@adr.encode = 0x12800000 + (@op0.vfull shl 31) + (@op0.data shl 0)
	if (@op1.type and @type_immc) & (@shf.success) & (@shf.kind = 0)
	    aarch64.restrict @shf.shift, 0, 48, 0x0f
	    aarch64.restrict @op1.data, 0, 0xFFFF
	    dd @adr.encode + (@shf.shift shl 17) + (@op1.data shl 5)
	else if @shf.empty = 0
	    err 'invalid operand'
	else if @op1.type = @type_immc_g0
	    aarch64.restrict @op1.data, 0, 0xFFFF
	    dd @adr.encode + (	       0 shl 17) + (((@op1.data shr ( 0)) and 0xFFFF) shl 5)
	else if @op1.type = @type_immc_g1
	    aarch64.restrict @op1.data, 0, 0xFFFFFFFF
	    dd @adr.encode + (	      16 shl 17) + (((@op1.data shr (16)) and 0xFFFF) shl 5)
	else if @op1.type = @type_immc_g2
	    aarch64.restrict @op1.data, 0, 0xFFFFFFFFFFFF
	    dd @adr.encode + (	      32 shl 17) + (((@op1.data shr (32)) and 0xFFFF) shl 5)
	else if @op1.type = @type_immc_g3
	    dd @adr.encode + (	      48 shl 17) + (((@op1.data shr (48)) and 0xFFFF) shl 5)

	else if @op1.type = @type_immr_g0
	    aarch64.store_reloc @op1.data, R_AARCH64_MOVW_SABS_G0, R_AARCH64_MOVW_PREL_G0
	    dd @adr.encode + (	       0 shl 17) + (((	      0 shr ( 0)) and 0xFFFF) shl 5)
	else if @op1.type = @type_immr_g1
	    aarch64.store_reloc @op1.data, R_AARCH64_MOVW_SABS_G1, R_AARCH64_MOVW_PREL_G1
	    dd @adr.encode + (	      16 shl 17) + (((	      0 shr (16)) and 0xFFFF) shl 5)
	else if @op1.type = @type_immr_g2
	    aarch64.store_reloc @op1.data, R_AARCH64_MOVW_SABS_G2, R_AARCH64_MOVW_PREL_G2
	    dd @adr.encode + (	      32 shl 17) + (((	      0 shr (32)) and 0xFFFF) shl 5)
	else if @op1.type = @type_immr_g3
	    aarch64.store_reloc @op1.data, R_AARCH64_MOVW_UABS_G3, R_AARCH64_MOVW_PREL_G3
	    dd @adr.encode + (	      48 shl 17) + (((	      0 shr (48)) and 0xFFFF) shl 5)

	else
	    err 'invalid operand'
	end if
    else
	err 'invalid operand'
    end if
end macro


;iterate <instr, code1, code2>, movk, 0x72800000,\
;				movn, 0x12800000,\
;				movz, 0x52800000
;    macro instr a*, b*, c
;	 aarch64.parse_operand @op0, a
;	 aarch64.parse_operand @op1, b
;	 aarch64.parse_shf	     c
;	 if (@op0.type and @type_wn_wzr_xn_xzr) & (@op1.type and @type_immc) & (@shf.success) & (@shf.kind = 0)
;	     aarch64.restrict @shf.shift 0, 48, 0x0f
;	     aarch64.restrict @op1.data, 0, 0xFFFF
;	     dd code1 + (@op0.vtype shl 31) + (@shf.shift shl 17) + (@op1.data shl 5) + (@op0.data shl 0)
;	 else if (@op0.type and @type_wn_wzr_xn_xzr) & (@op1.type and @type_immr) & (@shf.empty)
;
;	     dd code1 + (@op0.vtype shl 31) + (@shf.shift shl 17) + (@op1.data shl 5) + (@op0.data shl 0)
;	 else
;	     err 'invalid operand'
;	 end if
;    end macro
;end iterate



iterate <instr, code1, code2>, adc, 0x1a000000, 0x9a000000,\
			       sbc, 0x5a000000, 0xda000000,\
			       sbcs, 0x7a000000, 0xfa000000,\
			       adcs, 0x3a000000, 0xba000000
    macro instr a*, b*, c*
	aarch64.parse_operand @op0, a
	aarch64.parse_operand @op1, b
	aarch64.parse_operand @op2, c
	if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr) & (@op2.type and @type_wn_wzr)
	    dd code1 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
	else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr) & (@op2.type and @type_xn_xzr)
	    dd code2 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
	else
	    err 'invalid operand'
	end if
    end macro
end iterate

iterate <instr, code1, code2, code3, code4>, ccmn, 0x3a400000, 0xba400000, 0x3a400800, 0xba400800,\
					     ccmp, 0x7a400000, 0xfa400000, 0x7a400800, 0xfa400800
    macro instr a*, b*, c*, d*
	aarch64.parse_operand @op0, a
	aarch64.parse_operand @op1, b
	aarch64.parse_operand @op2, c
	aarch64.parse_con	    d
	if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr) & (@op2.type and @type_immc) & (@con.success)
	    dd code1 + (@con.kind shl 12) + (@op1.data shl 16) + (@op0.data shl 5) + ((@op2.data and 0x0F) shl 0)
	else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr) & (@op2.type and @type_immc) & (@con.success)
	    dd code2 + (@con.kind shl 12) + (@op1.data shl 16) + (@op0.data shl 5) + ((@op2.data and 0x0F) shl 0)
	else if (@op0.type and @type_wn_wzr) & (@op1.type and @type_immc) & (@op2.type and @type_immc) & (@con.success)
	    aarch64.restrict @op1.data, 0, 31
	    dd code3 + (@con.kind shl 12) + (@op1.data shl 16) + (@op0.data shl 5) + ((@op2.data and 0x0F) shl 0)
	else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_immc) & (@op2.type and @type_immc) & (@con.success)
	    aarch64.restrict @op1.data, 0, 31
	    dd code4 + (@con.kind shl 12) + (@op1.data shl 16) + (@op0.data shl 5) + ((@op2.data and 0x0F) shl 0)
	else
	    err 'invalid operand'
	end if
    end macro
end iterate

iterate <instr, c1, c2, c3, c4, c5, c6, c7>, adds, 0x31000000, 0xb1000000, 0x2b000000, 0xab000000, 0x2b200000, 0xab200000, 0xab200000,\
					     subs, 0x71000000, 0xf1000000, 0x6b000000, 0xeb000000, 0x6b200000, 0xeb200000, 0xeb200000
    macro instr a*, b*, c*, d
	aarch64.parse_operand @op0, a
	aarch64.parse_operand @op1, b
	aarch64.parse_operand @op2, c
	aarch64.parse_ext	    d
	aarch64.parse_shf	    d
	if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wsp) & (@op2.type and @type_immc)
	    aarch64.parse_aimm @op2.data, 1
	    dd c1 + (@imm.inv shl 30) + (@imm.shift shl 22) + (@imm.imm12 shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
	else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xsp) & (@op2.type and @type_immc)
	    aarch64.parse_aimm @op2.data, 1
	    dd c2 + (@imm.inv shl 30) + (@imm.shift shl 22) + (@imm.imm12 shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
	else if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr) & (@op2.type and @type_wn_wzr) & (@shf.success)
	    dd c3 + (@shf.kind shl 22) + (@op2.data shl 16) + (@shf.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
	else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr) & (@op2.type and @type_xn_xzr) & (@shf.success)
	    dd c4 + (@shf.kind shl 22) + (@op2.data shl 16) + (@shf.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
	else if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wsp) & (@op2.type and @type_wn_wzr) & (@ext.success)
	    dd c5 + (@op2.data shl 16) + (@ext.kind shl 13) + (@ext.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
	else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xsp) & (@op2.type and @type_wn_wzr) & (@ext.empty)
	    dd c6 + (@op2.data shl 16) + (	  2 shl 13) + ( 	0 shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
	else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xsp) & (@op2.type and @type_wn_wzr_xn_xzr) & (@ext.success)
	    dd c7 + (@op2.data shl 16) + (@ext.kind shl 13) + (@ext.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
	else
	    err 'invalid operand'
	end if
    end macro
end iterate

iterate <instr, code1, code2>, mneg, 0x1b00fc00, 0x9b00fc00,\
			       sdiv, 0x1ac00c00, 0x9ac00c00,\
			       udiv, 0x1ac00800, 0x9ac00800
    macro instr a*, b*, c*
	aarch64.parse_operand @op0, a
	aarch64.parse_operand @op1, b
	aarch64.parse_operand @op2, c
	if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr) & (@op2.type and @type_wn_wzr)
	    dd code1 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
	else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr) & (@op2.type and @type_xn_xzr)
	    dd code2 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
	else
	    err 'invalid operand'
	end if
    end macro
end iterate

iterate <instr, code1, code2>, ngc,  0x5a000000, 0xda000000,\
			       ngcs, 0x7a000000, 0xfa000000
    macro instr a*, c*
	aarch64.parse_operand @op0, a
	aarch64.parse_operand @op2, c
	if (@op0.type and @type_wn_wzr) & (@op2.type and @type_wn_wzr)
	    dd code1 + (@op2.data shl 16) + ( 31 shl 5) + (@op0.data shl 0)
	else if (@op0.type and @type_xn_xzr) & (@op2.type and @type_xn_xzr)
	    dd code2 + (@op2.data shl 16) + ( 31 shl 5) + (@op0.data shl 0)
	else
	    err 'invalid operand'
	end if
    end macro
end iterate

macro add a*, b*, c*, d
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    aarch64.parse_ext		d
    aarch64.parse_shf		d
    if (@op0.type and @type_wn_wsp) & (@op1.type and @type_wn_wsp) & (@op2.type and @type_immc)
	aarch64.parse_aimm @op2.data, 1
	dd 0x11000000 + (@imm.inv shl 30) + (@imm.shift shl 22) + (@imm.imm12 shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xsp) & (@op1.type and @type_xn_xsp) & (@op2.type and @type_immc)
	aarch64.parse_aimm @op2.data, 1
	dd 0x91000000 + (@imm.inv shl 30) + (@imm.shift shl 22) + (@imm.imm12 shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr) & (@op2.type and @type_wn_wzr) & (@shf.success)
	dd 0x0b000000 + (@shf.kind shl 22) + (@op2.data shl 16) + (@shf.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr) & (@op2.type and @type_xn_xzr) & (@shf.success)
	dd 0x8b000000 + (@shf.kind shl 22) + (@op2.data shl 16) + (@shf.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_wn_wsp) & (@op1.type and @type_wn_wsp) & (@op2.type and @type_wn_wzr) & (@ext.success)
	dd 0x0b200000 + (@op2.data shl 16) + (@ext.kind shl 13) + (@ext.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xsp) & (@op1.type and @type_xn_xsp) & (@op2.type and @type_wn_wzr) & (@ext.empty)
	dd 0x8b200000 + (@op2.data shl 16) + (	      2 shl 13) + (	    0 shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xsp) & (@op1.type and @type_xn_xsp) & (@op2.type and @type_wn_wzr_xn_xzr) & (@ext.success)
	dd 0x8b200000 + (@op2.data shl 16) + (@ext.kind shl 13) + (@ext.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xsp) & (@op1.type and @type_xn_xsp) & (@op2.type and @type_immc_lo12)
	dd 0x91000000 + (	 0 shl 30) + (	       0 shl 22) + ((@op2.data and 0xFFF) shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_wn_wsp) & (@op1.type and @type_wn_wsp) & (@op2.type and @type_immc_lo12)
	dd 0x11000000 + (	 0 shl 30) + (	       0 shl 22) + ((@op2.data and 0xFFF) shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xsp) & (@op1.type and @type_xn_xsp) & (@op2.type and @type_immr_lo12) & (@ext.empty)
	aarch64.store_reloc @op2.data, R_AARCH64_ADD_ABS_LO12_NC, -1
	dd 0x91000000 + (	 0 shl 30) + (	       0 shl 22) + ((	     0 and 0xFFF) shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_wn_wsp) & (@op1.type and @type_wn_wsp) & (@op2.type and @type_immr_lo12) & (@ext.empty)
	aarch64.store_reloc @op2.data, R_AARCH64_ADD_ABS_LO12_NC, -1
	dd 0x11000000 + (	 0 shl 30) + (	       0 shl 22) + ((	     0 and 0xFFF) shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_16b_etc_no_1d) & (@op0.type = @op1.type) & (@op0.type = @op2.type) & (@ext.empty)
	dd 0x0e208400 + (@op2.data shl 16) + (@op0.vfull shl 30) + (@op0.vsize shl 22) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_dn) & (@op0.type = @op1.type) & (@op0.type = @op2.type) & (@ext.empty)
	dd 0x5ee08400 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro sub a*, b*, c*, d
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    aarch64.parse_ext		d
    aarch64.parse_shf		d
    if (@op0.type and @type_wn_wsp) & (@op1.type and @type_wn_wsp) & (@op2.type and @type_immc)
	aarch64.parse_aimm @op2.data, 1
	dd 0x51000000 - (@imm.inv shl 30) + (@imm.shift shl 22) + (@imm.imm12 shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xsp) & (@op1.type and @type_xn_xsp) & (@op2.type and @type_immc)
	aarch64.parse_aimm @op2.data, 1
	dd 0xd1000000 - (@imm.inv shl 30) + (@imm.shift shl 22) + (@imm.imm12 shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr) & (@op2.type and @type_wn_wzr) & (@shf.success)
	dd 0x4b000000 + (@shf.kind shl 22) + (@op2.data shl 16) + (@shf.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr) & (@op2.type and @type_xn_xzr) & (@shf.success)
	dd 0xcb000000 + (@shf.kind shl 22) + (@op2.data shl 16) + (@shf.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_wn_wsp) & (@op1.type and @type_wn_wsp) & (@op2.type and @type_wn_wzr) & (@ext.success)
	dd 0x4b200000 + (@op2.data shl 16) + (@ext.kind shl 13) + (@ext.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xsp) & (@op1.type and @type_xn_xsp) & (@op2.type and @type_wn_wzr) & (@ext.empty)
	dd 0xcb200000 + (@op2.data shl 16) + (	      2 shl 13) + (	    0 shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xsp) & (@op1.type and @type_xn_xsp) & (@op2.type and @type_wn_wzr_xn_xzr) & (@ext.success)
	dd 0xcb200000 + (@op2.data shl 16) + (@ext.kind shl 13) + (@ext.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_16b_etc_no_1d) & (@op0.type = @op1.type) & (@op0.type = @op1.type)
	dd 0x2e208400 + (@op2.data shl 16) + (((bsf @op0.type) and 4) shl 28) + (((bsf @op0.type) and 3) shl 22) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_dn) & (@op0.type = @op1.type) & (@op0.type = @op2.type) & (@ext.empty)
	dd 0x7ee08400 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro


macro abs a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type and @type_dn) & (@op0.type = @op1.type)
	dd 0x5ee0b800 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_16b_etc_no_1d) & (@op0.type = @op1.type)
	dd 0x0e20b800 + (@op0.vfull shl 30) + (@op0.vsize shl 22) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if    
end macro

macro addhn a*, b*, c*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if (@op0.type = @type_vn_2s) & (@op1.type = @type_vn_2d) & (@op2.type = @type_vn_2d)
	dd 0x0ea04000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_4h) & (@op1.type = @type_vn_4s) & (@op2.type = @type_vn_4s)
	dd 0x0e604000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_8b) & (@op1.type = @type_vn_8h) & (@op2.type = @type_vn_8h)
	dd 0x0e204000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if    
end macro

macro addhn2 a*, b*, c*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if (@op0.type = @type_vn_4s) & (@op1.type = @type_vn_2d) & (@op2.type = @type_vn_2d)
	dd 0x4ea04000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_8h) & (@op1.type = @type_vn_4s) & (@op2.type = @type_vn_4s)
	dd 0x4e604000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_16b) & (@op1.type = @type_vn_8h) & (@op2.type = @type_vn_8h)
	dd 0x4e204000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if    
end macro

macro addp a*, b*, c
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if (@op0.type and @type_dn) & (@op1.type = @type_vn_2d) & (@op2.type = @type_empty)
	dd 0x5ef1b800+ (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_16b_etc_no_1d) & (@op0.type = @op1.type) & (@op1.type = @op1.type)
	dd 0x0e20bc00 + (@op0.vfull shl 30) + ((@op0.vsize) shl 22) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if    
end macro

macro addv a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type and @type_bn_hn_sn) & (@op1.type and @type_vn_all_bhs) & (@op0.vsize = @op1.vsize)
	dd 0x0e31b800 + (@op1.vfull shl 30) + ((@op1.vsize) shl 22) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if    
end macro


macro aarch64.h_aesd_ect a*, b*, code*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if @op0.type = @type_vn_16b & @op1.type = @type_vn_16b
	dd code + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro aese a*, b*
    aarch64.h_aesd_ect a, b, 0x4e284800
end macro

macro aesd a*, b*
    aarch64.h_aesd_ect a, b, 0x4e285800
end macro

macro aesimc a*, b*
    aarch64.h_aesd_ect a, b, 0x4e287800
end macro

macro aesmc a*, b*
    aarch64.h_aesd_ect a, b, 0x4e286800
end macro

macro mvn b*, c*, d
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    aarch64.parse_shf		d
    if (@op1.type and @type_wn_wzr) & (@op2.type and @type_wn_wzr) & (@shf.success)
	dd 0x2a200000 + (@shf.kind shl 22) + (@op2.data shl 16) + (@shf.shift shl 10) + (31 shl 5) + (@op1.data shl 0)
    else if (@op1.type and @type_xn_xzr) & (@op2.type and @type_xn_xzr) & (@shf.success)
	dd 0xaa200000 + (@shf.kind shl 22) + (@op2.data shl 16) + (@shf.shift shl 10) + (31 shl 5) + (@op1.data shl 0)
    else if (@op1.type and @type_vn_16b_etc) & (@op2.type = @op1.type) & (@shf.empty)
	dd 0x2e205800 + (@op1.vfull shl 30) + (@op2.data shl 5) + (@op1.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro not b*, c*, d
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    aarch64.parse_shf		d
    if (@op1.type and @type_vn_16b_etc) & (@op2.type = @op1.type) & (@shf.empty)
	dd 0x2e205800 + (@op1.vfull shl 30) + (@op2.data shl 5) + (@op1.data shl 0)
    else
	err 'invalid operand'
    end if
end macro



macro and a*, b*, c*, d
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    aarch64.parse_shf		d
    if (@op0.type and @type_wn_wsp) & (@op1.type and @type_wn_wzr) & (@op2.type and @type_immc) & (@shf.empty)
	aarch64.parse_bimm32 @op2.data, 1
	dd 0x12000000 + (@imm.n shl 22) + (@imm.r shl 16) + (@imm.s shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xsp) & (@op1.type and @type_xn_xsp) & (@op2.type and @type_immc) & (@shf.empty)
	aarch64.parse_bimm64 @op2.data, 1
	dd 0x92000000 + (@imm.n shl 22) + (@imm.r shl 16) + (@imm.s shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr) & (@op2.type and @type_wn_wzr) & (@shf.success)
	dd 0x0a000000 + (@shf.kind shl 22) + (@op2.data shl 16) + (@shf.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr) & (@op2.type and @type_xn_xzr) & (@shf.success)
	dd 0x8a000000 + (@shf.kind shl 22) + (@op2.data shl 16) + (@shf.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_16b_etc) & (@op0.type = @op1.type) & (@op0.type = @op2.type)
	dd 0x0e201c00 + (@op0.vfull shl 30) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)	
    else
	err 'invalid operand'
    end if
end macro

macro orn a*, b*, c*, d
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    aarch64.parse_shf		d
    if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr) & (@op2.type and @type_wn_wzr) & (@shf.success)
	dd 0x2a200000 + (@shf.kind shl 22) + (@op2.data shl 16) + (@shf.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr) & (@op2.type and @type_xn_xzr) & (@shf.success)
	dd 0xaa200000 + (@shf.kind shl 22) + (@op2.data shl 16) + (@shf.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_16b_etc) & (@op0.type = @op1.type) & (@op0.type = @op2.type)
	dd 0x0ee01c00 + (@op0.vfull shl 30) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)	
    else
	err 'invalid operand'
    end if
end macro


macro eor a*, b*, c*, d
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    aarch64.parse_shf		d
    if (@op0.type and @type_wn_wsp) & (@op1.type and @type_wn_wzr) & (@op2.type and @type_immc) & (@shf.empty)
	aarch64.parse_bimm32 @op2.data, 1
	dd 0x52000000 + (@imm.n shl 22) + (@imm.r shl 16) + (@imm.s shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xsp) & (@op1.type and @type_xn_xsp) & (@op2.type and @type_immc) & (@shf.empty)
	aarch64.parse_bimm64 @op2.data, 1
	dd 0xd2000000 + (@imm.n shl 22) + (@imm.r shl 16) + (@imm.s shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr) & (@op2.type and @type_wn_wzr) & (@shf.success)
	dd 0x4a000000 + (@shf.kind shl 22) + (@op2.data shl 16) + (@shf.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr) & (@op2.type and @type_xn_xzr) & (@shf.success)
	dd 0xca000000 + (@shf.kind shl 22) + (@op2.data shl 16) + (@shf.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_16b_etc) & (@op0.type = @op1.type) & (@op0.type = @op2.type)
	dd 0x2e201c00 + (@op0.vfull shl 30) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)	
    else
	err 'invalid operand'
    end if
end macro

macro bif a*, b*, c*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if (@op0.type and @type_vn_16b_etc) & (@op0.type = @op1.type) & (@op0.type = @op2.type)
	dd 0x2ee01c00 + (@op0.vfull shl 30) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)	
    else
	err 'invalid operand'
    end if
end macro

macro bit a*, b*, c*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if (@op0.type and @type_vn_16b_etc) & (@op0.type = @op1.type) & (@op0.type = @op2.type)
	dd 0x2ea01c00 + (@op0.vfull shl 30) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)	
    else
	err 'invalid operand'
    end if
end macro

macro bsl a*, b*, c*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if (@op0.type and @type_vn_16b_etc) & (@op0.type = @op1.type) & (@op0.type = @op2.type)
	dd 0x2e601c00 + (@op0.vfull shl 30) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)	
    else
	err 'invalid operand'
    end if
end macro


macro bic a*, b*, c, d
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_shf		d
    if (@op0.type and @type_wn_wzr_xn_xzr)
	aarch64.parse_operand @op2, c
	if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr) & (@op2.type and @type_wn_wzr) & (@shf.success)
	    dd 0x0a200000 + (@shf.kind shl 22) + (@op2.data shl 16) + (@shf.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
	else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr) & (@op2.type and @type_xn_xzr) & (@shf.success)
	    dd 0x8a200000 + (@shf.kind shl 22) + (@op2.data shl 16) + (@shf.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
	else
	    err 'invalid operand'
	end if
    else if @shf.empty = 0
	err 'invalid operand'
    else if (@op0.type and @type_vn_16b_etc) & (@op0.type = @op1.type)
	aarch64.parse_operand @op2, c
	if @op0.type = @op2.type
	    dd 0x0e601c00 + (@op0.vfull shl 30) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)	    
	else
	    err 'invalid operand'
	end if
    else if (@op0.type and @type_vn_4h_8h_2s_4s) & (@op1.type = @type_immc)
	aarch64.parse_shf c
	if @shf.success & @shf.kind = 0
	    aarch64.restrict @shf.shift, 0, 8*(@op0.vsize and 2) + 8, 7
	    aarch64.restrict @op1.data, 0, 0xFF
	    dd 0x2f009400 + (@op0.vfull shl 30) - ((@op0.vsize and 2) shl 14) + (@shf.shift shl 10) + ((@op1.data and 0xE0) shl 11) + ((@op1.data and 0x1F) shl 5) + (@op0.data shl 0)	      
	else
	    err 'invalid shift'
	end if
    else
	err 'invalid operand'
    end if	  
end macro

macro orr a*, b*, c, d
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_shf		d
    if (@op0.type and @type_wn_wzr_xn_xzr)
	aarch64.parse_operand @op2, c
	if (@op0.type and @type_wn_wsp) & (@op1.type and @type_wn_wzr) & (@op2.type and @type_immc) & (@shf.empty)
	    aarch64.parse_bimm32 @op2.data, 1
	    dd 0x32000000 + (@imm.n shl 22) + (@imm.r shl 16) + (@imm.s shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
	else if (@op0.type and @type_xn_xsp) & (@op1.type and @type_xn_xsp) & (@op2.type and @type_immc) & (@shf.empty)
	    aarch64.parse_bimm64 @op2.data, 1
	    dd 0xb2000000 + (@imm.n shl 22) + (@imm.r shl 16) + (@imm.s shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
	else if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr) & (@op2.type and @type_wn_wzr) & (@shf.success)
	    dd 0x2a000000 + (@shf.kind shl 22) + (@op2.data shl 16) + (@shf.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
	else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr) & (@op2.type and @type_xn_xzr) & (@shf.success)
	    dd 0xaa000000 + (@shf.kind shl 22) + (@op2.data shl 16) + (@shf.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
	else
	    err 'invalid operand'
	end if
    else if @shf.empty = 0
	err 'invalid operand'
    else if (@op0.type and @type_vn_16b_etc) & (@op0.type = @op1.type)
	aarch64.parse_operand @op2, c
	if @op0.type = @op2.type
	    dd 0x0ea01c00 + (@op0.vfull shl 30) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)	    
	else
	    err 'invalid operand'
	end if
    else if (@op0.type and @type_vn_4h_8h_2s_4s) & (@op1.type = @type_immc)
	aarch64.parse_shf c
	if @shf.success & @shf.kind = 0
	    aarch64.restrict @shf.shift, 0, 8*(@op0.vsize and 2) + 8, 7
	    aarch64.restrict @op1.data, 0, 0xFF
	    dd 0x0f009400 + (@op0.vfull shl 30) - ((@op0.vsize and 2) shl 14) + (@shf.shift shl 10) + ((@op1.data and 0xE0) shl 11) + ((@op1.data and 0x1F) shl 5) + (@op0.data shl 0)	      
	else
	    err 'invalid shift'
	end if
    else
	err 'invalid operand'
    end if
end macro


macro aarch64.parse_uimm64_movi x
    @imm.data = x and 0x0101010101010101
    if @imm.data * 0xFF = x
	@imm.data = (@imm.data and 0x0001000100010001) or ((@imm.data and 0x0100010001000100) shr (8-1))
	@imm.data = (@imm.data and 0x0000000300000003) or ((@imm.data and 0x0003000000030000) shr (16-2))
	@imm.data = (@imm.data and 0x000000000000000F) or ((@imm.data and 0x0000000F00000000) shr (32-4))
	@imm.data = @imm.data and 0xFF
    else
	err 'immediate cannot be encoded'
    end if
end macro

macro aarch64.parse_shf_movi x
    match =msl i, x
	@shf.kind = 17
	@shf.shift = +i
	@shf.success = 1
	@shf.empty = 0
    else
	aarch64.parse_shf x
    end match
end macro

macro movi a*, b*, c
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_shf_movi c
    if (@shf.success) & (@shf.kind = 0) & (@op0.type and @type_vn_4h_8h_2s_4s) & (@op1.type = @type_immc)
	aarch64.restrict @shf.shift, 0, 8*(@op0.vsize and 2) + 8, 7
	aarch64.restrict @op1.data, 0, 0xFF
	dd 0x0f008400 + (@op0.vfull shl 30) - ((@op0.vsize and 2) shl 14) + (@shf.shift shl 10) + ((@op1.data and 0xE0) shl 11) + ((@op1.data and 0x1F) shl 5) + (@op0.data shl 0)	  
    else if (@shf.success) & (@shf.kind = 17) & (@op0.type and @type_vn_2s_4s) & (@op1.type = @type_immc)
	aarch64.restrict @shf.shift, 8, 16, 7
	aarch64.restrict @op1.data, 0, 0xFF
	dd 0x0f00c400 + (@op0.vfull shl 30) + ((@shf.shift and 16) shl 8) + ((@op1.data and 0xE0) shl 11) + ((@op1.data and 0x1F) shl 5) + (@op0.data shl 0)	    
    else if (@shf.empty) & (@op0.type and @type_vn_8b_16b) & (@op1.type = @type_immc)
	aarch64.restrict @op1.data, 0, 0xFF
	dd 0x0f00e400 + (@op0.vfull shl 30) + ((@op1.data and 0xE0) shl 11) + ((@op1.data and 0x1F) shl 5) + (@op0.data shl 0)
    else if (@shf.empty) & (@op0.type = @type_vn_2d) & (@op1.type = @type_immc)
	aarch64.parse_uimm64_movi @op1.data
	dd 0x6f00e400 + ((@imm.data and 0xE0) shl 11) + ((@imm.data and 0x1F) shl 5) + (@op0.data shl 0)
    else if (@shf.empty) & (@op0.type = @type_dn) & (@op1.type = @type_immc)
	aarch64.parse_uimm64_movi @op1.data
	dd 0x2f00e400 + ((@imm.data and 0xE0) shl 11) + ((@imm.data and 0x1F) shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro mvni a*, b*, c
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_shf_movi c
    if (@shf.success) & (@shf.kind = 0) & (@op0.type and @type_vn_4h_8h_2s_4s) & (@op1.type = @type_immc)
	aarch64.restrict @shf.shift, 0, 8*(@op0.vsize and 2) + 8, 7
	aarch64.restrict @op1.data, 0, 0xFF
	dd 0x2f008400 + (@op0.vfull shl 30) - ((@op0.vsize and 2) shl 14) + (@shf.shift shl 10) + ((@op1.data and 0xE0) shl 11) + ((@op1.data and 0x1F) shl 5) + (@op0.data shl 0)	  
    else if (@shf.success) & (@shf.kind = 17) & (@op0.type and @type_vn_2s_4s) & (@op1.type = @type_immc)
	aarch64.restrict @shf.shift, 8, 16, 7
	aarch64.restrict @op1.data, 0, 0xFF
	dd 0x2f00c400 + (@op0.vfull shl 30) + ((@shf.shift and 16) shl 8) + ((@op1.data and 0xE0) shl 11) + ((@op1.data and 0x1F) shl 5) + (@op0.data shl 0)	    
    else
	err 'invalid operand'
    end if
end macro


macro clz a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr)
	dd 0x5ac01000 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr)
	dd 0xdac01000 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_8b_16b_4h_8h_2s_4s) & (@op0.type = @op1.type)
	dd 0x2e204800 + (@op0.vfull shl 30) + ((@op0.vsize) shl 22) + (@op1.data shl 5) + (@op0.data shl 0)	   
    else
	err 'invalid operand'
    end if
end macro

macro cls a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr)
	dd 0x5ac01400 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr)
	dd 0xdac01400 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_8b_16b_4h_8h_2s_4s) & (@op0.type = @op1.type)
	dd 0x0e204800 + (@op0.vfull shl 30) + ((@op0.vsize) shl 22) + (@op1.data shl 5) + (@op0.data shl 0)	   
    else
	err 'invalid operand'
    end if
end macro

macro cnt a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type and @type_vn_8b_16b) & (@op0.type = @op1.type)
	dd 0x0e205800 + (@op0.vfull shl 30) + (@op1.data shl 5) + (@op0.data shl 0)	   
    else
	err 'invalid operand'
    end if
end macro



macro aarch64.h_cmeq_ect a*, b*, c*, code1*, code2*, code3*, code4*, code5*, code6*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if (@op0.type and @type_vn_16b_etc_no_1d) & (@op0.type = @op1.type) & (@op0.type = @op2.type)
	dd code1 + (@op0.vfull shl 30) + ((@op0.vsize) shl 22) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_16b_etc_no_1d) & (@op0.type = @op1.type) & (@op2.type = @type_immc) & (@op2.data = 0) & code2 <> -1
	dd code2 + (@op0.vfull shl 30) + ((@op0.vsize) shl 22) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_16b_etc_no_1d) & (@op0.type = @op2.type) & (@op1.type = @type_immc) & (@op1.data = 0) & code3 <> -1
	dd code3 + (@op0.vfull shl 30) + ((@op0.vsize) shl 22) + (@op2.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_dn) & (@op0.type = @op1.type) & (@op0.type = @op2.type)
	dd code4 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_dn) & (@op0.type = @op1.type) & (@op2.type = @type_immc) & (@op2.data = 0) & code5 <> -1
	dd code5 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_dn) & (@op0.type = @op2.type) & (@op1.type = @type_immc) & (@op1.data = 0) & code6 <> -1
	dd code6 + (@op2.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro cmeq a*, b*, c*
    aarch64.h_cmeq_ect a, b, c, 0x2e208c00, 0x0e209800, 0x0e209800, 0x7ee08c00, 0x5ee09800, 0x5ee09800
end macro

macro cmhs a*, b*, c*
    aarch64.h_cmeq_ect a, b, c, 0x2e203c00,    -1     ,    -1	  , 0x7ee03c00,     -1	  ,	-1
end macro

macro cmge a*, b*, c*
    aarch64.h_cmeq_ect a, b, c, 0x0e203c00, 0x2e208800, 0x2e209800, 0x5ee03c00, 0x7ee08800, 0x7ee09800
end macro

macro cmhi a*, b*, c*
    aarch64.h_cmeq_ect a, b, c, 0x2e203400,    -1     ,    -1	  , 0x7ee03400,     -1	  ,	-1
end macro

macro cmgt a*, b*, c*
    aarch64.h_cmeq_ect a, b, c, 0x0e203400, 0x0e208800, 0x0e20a800, 0x5ee03400, 0x5ee08800, 0x5ee0a800
end macro

macro cmtst a*, b*, c*
    aarch64.h_cmeq_ect a, b, c, 0x0e208c00,    -1     ,    -1	  , 0x5ee08c00,     -1	  ,	-1
end macro

macro cmls a*, b*, c*
    cmhs a, c, b
end macro

macro cmle a*, b*, c*
    cmge a, c, b
end macro

macro cmlo a*, b*, c*
    cmhi a, c, b
end macro

macro cmlt a*, b*, c*
    cmgt a, c, b
end macro


macro aarch64.h_crc_ect a*, b*, c*, rtype*, code*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr) & (@op2.type and rtype)
	dd code + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro crc32b a*, b*, c*
    aarch64.h_crc_ect a, b, c, @type_wn_wzr, 0x1ac04000
end macro

macro crc32cb a*, b*, c*
    aarch64.h_crc_ect a, b, c, @type_wn_wzr, 0x1ac05000
end macro

macro crc32ch a*, b*, c*
    aarch64.h_crc_ect a, b, c, @type_wn_wzr, 0x1ac05400
end macro

macro crc32cw a*, b*, c*
    aarch64.h_crc_ect a, b, c, @type_wn_wzr, 0x1ac05800
end macro

macro crc32cx a*, b*, c*
    aarch64.h_crc_ect a, b, c, @type_xn_xzr, 0x9ac05c00
end macro

macro crc32h a*, b*, c*
    aarch64.h_crc_ect a, b, c, @type_wn_wzr, 0x1ac04400
end macro

macro crc32w a*, b*, c*
    aarch64.h_crc_ect a, b, c, @type_wn_wzr, 0x1ac04800
end macro

macro crc32x a*, b*, c*
    aarch64.h_crc_ect a, b, c, @type_xn_xzr, 0x9ac04c00
end macro

macro dup a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type and @type_vn_16b_etc_no_1d) & (@op1.type and @type_vn_bi_etc) & ((@op0.vsize) = @op1.vsize)
	dd 0x0e000400 + (@op0.vfull shl 30) + ((0x10000 + @op1.idx shl 17) shl (@op0.vsize)) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_8b_16b_4h_8h_2s_4s) & (@op1.type and @type_wn_wzr)
	dd 0x0e000c00 + (@op0.vfull shl 30) + ((0x10000) shl (@op0.vsize)) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_2d) & (@op1.type and @type_xn_xzr)
	dd 0x4e080c00 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_bn_hn_sn_dn) & (@op1.type and @type_vn_bi_etc) & ((@op0.vsize) = @op1.vsize)
	dd 0x5e000400 + ((0x10000 + @op1.idx shl 17) shl (@op0.vsize)) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro ext a*, b*, c*, d
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    aarch64.parse_operand @op3, d

    if (@op0.type and @type_vn_8b_16b) & (@op0.type = @op1.type) & (@op0.type = @op2.type) & (@op3.type = @type_immc)
	dd 0x2e000000 + (@op0.vfull shl 30) + (@op3.data shl 11) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro eon a*, b*, c*, d
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    aarch64.parse_shf		d
    if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr) & (@op2.type and @type_wn_wzr) & (@shf.success)
	dd 0x4a200000 + (@shf.kind shl 22) + (@op2.data shl 16) + (@shf.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr) & (@op2.type and @type_xn_xzr) & (@shf.success)
	dd 0xca200000 + (@shf.kind shl 22) + (@op2.data shl 16) + (@shf.shift shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro extr a*, b*, c*, d*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    aarch64.parse_operand @op3, d
    if (@op0.type and @type_wn_wzr) & (@op0.type and @type_wn_wzr) & (@op2.type and @type_wn_wzr) & (@op3.type and @type_immc)
	dd 0x13800000 + (@op2.data shl 16) + ((@op3.data and 0x1F) shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xzr) & (@op0.type and @type_xn_xzr) & (@op2.type and @type_xn_xzr) & (@op3.type and @type_immc)
	dd 0x93c00000 + (@op2.data shl 16) + ((@op3.data and 0x3F) shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operands'
    end if
end macro

macro madd a*, b*, c*, d*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    aarch64.parse_operand @op3, d
    if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr) & (@op2.type and @type_xn_xzr) & (@op3.type and @type_xn_xzr)
	dd 0x9b000000 + (@op2.data shl 16) + (@op3.data shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr) & (@op2.type and @type_wn_wzr) & (@op3.type and @type_wn_wzr)
	dd 0x1b000000 + (@op2.data shl 16) + (@op3.data shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro msub a*, b*, c*, d*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    aarch64.parse_operand @op3, d
    if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr) & (@op2.type and @type_xn_xzr) & (@op3.type and @type_xn_xzr)
	dd 0x9b008000 + (@op2.data shl 16) + (@op3.data shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr) & (@op2.type and @type_wn_wzr) & (@op3.type and @type_wn_wzr)
	dd 0x1b008000 + (@op2.data shl 16) + (@op3.data shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro


macro ins a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type = @type_vn_bi) & (@op1.type = @type_vn_bi)
	aarch64.restrict @op0.idx, 0, 15
	aarch64.restrict @op1.idx, 0, 15
	dd 0x6e010400 + (@op0.idx shl 17) + (@op1.idx shl 11) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_di) & (@op1.type = @type_vn_di)
	aarch64.restrict @op0.idx, 0, 1
	aarch64.restrict @op1.idx, 0, 1
	dd 0x6e080400 + (@op0.idx shl 20) + (@op1.idx shl 14) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_hi) & (@op1.type = @type_vn_hi)
	aarch64.restrict @op0.idx, 0, 7
	aarch64.restrict @op1.idx, 0, 7
	dd 0x6e020400 + (@op0.idx shl 18) + (@op1.idx shl 12) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_si) & (@op1.type = @type_vn_si)
	aarch64.restrict @op0.idx, 0, 3
	aarch64.restrict @op1.idx, 0, 3
	dd 0x6e040400 + (@op0.idx shl 19) + (@op1.idx shl 13) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_bi) & (@op1.type and @type_wn_wzr)
	aarch64.restrict @op0.idx, 0, 15
	dd 0x4e011c00 + (@op0.idx shl 17) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_di) & (@op1.type and @type_xn_xzr)
	aarch64.restrict @op0.idx, 0, 1
	dd 0x4e081c00 + (@op0.idx shl 20) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_hi) & (@op1.type and @type_wn_wzr)
	aarch64.restrict @op0.idx, 0, 7
	dd 0x4e021c00 + (@op0.idx shl 18) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_si) & (@op1.type and @type_wn_wzr)
	aarch64.restrict @op0.idx, 0, 3
	dd 0x4e041c00 + (@op0.idx shl 19) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro mla a*, b*, c*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if (@op0.type and @type_vn_8b_16b_4h_8h_2s_4s) & (@op0.type = @op1.type) & (@op0.type = @op2.type)
	dd 0x0e209400 + (@op0.vfull shl 30) + ((@op0.vsize) shl 22) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_2s_4s) & (@op0.type = @op1.type) & (@op2.type = @type_vn_si)
	dd 0x2f800000 + ((@op2.idx and 1) shl 21) + ((@op2.idx and 2) shl 10) + (@op0.vfull shl 30) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_4h_8h) & (@op0.type = @op1.type) & (@op2.type = @type_vn_hi)
	aarch64.restrict @op2.data, 0, 15
	dd 0x2f400000 + ((@op2.idx and 3) shl 20) + ((@op2.idx and 4) shl 9) + (@op0.vfull shl 30) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro mls a*, b*, c*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if (@op0.type and @type_vn_8b_16b_4h_8h_2s_4s) & (@op0.type = @op1.type) & (@op0.type = @op2.type)
	dd 0x2e209400 + (@op0.vfull shl 30) + ((@op0.vsize) shl 22) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_2s_4s) & (@op0.type = @op1.type) & (@op2.type = @type_vn_si)
	dd 0x2f804000 + ((@op2.idx and 1) shl 21) + ((@op2.idx and 2) shl 10) + (@op0.vfull shl 30) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_4h_8h) & (@op0.type = @op1.type) & (@op2.type = @type_vn_hi)
	aarch64.restrict @op2.data, 0, 15
	dd 0x2f404000 + ((@op2.idx and 3) shl 20) + ((@op2.idx and 4) shl 9) + (@op0.vfull shl 30) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro



macro mul a*, b*, c*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr) & (@op2.type and @type_wn_wzr)
	dd 0x1b007c00 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr) & (@op2.type and @type_xn_xzr)
	dd 0x9b007c00 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_8b_16b_4h_8h_2s_4s) & (@op0.type = @op1.type) & (@op0.type = @op2.type)
	dd 0x0e209c00 + (@op0.vfull shl 30) + ((@op0.vsize) shl 22) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_2s_4s) & (@op0.type = @op1.type) & (@op2.type = @type_vn_si)
	dd 0x0f808000 + ((@op2.idx and 1) shl 21) + ((@op2.idx and 2) shl 10) + (@op0.vfull shl 30) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_4h_8h) & (@op0.type = @op1.type) & (@op2.type = @type_vn_hi)
	aarch64.restrict @op2.data, 0, 15
	dd 0x0f408000 + ((@op2.idx and 3) shl 20) + ((@op2.idx and 4) shl 9) + (@op0.vfull shl 30) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro


macro neg b*, c*, d
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    aarch64.parse_shf		d
    if (@op1.type and @type_wn_wzr) & (@op2.type and @type_wn_wzr) & (@shf.success)
	dd 0x4b000000 + (@shf.kind shl 22) + (@op2.data shl 16) + (@shf.shift shl 10) + (31 shl 5) + (@op1.data shl 0)
    else if (@op1.type and @type_xn_xzr) & (@op2.type and @type_xn_xzr) & (@shf.success)
	dd 0xcb000000 + (@shf.kind shl 22) + (@op2.data shl 16) + (@shf.shift shl 10) + (31 shl 5) + (@op1.data shl 0)
    else if (@op1.type and @type_vn_16b_etc_no_1d) & (@op1.type = @op2.type) & (@shf.empty)
	dd 0x2e20b800 + (@op1.vfull shl 30) + ((@op1.vsize) shl 22) + (@op2.data shl 5) + (@op1.data shl 0)
    else if (@op1.type = @type_dn) & (@op1.type = @op2.type) & (@shf.empty)
	dd 0x7ee0b800 + (@op2.data shl 5) + (@op1.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro pmul a*, b*, c*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if (@op0.type = @type_vn_16b) & (@op1.type = @type_vn_16b) & (@op2.type = @type_vn_16b)
	dd 0x6e209c00 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_8b) & (@op1.type = @type_vn_8b) & (@op2.type = @type_vn_8b)
	dd 0x2e209c00 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro pmull a*, b*, c*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if (@op0.type = @type_vn_1q) & (@op1.type = @type_vn_1d) & (@op2.type = @type_vn_1d)
	dd 0x0ee0e000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_8h) & (@op1.type = @type_vn_8b) & (@op2.type = @type_vn_8b)
	dd 0x0e20e000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro pmull2 a*, b*, c*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if (@op0.type = @type_vn_1q) & (@op1.type = @type_vn_2d) & (@op2.type = @type_vn_2d)
	dd 0x4ee0e000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_8h) & (@op1.type = @type_vn_16b) & (@op2.type = @type_vn_16b)
	dd 0x4e20e000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro



macro rbit A*, B*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B    
    if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr)
	dd 0x5ac00000 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr)
	dd 0xdac00000 + (@op1.data shl 5) + (@op0.data shl 0)
    else if @op0.type and @type_vn_8b_16b & @op1.type = @op0.type
	dd 0x2e605800 + (@op0.vfull shl 30) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro rev A*, B*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr)
	dd 0x5ac00800 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr)
	dd 0xdac00c00 + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro rev16 A*, B*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B    
    if (@op0.type and @type_wn_wzr) & (@op1.type and @type_wn_wzr)
	dd 0x5ac00400 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr)
	dd 0xdac00400 + (@op1.data shl 5) + (@op0.data shl 0)
    else if @op0.type and @type_vn_8b_16b & @op1.type = @op0.type
	dd 0x0e201800 + (@op0.vfull shl 30) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro rev32 A*, B*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B    
    if (@op0.type and @type_xn_xzr) & (@op1.type and @type_xn_xzr)
	dd 0xdac00800 + (@op1.data shl 5) + (@op0.data shl 0)
    else if @op0.type and @type_vn_8b_16b_4h_8h & @op1.type = @op0.type
	dd 0x2e200800 + (@op0.vfull shl 30) + (@op0.vsize shl 22) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro rev64 A*, B*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B    
    if @op0.type and @type_vn_8b_16b_4h_8h_2s_4s & @op1.type = @op0.type
	dd 0x0e200800 + (@op0.vfull shl 30) + (@op0.vsize shl 22) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro


macro sha1h A*, B*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    if @op0.type = @type_sn & @op1.type = @type_sn
	dd 0x5e280800 + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro aarch64.h_sha1c A*, B*, C*, code*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if @op0.type = @type_qn & @op1.type = @type_sn & @op2.type = @type_vn_4s
	dd code + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro sha1c A*, B*, C*
    aarch64.h_sha1c A, B, C, 0x5e000000
end macro

macro sha1m A*, B*, C*
    aarch64.h_sha1c A, B, C, 0x5e002000
end macro

macro sha1p A*, B*, C*
    aarch64.h_sha1c A, B, C, 0x5e001000
end macro

macro aarch64.h_sha1su1 A*, B*, code*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    if @op0.type = @type_vn_4s & @op1.type = @type_vn_4s
	dd code + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro sha1su1 A*, B*
    aarch64.h_sha1su1 A, B, 0x5e281800
end macro

macro sha256su0 A*, B*
    aarch64.h_sha1su1 A, B, 0x5e282800
end macro

macro aarch64.h_sha1su0 A*, B*, C*, code*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if @op0.type = @type_vn_4s & @op1.type = @type_vn_4s & @op2.type = @type_vn_4s
	dd code + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro sha1su0 A*, B*, C*
    aarch64.h_sha1su0 A, B, C, 0x5e003000
end macro

macro sha256su1 A*, B*, C*
    aarch64.h_sha1su0 A, B, C, 0x5e006000
end macro

macro aarch64.h_sha256h A*, B*, C*, code*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if @op0.type = @type_qn & @op1.type = @type_qn & @op2.type = @type_vn_4s
	dd code + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro sha256h A*, B*, C*
    aarch64.h_sha256h A, B, C, 0x5e004000
end macro

macro sha256h2 A*, B*, C*
    aarch64.h_sha256h A, B, C, 0x5e005000
end macro
