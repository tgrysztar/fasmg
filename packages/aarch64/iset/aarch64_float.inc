macro aarch64.h_float_f2 a*, b*, code1*, code2*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if @op0.type = @type_dn
	@adr.encode = 0x00400000 + code1
    else if @op0.type = @type_sn
	@adr.encode = 0x00000000 + code1
    else if @op0.type = @type_vn_2d
	@adr.encode = 0x40400000 + code2
    else if @op0.type = @type_vn_2s
	@adr.encode = 0x00000000 + code2
    else if @op0.type = @type_vn_4s
	@adr.encode = 0x40000000 + code2
    else
	err 'invalid operand'
    end if
    if @op1.type = @op0.type
	dd @adr.encode + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro fabs a*, b*
    aarch64.h_float_f2 a, b, 0x1e20c000, 0x0ea0f800
end macro

macro fneg a*, b*
    aarch64.h_float_f2 a, b, 0x1e214000, 0x2ea0f800
end macro

macro frecpe a*, b*
    aarch64.h_float_f2 a, b, 0x5ea1d800, 0x0ea1d800
end macro

macro frecpx a*, b*
    aarch64.h_float_f2 a, b, 0x5ea1f800, -1
end macro

macro frinta a*, b*
    aarch64.h_float_f2 a, b, 0x1e264000, 0x2e218800
end macro

macro frinti a*, b*
    aarch64.h_float_f2 a, b, 0x1e27c000, 0x2ea19800
end macro

macro frintm a*, b*
    aarch64.h_float_f2 a, b, 0x1e254000, 0x0e219800
end macro

macro frintn a*, b*
    aarch64.h_float_f2 a, b, 0x1e244000, 0x0e218800
end macro

macro frintp a*, b*
    aarch64.h_float_f2 a, b, 0x1e24c000, 0x0ea18800
end macro

macro frintx a*, b*
    aarch64.h_float_f2 a, b, 0x1e274000, 0x2e219800
end macro

macro frintz a*, b*
    aarch64.h_float_f2 a, b, 0x1e25c000, 0x0ea19800
end macro

macro frsqrte a*, b*
    aarch64.h_float_f2 a, b, 0x7ea1d800, 0x2ea1d800
end macro

macro fsqrt a*, b*
    aarch64.h_float_f2 a, b, 0x1e21c000, 0x2ea1f800
end macro


macro aarch64.h_float_f2a a*, b*, c, code1*, code2*, code3*, code4*, code5*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if (@op0.type = @type_dn) & (@op1.type = @type_vn_2d) & (@op2.type = @type_empty)
	dd code1 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_sn) & (@op1.type = @type_vn_2s) & (@op2.type = @type_empty)
	dd code2 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_2d) & (@op1.type = @type_vn_2d) & (@op2.type = @type_vn_2d)
	dd code3 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_2s) & (@op1.type = @type_vn_2s) & (@op2.type = @type_vn_2s)
	dd code4 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_4s) & (@op1.type = @type_vn_4s) & (@op2.type = @type_vn_4s)
	dd code5 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro faddp a*, b*, c
    aarch64.h_float_f2a a, b, c, 0x7e70d800, 0x7e30d800, 0x6e60d400, 0x2e20d400, 0x6e20d400
end macro

macro fmaxnmp a*, b*, c
    aarch64.h_float_f2a a, b, c, 0x7e70c800, 0x7e30c800, 0x6e60c400, 0x2e20c400, 0x6e20c400
end macro

macro fmaxp a*, b*, c
    aarch64.h_float_f2a a, b, c, 0x7e70f800, 0x7e30f800, 0x6e60f400, 0x2e20f400, 0x6e20f400
end macro

macro fminnmp a*, b*, c
    aarch64.h_float_f2a a, b, c, 0x7ef0c800, 0x7eb0c800, 0x6ee0c400, 0x2ea0c400, 0x6ea0c400
end macro

macro fminp a*, b*, c
    aarch64.h_float_f2a a, b, c, 0x7ef0f800, 0x7eb0f800, 0x6ee0f400, 0x2ea0f400, 0x6ea0f400
end macro



macro aarch64.h_float_f3 a*, b*, c*, code1*, code2*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if @op0.type = @type_dn
	@adr.encode = 0x00400000 + code1
    else if @op0.type = @type_sn
	@adr.encode = 0x00000000 + code1
    else if @op0.type = @type_vn_2d
	@adr.encode = 0x40400000 + code2
    else if @op0.type = @type_vn_2s
	@adr.encode = 0x00000000 + code2
    else if @op0.type = @type_vn_4s
	@adr.encode = 0x40000000 + code2
    else
	err 'invalid operand'
    end if
    if (@op1.type = @op0.type) & (@op2.type = @op0.type)
	dd @adr.encode + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro frecps a*, b*, c*
    aarch64.h_float_f3 a, b, c, 0x5e20fc00, 0x0e20fc00
end macro

macro fabd a*, b*, c*
    aarch64.h_float_f3 a, b, c, 0x7ea0d400, 0x2ea0d400
end macro

macro facge a*, b*, c*
    aarch64.h_float_f3 a, b, c, 0x7e20ec00, 0x2e20ec00
end macro

macro facgt a*, b*, c*
    aarch64.h_float_f3 a, b, c, 0x7ea0ec00, 0x2ea0ec00
end macro

macro fadd a*, b*, c*
    aarch64.h_float_f3 a, b, c, 0x1e202800, 0x0e20d400
end macro

macro frsqrts a*, b*, c*
    aarch64.h_float_f3 a, b, c, 0x5ea0fc00, 0x0ea0fc00
end macro

macro fsub a*, b*, c*
    aarch64.h_float_f3 a, b, c, 0x1e203800, 0x0ea0d400
end macro

macro fdiv a*, b*, c*
    aarch64.h_float_f3 a, b, c, 0x1e201800, 0x2e20fc00
end macro

macro fmax a*, b*, c*
    aarch64.h_float_f3 a, b, c, 0x1e204800, 0x0e20f400
end macro

macro fmaxnm a*, b*, c*
    aarch64.h_float_f3 a, b, c, 0x1e206800, 0x0e20c400
end macro

macro fmin a*, b*, c*
    aarch64.h_float_f3 a, b, c, 0x1e205800, 0x0ea0f400
end macro

macro fminnm a*, b*, c*
    aarch64.h_float_f3 a, b, c, 0x1e207800, 0x0ea0c400
end macro


macro aarch64.h_float_f4 a*, b*, c*, d*, code
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    aarch64.parse_operand @op3, d
    if @op0.type = @type_dn
	@adr.encode = 0x00400000 + code
    else if @op0.type = @type_sn
	@adr.encode = 0x00000000 + code
    else
	err 'invalid operand'
    end if
    if (@op1.type = @op0.type) & (@op2.type = @op0.type) & (@op3.type = @op0.type)
	dd @adr.encode + (@op2.data shl 16) + (@op3.data shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro fmadd a*, b*, c*, d*
    aarch64.h_float_f4 a, b, c, d, 0x1f000000
end macro

macro fmsub a*, b*, c*, d*
    aarch64.h_float_f4 a, b, c, d, 0x1f008000
end macro

macro fnmsub a*, b*, c*, d*
    aarch64.h_float_f4 a, b, c, d, 0x1f208000
end macro

macro fnmadd a*, b*, c*, d*
    aarch64.h_float_f4 a, b, c, d, 0x1f200000
end macro



macro fmaxnmv a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type = @type_sn) & (@op1.type = @type_vn_4s)
	dd 0x6e30c800 + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if    
end macro

macro fmaxv a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type = @type_sn) & (@op1.type = @type_vn_4s)
	dd 0x6e30f800 + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if    
end macro

macro fminnmv a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type = @type_sn) & (@op1.type = @type_vn_4s)
	dd 0x6eb0c800 + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if    
end macro

macro fminv a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type = @type_sn) & (@op1.type = @type_vn_4s)
	dd 0x6eb0f800 + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if    
end macro


macro fnmul a*, b*, c*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if (@op0.type = @type_dn) & (@op1.type = @type_dn) & (@op2.type = @type_dn)
	dd 0x1e608800 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_sn) & (@op1.type = @type_sn) & (@op2.type = @type_sn)
	dd 0x1e208800 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro


macro fccmp a*, b*, c*, d*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    aarch64.parse_con		d
    if (@op0.type and @type_dn) & (@op1.type and @type_dn) & (@op2.type and @type_immc) & (@con.success)
	dd 0x1e600400 + (@con.kind shl 12) + (@op1.data shl 16) + (@op0.data shl 5) + ((@op2.data and 0x0F) shl 0)
    else if (@op0.type and @type_sn) & (@op1.type and @type_sn) & (@op2.type and @type_immc) & (@con.success)
	dd 0x1e200400 + (@con.kind shl 12) + (@op1.data shl 16) + (@op0.data shl 5) + ((@op2.data and 0x0F) shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro fccmpe a*, b*, c*, d*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    aarch64.parse_con		d
    if (@op0.type and @type_dn) & (@op1.type and @type_dn) & (@op2.type and @type_immc) & (@con.success)
	dd 0x1e600410 + (@con.kind shl 12) + (@op1.data shl 16) + (@op0.data shl 5) + ((@op2.data and 0x0F) shl 0)
    else if (@op0.type and @type_sn) & (@op1.type and @type_sn) & (@op2.type and @type_immc) & (@con.success)
	dd 0x1e200410 + (@con.kind shl 12) + (@op1.data shl 16) + (@op0.data shl 5) + ((@op2.data and 0x0F) shl 0)
    else
	err 'invalid operand'
    end if
end macro


macro aarch64.h_fcm a*, b*, c*, code1, code2, code3
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if @op0.type = @type_dn
	@adr.encode = 0x50400000
    else if @op0.type = @type_sn
	@adr.encode = 0x50000000
    else if @op0.type = @type_vn_2d
	@adr.encode = 0x40400000
    else if @op0.type = @type_vn_2s
	@adr.encode = 0x00000000
    else if @op0.type = @type_vn_4s
	@adr.encode = 0x40000000
    else
	err 'invalid operand'
    end if
    if (@op1.type = @op0.type) & (@op2.type = @op0.type)
	dd code1 + @adr.encode + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op1.type = @op0.type) & (@op2.type = @type_immc) & (@op2.data = 0)
	dd code2 + @adr.encode			    + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op2.type = @op0.type) & (@op1.type = @type_immc) & (@op1.data = 0)
	dd code3 + @adr.encode			    + (@op2.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro fcmeq a*, b*, c*
    aarch64.h_fcm a, b, c, 0x0e20e400, 0x0ea0d800, 0x0ea0d800
end macro

macro fcmge a*, b*, c*
    aarch64.h_fcm a, b, c, 0x2e20e400, 0x2ea0c800, 0x2ea0d800
end macro

macro fcmle a*, b*, c*
    aarch64.h_fcm a, c, b, 0x2e20e400, 0x2ea0c800, 0x2ea0d800
end macro

macro fcmgt a*, b*, c*
    aarch64.h_fcm a, b, c, 0x2ea0e400, 0x0ea0c800, 0x0ea0e800
end macro

macro fcmlt a*, b*, c*
    aarch64.h_fcm a, c, b, 0x2ea0e400, 0x0ea0c800, 0x0ea0e800
end macro


macro fcsel a*, b*, c*, d*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    aarch64.parse_con		d
    if (@op0.type = @type_sn) & (@op1.type = @type_sn) & (@op2.type = @type_sn) & (@con.success)
	dd 0x1e200c00 + ((@con.kind) shl 12) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_dn) & (@op1.type = @type_dn) & (@op2.type = @type_dn) & (@con.success)
	dd 0x1e600c00 + (@con.kind shl 12) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro fcmp a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type = @type_dn) & (@op1.type = @type_dn)
	dd 0x1e602000 + (@op1.data shl 16) + (@op0.data shl 5)
    else if (@op0.type = @type_dn) & (@op1.type = @type_immc) & (@op1.data = 0)
	dd 0x1e602008			   + (@op0.data shl 5)
    else if (@op0.type = @type_sn) & (@op1.type = @type_sn)
	dd 0x1e202000 + (@op1.data shl 16) + (@op0.data shl 5)
    else if (@op0.type = @type_sn) & (@op1.type = @type_immc) & (@op1.data = 0)
	dd 0x1e202008			   + (@op0.data shl 5)
    else
	err 'invalid operands'
    end if
end macro

macro fcmpe a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type = @type_dn) & (@op1.type = @type_dn)
	dd 0x1e602010 + (@op1.data shl 16) + (@op0.data shl 5)
    else if (@op0.type = @type_dn) & (@op1.type = @type_immc) & (@op1.data = 0)
	dd 0x1e602018			   + (@op0.data shl 5)
    else if (@op0.type = @type_sn) & (@op1.type = @type_sn)
	dd 0x1e202010 + (@op1.data shl 16) + (@op0.data shl 5)
    else if (@op0.type = @type_sn) & (@op1.type = @type_immc) & (@op1.data = 0)
	dd 0x1e202018			   + (@op0.data shl 5)
    else
	err 'invalid operands'
    end if
end macro


macro fcvtl a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type = @type_vn_2d) & (@op1.type = @type_vn_2s)
	dd 0x0e617800 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_4s) & (@op1.type = @type_vn_4h)
	dd 0x0e217800 + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operands'
    end if
end macro

macro fcvtl2 a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type = @type_vn_2d) & (@op1.type = @type_vn_4s)
	dd 0x4e617800 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_4s) & (@op1.type = @type_vn_8h)
	dd 0x4e217800 + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operands'
    end if
end macro

macro fcvtn a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type = @type_vn_2s) & (@op1.type = @type_vn_2d)
	dd 0x0e616800 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_4h) & (@op1.type = @type_vn_4s)
	dd 0x0e216800 + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operands'
    end if
end macro

macro fcvtn2 a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type = @type_vn_4s) & (@op1.type = @type_vn_2d)
	dd 0x4e616800 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_8h) & (@op1.type = @type_vn_4s)
	dd 0x4e216800 + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operands'
    end if
end macro

macro fcvtxn a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type = @type_sn) & (@op1.type = @type_dn)
	dd 0x7e616800 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_2s) & (@op1.type = @type_vn_2d)
	dd 0x2e616800 + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operands'
    end if
end macro

macro fcvtxn2 a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type = @type_vn_4s) & (@op1.type = @type_vn_2d)
	dd 0x6e616800 + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operands'
    end if
end macro


macro fcvt a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type = @type_dn) & (@op1.type = @type_hn)
	dd 0x1ee2c000 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_dn) & (@op1.type = @type_sn)
	dd 0x1e22c000 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_hn) & (@op1.type = @type_dn)
	dd 0x1e63c000 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_hn) & (@op1.type = @type_sn)
	dd 0x1e23c000 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_sn) & (@op1.type = @type_dn)
	dd 0x1e624000 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_sn) & (@op1.type = @type_hn)
	dd 0x1ee24000 + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro


macro aarch64.h_fcvt a*, b*, c*, reverse*, code1*, code2*, code3, code4
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if reverse
	@imm.data = @op0.type
	@op0.type = @op1.type
	@op1.type = @imm.data
    end if
    if @op2.type <> @type_immc
	err 'invalid operand'
    end if
    @imm.data = ((-@op2.data) and 63)
    if @op2.data = 0
	@adr.encode1 = code1
	@adr.encode2 = code2
    else
	@adr.encode1 = code3
	@adr.encode2 = code4
    end if
    @adr.encode1 = @adr.encode1 + (@imm.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    @adr.encode2 = @adr.encode2 + (@imm.data shl 10) + (@op1.data shl 5) + (@op0.data shl 0)
    if (@op0.type = @type_dn) & (@op1.type = @type_dn)
	aarch64.restrict @op2.data, 0, 64
	dd @adr.encode1 + 0x50400000
    else if (@op0.type = @type_sn) & (@op1.type = @type_sn)
	aarch64.restrict @op2.data, 0, 32
	dd @adr.encode1 + 0x50000000
    else if (@op0.type = @type_vn_2d) & (@op1.type = @type_vn_2d)
	aarch64.restrict @op2.data, 0, 64
	dd @adr.encode1 + 0x40400000
    else if (@op0.type = @type_vn_2s) & (@op1.type = @type_vn_2s)
	aarch64.restrict @op2.data, 0, 32
	dd @adr.encode1 + 0x00000000
    else if (@op0.type = @type_vn_4s) & (@op1.type = @type_vn_4s)
	aarch64.restrict @op2.data, 0, 32
	dd @adr.encode1 + 0x40000000
    else if (@op0.type and @type_wn_wzr) & (@op1.type = @type_dn)
	aarch64.restrict @op2.data, 0, 32
	dd @adr.encode2 + 0x00400000
    else if (@op0.type and @type_wn_wzr) & (@op1.type = @type_sn)
	aarch64.restrict @op2.data, 0, 32
	dd @adr.encode2 + 0x00000000
    else if (@op0.type and @type_xn_xzr) & (@op1.type = @type_dn)
	aarch64.restrict @op2.data, 0, 64
	dd @adr.encode2 + 0x80400000
    else if (@op0.type and @type_xn_xzr) & (@op1.type = @type_sn)
	aarch64.restrict @op2.data, 0, 64
	dd @adr.encode2 + 0x80000000
    else
	err 'invalid operand'
    end if
end macro

macro fcvtas a*, b*
    aarch64.h_fcvt a, b, 0, 0, 0x0e21c800, 0x1e240000
end macro

macro fcvtau a*, b*
    aarch64.h_fcvt a, b, 0, 0, 0x2e21c800, 0x1e250000
end macro

macro fcvtms a*, b*
    aarch64.h_fcvt a, b, 0, 0, 0x0e21b800, 0x1e300000
end macro

macro fcvtmu a*, b*
    aarch64.h_fcvt a, b, 0, 0, 0x2e21b800, 0x1e310000
end macro

macro fcvtns a*, b*
    aarch64.h_fcvt a, b, 0, 0, 0x0e21a800, 0x1e200000
end macro

macro fcvtnu a*, b*
    aarch64.h_fcvt a, b, 0, 0, 0x2e21a800, 0x1e210000
end macro

macro fcvtps a*, b*
    aarch64.h_fcvt a, b, 0, 0, 0x0ea1a800, 0x1e280000
end macro

macro fcvtpu a*, b*
    aarch64.h_fcvt a, b, 0, 0, 0x2ea1a800, 0x1e290000
end macro

macro fcvtzs a*, b*, c:0
    aarch64.h_fcvt a, b, c, 0, 0x0ea1b800, 0x1e380000, 0x0f00fc00, 0x1e180000
end macro

macro fcvtzu a*, b*, c:0
    aarch64.h_fcvt a, b, c, 0, 0x2ea1b800, 0x1e390000, 0x2f00fc00, 0x1e190000
end macro

macro scvtf a*, b*, c:0
    aarch64.h_fcvt a, b, c, 1, 0x0e21d800, 0x1e220000, 0x0f00e400, 0x1e020000
end macro

macro ucvtf a*, b*, c:0
    aarch64.h_fcvt a, b, c, 1, 0x2e21d800, 0x1e230000, 0x2f00e400, 0x1e030000
end macro


macro fmul a*, b*, c*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if (@op0.type = @type_dn) & (@op1.type = @type_dn) & (@op2.type = @type_dn)
	dd 0x1e600800 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_dn) & (@op1.type = @type_dn) & (@op2.type = @type_vn_di)
	dd 0x5fc09000 + ((@op2.idx and 1) shl 11) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_sn) & (@op1.type = @type_sn) & (@op2.type = @type_sn)
	dd 0x1e200800 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_sn) & (@op1.type = @type_sn) & (@op2.type = @type_vn_si)
	dd 0x5f809000 + ((@op2.idx and 1) shl 21) + ((@op2.idx and 2) shl 10) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_2d) & (@op1.type = @type_vn_2d) & (@op2.type = @type_vn_2d)
	dd 0x6e60dc00 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_2d) & (@op1.type = @type_vn_2d) & (@op2.type = @type_vn_di)
	dd 0x4fc09000 + ((@op2.idx and 1) shl 11) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_2s) & (@op1.type = @type_vn_2s) & (@op2.type = @type_vn_2s)
	dd 0x2e20dc00 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_2s) & (@op1.type = @type_vn_2s) & (@op2.type = @type_vn_si)
	dd 0x0f809000 + ((@op2.idx and 1) shl 21) + ((@op2.idx and 2) shl 10) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_4s) & (@op1.type = @type_vn_4s) & (@op2.type = @type_vn_4s)
	dd 0x6e20dc00 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_4s) & (@op1.type = @type_vn_4s) & (@op2.type = @type_vn_si)
	dd 0x4f809000 + ((@op2.idx and 1) shl 21) + ((@op2.idx and 2) shl 10) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro fmulx a*, b*, c*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if (@op0.type = @type_dn) & (@op1.type = @type_dn) & (@op2.type = @type_dn)
	dd 0x5e60dc00 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_dn) & (@op1.type = @type_dn) & (@op2.type = @type_vn_di)
	dd 0x7fc09000 + ((@op2.idx and 1) shl 11) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_sn) & (@op1.type = @type_sn) & (@op2.type = @type_sn)
	dd 0x5e20dc00 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_sn) & (@op1.type = @type_sn) & (@op2.type = @type_vn_si)
	dd 0x7f809000 + ((@op2.idx and 1) shl 21) + ((@op2.idx and 2) shl 10) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_2d) & (@op1.type = @type_vn_2d) & (@op2.type = @type_vn_2d)
	dd 0x4e60dc00 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_2d) & (@op1.type = @type_vn_2d) & (@op2.type = @type_vn_di)
	dd 0x6fc09000 + ((@op2.idx and 1) shl 11) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_2s) & (@op1.type = @type_vn_2s) & (@op2.type = @type_vn_2s)
	dd 0x0e20dc00 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_2s) & (@op1.type = @type_vn_2s) & (@op2.type = @type_vn_si)
	dd 0x2f809000 + ((@op2.idx and 1) shl 21) + ((@op2.idx and 2) shl 10) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_4s) & (@op1.type = @type_vn_4s) & (@op2.type = @type_vn_4s)
	dd 0x4e20dc00 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_4s) & (@op1.type = @type_vn_4s) & (@op2.type = @type_vn_si)
	dd 0x6f809000 + ((@op2.idx and 1) shl 21) + ((@op2.idx and 2) shl 10) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro fmla a*, b*, c*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if (@op0.type = @type_dn) & (@op1.type = @type_dn) & (@op2.type = @type_vn_di)
	dd 0x5fc01000 + ((@op2.idx and 1) shl 11) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_sn) & (@op1.type = @type_sn) & (@op2.type = @type_vn_si)
	dd 0x5f801000 + ((@op2.idx and 1) shl 21) + ((@op2.idx and 2) shl 10) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_2d) & (@op1.type = @type_vn_2d) & (@op2.type = @type_vn_2d)
	dd 0x4e60cc00 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_2d) & (@op1.type = @type_vn_2d) & (@op2.type = @type_vn_di)
	dd 0x4fc01000 + ((@op2.idx and 1) shl 11) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_2s) & (@op1.type = @type_vn_2s) & (@op2.type = @type_vn_2s)
	dd 0x0e20cc00 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_2s) & (@op1.type = @type_vn_2s) & (@op2.type = @type_vn_si)
	dd 0x0f801000 + ((@op2.idx and 1) shl 21) + ((@op2.idx and 2) shl 10) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_4s) & (@op1.type = @type_vn_4s) & (@op2.type = @type_vn_4s)
	dd 0x4e20cc00 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_4s) & (@op1.type = @type_vn_4s) & (@op2.type = @type_vn_si)
	dd 0x4f801000 + ((@op2.idx and 1) shl 21) + ((@op2.idx and 2) shl 10) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro fmls a*, b*, c*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    aarch64.parse_operand @op2, c
    if (@op0.type = @type_dn) & (@op1.type = @type_dn) & (@op2.type = @type_vn_di)
	dd 0x5fc05000 + ((@op2.idx and 1) shl 11) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_sn) & (@op1.type = @type_sn) & (@op2.type = @type_vn_si)
	dd 0x5f805000 + ((@op2.idx and 1) shl 21) + ((@op2.idx and 2) shl 10) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_2d) & (@op1.type = @type_vn_2d) & (@op2.type = @type_vn_2d)
	dd 0x4ee0cc00 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_2d) & (@op1.type = @type_vn_2d) & (@op2.type = @type_vn_di)
	dd 0x4fc05000 + ((@op2.idx and 1) shl 11) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_2s) & (@op1.type = @type_vn_2s) & (@op2.type = @type_vn_2s)
	dd 0x0ea0cc00 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_2s) & (@op1.type = @type_vn_2s) & (@op2.type = @type_vn_si)
	dd 0x0f805000 + ((@op2.idx and 1) shl 21) + ((@op2.idx and 2) shl 10) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_4s) & (@op1.type = @type_vn_4s) & (@op2.type = @type_vn_4s)
	dd 0x4ea0cc00 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_4s) & (@op1.type = @type_vn_4s) & (@op2.type = @type_vn_si)
	dd 0x4f805000 + ((@op2.idx and 1) shl 21) + ((@op2.idx and 2) shl 10) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro


macro fmov a*, b*
    aarch64.parse_operand @op0, a
    aarch64.parse_operand @op1, b
    if (@op0.type and @type_sn) & (@op1.type and @type_sn)
	dd 0x1e204000 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_dn) & (@op1.type and @type_dn)
	dd 0x1e604000 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_wn_wzr) & (@op1.type and @type_sn)
	dd 0x1e260000 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_sn) & (@op1.type and @type_wn_wzr)
	dd 0x1e270000 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_dn)
	dd 0x9e660000 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_dn) & (@op1.type and @type_xn_xzr)
	dd 0x9e670000 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_xn_xzr) & (@op1.type and @type_vn_di) & (@op1.idx = 1)
	dd 0x9eae0000 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_di) & (@op0.idx = 1) & (@op1.type and @type_xn_xzr)
	dd 0x9eaf0000 + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_dn) & (@op1.type and @type_immf)
	dd 0x1e601000 + (@op1.data shl 13) + (@op0.data shl 0)
    else if (@op0.type and @type_sn) & (@op1.type and @type_immf)
	dd 0x1e201000 + (@op1.data shl 13) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_2s) & (@op1.type and @type_immf)
	dd 0x0f00f400 + ((@op1.data and 0xE0) shl 11) + ((@op1.data and 0x1F) shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_4s) & (@op1.type and @type_immf)
	dd 0x4f00f400 + ((@op1.data and 0xE0) shl 11) + ((@op1.data and 0x1F) shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_2d) & (@op1.type and @type_immf)
	dd 0x6f00f400 + ((@op1.data and 0xE0) shl 11) + ((@op1.data and 0x1F) shl 5) + (@op0.data shl 0)
    else
	err 'invalid operands'
    end if
end macro
