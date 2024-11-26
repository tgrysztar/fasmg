macro nop
    dd 0xd503201f
end macro

macro wfe a&
    dd 0xd503205f
end macro

macro wfi a&
    dd 0xd503207f
end macro

macro yield a&
    dd 0xd503203f
end macro

macro tlbi a*, b:xzr
    aarch64.parse_operand @op0, b
    match =alle1?, a
	dd 0xd50c8780 + (@op0.data shl 0)
    else match =aside1?, a
	dd 0xd5088740 + (@op0.data shl 0)
    else
	err 'invalid operand'
    end match
end macro

macro sev
    dd 0xd503209f
end macro

macro sevl
    dd 0xd50320bf
end macro

macro mrs a*, b*
    aarch64.parse_operand @op0, a
    match =actlr_el1?, b
	dd 0xd5381020 + (@op0.data shl 0)
    else match =vttbr_el2?, b
	dd 0xd53c2100 + (@op0.data shl 0)
    else match =spsel?, b
	dd 0xd5384200 + (@op0.data shl 0)
    else match =s3_2_c3_c4_5?, b
	dd 0xd53a34a0 + (@op0.data shl 0)
    else
	err 'invalid operand'
    end match
end macro

namespace aarch64.mrs
    actlr_el1? = 0xd5381020
    vttbr_el2? = 0xd53c2100
    spsel? = 0xd5384200
    s3_2_c3_c4_5? = 0xd53a34a0
    nzcv? = 0xd53b4200
end namespace

macro mrs a*, b*
    aarch64.parse_operand @op0, a
    if defined aarch64.mrs.b
       dd aarch64.mrs.b + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro


macro msr a*, b*
    aarch64.parse_operand @op0, b
    match =actlr_el1?, a
	dd 0xd5181020 + (@op0.data shl 0)
    else match =vttbr_el2?, a
	dd 0xd51c2100 + (@op0.data shl 0)
    else match =spsel?, a
	dd 0xd5184200 + (@op0.data shl 0)
    else match =s3_2_c3_c4_5?, a
	dd 0xd51a34a0 + (@op0.data shl 0)
    else
	err 'invalid operand'
    end match
end macro

macro aarch64.parse_reg_cn ns, A
    match =c0?, A
	ns.data = 0
    else match =c1?, A
	ns.data = 1
    else match =c2?, A
	ns.data = 2
    else match =c3?, A
	ns.data = 3
    else match =c4?, A
	ns.data = 4
    else match =c5?, A
	ns.data = 5
    else match =c6?, A
	ns.data = 6
    else match =c7?, A
	ns.data = 7
    else match =c8?, A
	ns.data = 8
    else match =c9?, A
	ns.data = 9
    else match =c10?, A
	ns.data = 10
    else match =c11?, A
	ns.data = 11
    else match =c12?, A
	ns.data = 12
    else match =c13?, A
	ns.data = 13
    else match =c14?, A
	ns.data = 14
    else match =c15?, A
	ns.data = 15
    else
	err 'invalid oeprand'
    end match
end macro

macro sys A*, B*, C*, D*, E:xzr
    aarch64.parse_operand @op0, A
    aarch64.parse_reg_cn  @op1, B
    aarch64.parse_reg_cn  @op2, C
    aarch64.parse_operand @op3, D
    aarch64.parse_operand @opA, E
    if (@op0.type = @type_immc) & (@op3.type = @type_immc) & (@opA.type and @type_xn_xzr)
	dd 0xd5080000 + (@op0.data shl 16) + (@op1.data shl 12) + (@op2.data shl 8) + (@op3.data shl 5) + (@opA.data shl 0)
    else 
	err 'invalid operand'
    end if
end macro

macro sysl E*, A*, B*, C*, D*
    aarch64.parse_operand @op0, A
    aarch64.parse_reg_cn  @op1, B
    aarch64.parse_reg_cn  @op2, C
    aarch64.parse_operand @op3, D
    aarch64.parse_operand @opA, E
    if (@op0.type = @type_immc) & (@op3.type = @type_immc) & (@opA.type and @type_xn_xzr)
	dd 0xd5280000 + (@op0.data shl 16) + (@op1.data shl 12) + (@op2.data shl 8) + (@op3.data shl 5) + (@opA.data shl 0)
    else 
	err 'invalid operand'
    end if
end macro


macro hint a&
    aarch64.parse_operand @op0, a
    if (@op0.type and @type_immc)
	aarch64.restrict @op0.data, 0, 127
	dd 0xd503201f + (@op0.data shl 5)
    else
	err 'invalid operand'
    end if
end macro

macro eret
    dd 0xd69f03e0
end macro


macro dc a, b
    aarch64.parse_operand @op0, b
    if (@op0.type and @type_xn_xzr)
	match =cisw?, a
	    dd 0xd5087e40 + @op0.data
	else match =civac?, a
	    dd 0xd50b7e20 + @op0.data
	else match =csw?, a
	    dd 0xd5087a40 + @op0.data
	else match =cvac?, a
	    dd 0xd50b7a20 + @op0.data
	else match =cvau?, a
	    dd 0xd50b7b20 + @op0.data
	else match =isw?, a
	    dd 0xd5087640 + @op0.data
	else match =ivac?, a
	    dd 0xd5087620 + @op0.data
	else match =zva?, a
	    dd 0xd50b7420 + @op0.data
	else
	    err 'invalid operand'
	end match
    else
	err 'invalid operand'
    end if
end macro

macro ic a, b:xzr
    aarch64.parse_operand @op0, b
    if (@op0.type and @type_xn_xzr)
	match =iallu?, a
	    dd 0xd5087500 + @op0.data
	else match =ialluis?, a
	    dd 0xd5087100 + @op0.data
	else match =ivau?, a
	    dd 0xd50b7520 + @op0.data
	else
	    err 'invalid operand'
	end match
    else
	err 'invalid operand'
    end if
end macro

macro isb a&
    dd 0xd5033fdf
end macro

macro dmb a&
    match =ish?, a
	dd 0xd5033bbf
    else match =ishld?, a
	dd 0xd50339bf
    else match =ishst?, a
	dd 0xd5033abf
    else match =ld?, a
	dd 0xd5033dbf
    else match =nsh?, a
	dd 0xd50337bf
    else match =nshld?, a
	dd 0xd50335bf
    else match =nshst?, a
	dd 0xd50336bf
    else match =osh?, a
	dd 0xd50333bf
    else match =oshld?, a
	dd 0xd50331bf
    else match =oshst?, a
	dd 0xd50332bf
    else match =st?, a
	dd 0xd5033ebf
    else match =sy?, a
	dd 0xd5033fbf
    else
	err 'invalid operand'
    end match
end macro

macro drps
    dd 0xd6bf03e0
end macro

macro dsb a&
    match =ish?, a
	dd 0xd5033b9f
    else match =ishld?, a
	dd 0xd503399f
    else match =ishst?, a
	dd 0xd5033a9f
    else match =ld?, a
	dd 0xd5033d9f
    else match =nsh?, a
	dd 0xd503379f
    else match =nshld?, a
	dd 0xd503359f
    else match =nshst?, a
	dd 0xd503369f
    else match =osh?, a
	dd 0xd503339f
    else match =oshld?, a
	dd 0xd503319f
    else match =oshst?, a
	dd 0xd503329f
    else match =st?, a
	dd 0xd5033e9f
    else match =sy?, a
	dd 0xd5033f9f
    else
	err 'invalid operand'
    end match
end macro

iterate <instr, code>, dcps1, 0xd4a00001, dcps2, 0xd4a00002, dcps3, 0xd4a00003,\
		       hlt, 0xd4400000, hvc, 0xd4000002, smc, 0xd4000003
    macro instr a:0
	aarch64.parse_operand @op0, a
	if (@op0.type and @type_immc)
	    aarch64.restrict @op0.data, 0, 0xFFFF
	    dd code + (@op0.data shl 5)
	else
	    err 'invalid operand'
	end if
    end macro
end iterate

macro clrex a:15
    aarch64.parse_operand @op0, a
    if (@op0.type and @type_immc)
	aarch64.restrict @op0.data, 0, 0xF
	dd 0xd503305f + (@op0.data shl 8)
    else
	err 'invalid operand'
    end if
end macro


macro at A*, B*
    aarch64.parse_operand @op1, B
    if @op1.type and @type_xn_xzr
	match =s12e0r, A
	    dd 0xd50c78c0 + @op1.data
	else match =s12e0w, A
	    dd 0xd50c78e0 + @op1.data
	else match =s12e1r, A
	    dd 0xd50c7880 + @op1.data
	else match =s12e1w, A
	    dd 0xd50c78a0 + @op1.data
	else match =s1e0r, A
	    dd 0xd5087840 + @op1.data
	else match =s1e0w, A
	    dd 0xd5087860 + @op1.data
	else match =s1e1r, A
	    dd 0xd5087800 + @op1.data
	else match =s1e1w, A
	    dd 0xd5087820 + @op1.data
	else match =s1e2r, A
	    dd 0xd50c7800 + @op1.data
	else match =s1e2w, A
	    dd 0xd50c7820 + @op1.data
	else match =s1e3r, A
	    dd 0xd50e7800 + @op1.data
	else match =s1e3w, A
	    dd 0xd50e7820 + @op1.data
	else
	    err 'invalid operand'
	end match
    else
	err 'invalid operand'
    end if
end macro
