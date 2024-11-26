
macro aarch64.h_uqshl A*, B*, C*, code1, code2, code3, code4
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if @op2.type = @type_immc
	if (@op0.type and @type_vn_16b_etc_no_1d) & (@op1.type = @op0.type)
	    aarch64.restrict @op2.data, 0, (8 shl @op0.vsize) - 1
	    dd code1 + (@op0.vfull shl 30) + (((8 shl @op0.vsize) + @op2.data) shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
	else if (@op0.type and @type_bn_hn_sn_dn) & (@op1.type = @op0.type)
	    aarch64.restrict @op2.data, 0, (8 shl @op0.vsize) - 1
	    dd code2 + (((8 shl @op0.vsize) + @op2.data) shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
	else
	    err 'invalid operand'
	end if
    else
	if (@op0.type and @type_vn_16b_etc_no_1d) & (@op1.type = @op0.type) & (@op2.type = @op0.type)
	    dd code3 + (@op0.vfull shl 30) + (@op0.vsize shl 22) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
	else if (@op0.type and @type_bn_hn_sn_dn) & (@op1.type = @op0.type) & (@op2.type = @op0.type)
	    dd code4 + (@op0.vsize shl 22) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
	else
	    err 'invalid operand'
	end if
    end if
end macro

macro uqshl A*, B*, C*
    aarch64.h_uqshl A, B, C, 0x2f007400, 0x7f007400, 0x2e204c00, 0x7e204c00
end macro

macro sqshl A*, B*, C*
    aarch64.h_uqshl A, B, C, 0x0f007400, 0x5f007400, 0x0e204c00, 0x5e204c00
end macro

macro sqshlu A*, B*, C*   ; sqshlu must have immediate, so cause an overflow with register
    aarch64.h_uqshl A, B, C, 0x2f006400, 0x7f006400, 0x100000000, 0x100000000
end macro

macro aarch64.h_sri A*, B*, C*, inv, code1*, code2*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if @op2.type = @type_immc & @op1.type = @op0.type
	if @op0.type and @type_vn_16b_etc_no_1d
	    if inv
		aarch64.restrict @op2.data, 1, (8 shl @op0.vsize)
		dd code1 + (@op0.vfull shl 30) + (((16 shl @op0.vsize) - @op2.data) shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
	    else
		aarch64.restrict @op2.data, 0, (8 shl @op0.vsize) - 1
		dd code1 + (@op0.vfull shl 30) + (((8 shl @op0.vsize) + @op2.data) shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
	    end if
	else if @op0.type = @type_dn
	    if inv
		aarch64.restrict @op2.data, 1, 64
		dd code2 + ((64 - @op2.data) shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
	    else
		aarch64.restrict @op2.data, 0, 63
		dd code2 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
	    end if
	else
	    err 'invalid operand'
	end if
    else
	err 'invalid operand'
    end if
end macro

macro sri A*, B*, C*
    aarch64.h_sri A, B, C, 1, 0x2f004400, 0x7f404400
end macro

macro srshr A*, B*, C*
    aarch64.h_sri A, B, C, 1, 0x0f002400, 0x5f402400
end macro

macro srsra A*, B*, C*
    aarch64.h_sri A, B, C, 1, 0x0f003400, 0x5f403400
end macro

macro sshr A*, B*, C*
    aarch64.h_sri A, B, C, 1, 0x0f000400, 0x5f400400
end macro

macro ssra A*, B*, C*
    aarch64.h_sri A, B, C, 1, 0x0f001400, 0x5f401400
end macro

macro urshr A*, B*, C*
    aarch64.h_sri A, B, C, 1, 0x2f002400, 0x7f402400
end macro

macro ursra A*, B*, C*
    aarch64.h_sri A, B, C, 1, 0x2f003400, 0x7f403400
end macro

macro ushr A*, B*, C*
    aarch64.h_sri A, B, C, 1, 0x2f000400, 0x7f400400
end macro

macro usra A*, B*, C*
    aarch64.h_sri A, B, C, 1, 0x2f001400, 0x7f401400
end macro

macro shl A*, B*, C*
    aarch64.h_sri A, B, C, 0, 0x0f005400, 0x5f405400
end macro

macro sli A*, B*, C*
    aarch64.h_sri A, B, C, 0, 0x2f005400, 0x7f405400
end macro


macro aarch64.h_sqabs A*, B*, code1*, code2*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    if @op0.type and @type_vn_16b_etc_no_1d & @op1.type = @op0.type
	dd code1 + (@op0.vfull shl 30) + (@op0.vsize shl 22) + (@op1.data shl 5) + (@op0.data shl 0)
    else if @op0.type and @type_bn_hn_sn_dn & @op1.type = @op0.type
	dd code2 + (@op0.vsize shl 22) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro sqabs A*, B*
    aarch64.h_sqabs A, B, 0x0e207800, 0x5e207800
end macro

macro suqadd A*, B*
    aarch64.h_sqabs A, B, 0x0e203800, 0x5e203800
end macro

macro usqadd A*, B*
    aarch64.h_sqabs A, B, 0x2e203800, 0x7e203800
end macro

macro sqneg A*, B*
    aarch64.h_sqabs A, B, 0x2e207800, 0x7e207800
end macro


macro aarch64.h_smaxv A*, B*, code*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    if (@op0.type and @type_bn_hn_sn) & (@op1.type and @type_vn_8b_16b_4h_8h_4s) & (@op0.vsize = @op1.vsize)
	dd code + (@op1.vfull shl 30) + (@op1.vsize shl 22) + (@op1.data shl 5) + (@op0.data shl 0)
    end if
end macro

macro smaxv A*, B*
    aarch64.h_smaxv A, B, 0x0e30a800
end macro

macro sminv A*, B*
    aarch64.h_smaxv A, B, 0x0e31a800
end macro

macro umaxv A*, B*
    aarch64.h_smaxv A, B, 0x2e30a800
end macro

macro uminv A*, B*
    aarch64.h_smaxv A, B, 0x2e31a800
end macro


macro aarch64.h_uaba A*, B*, C*, code
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if @op0.type and @type_vn_16b_etc_no_1d & @op0.type = @op1.type & @op0.type = @op2.type
	dd code + (@op0.vfull shl 30) + (@op0.vsize shl 22) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro uaba A*, B*, C*
    aarch64.h_uaba A, B, C, 0x2e207c00
end macro

macro uabd A*, B*, C*
    aarch64.h_uaba A, B, C, 0x2e207400
end macro

macro shadd A*, B*, C*
    aarch64.h_uaba A, B, C, 0x0e200400
end macro

macro shsub A*, B*, C*
    aarch64.h_uaba A, B, C, 0x0e202400
end macro

macro smax A*, B*, C*
    aarch64.h_uaba A, B, C, 0x0e206400
end macro

macro smaxp A*, B*, C*
    aarch64.h_uaba A, B, C, 0x0e20a400
end macro

macro smin A*, B*, C*
    aarch64.h_uaba A, B, C, 0x0e206c00
end macro

macro sminp A*, B*, C*
    aarch64.h_uaba A, B, C, 0x0e20ac00
end macro

macro srhadd A*, B*, C*
    aarch64.h_uaba A, B, C, 0x0e201400
end macro

macro saba A*, B*, C*
    aarch64.h_uaba A, B, C, 0x0e207c00
end macro

macro sabd A*, B*, C*
    aarch64.h_uaba A, B, C, 0x0e207400
end macro

macro uhadd A*, B*, C*
    aarch64.h_uaba A, B, C, 0x2e200400
end macro

macro uhsub A*, B*, C*
    aarch64.h_uaba A, B, C, 0x2e202400
end macro

macro umax A*, B*, C*
    aarch64.h_uaba A, B, C, 0x2e206400
end macro

macro umaxp A*, B*, C*
    aarch64.h_uaba A, B, C, 0x2e20a400
end macro

macro urhadd A*, B*, C*
    aarch64.h_uaba A, B, C, 0x2e201400
end macro

macro umin A*, B*, C*
    aarch64.h_uaba A, B, C, 0x2e206c00
end macro

macro uminp A*, B*, C*
    aarch64.h_uaba A, B, C, 0x2e20ac00
end macro


macro aarch64.h_saddw A*, B*, C*, code
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if @op0.type = @type_vn_2d & @op1.type = @type_vn_2d & @op2.type = @type_vn_2s
	dd code + 0x00800000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if @op0.type = @type_vn_4s & @op1.type = @type_vn_4s & @op2.type = @type_vn_4h
	dd code + 0x00400000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if @op0.type = @type_vn_8h & @op1.type = @type_vn_8h & @op2.type = @type_vn_8b
	dd code + 0x00000000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro saddw A*, B*, C*
    aarch64.h_saddw A, B, C, 0x0e201000
end macro

macro ssubw A*, B*, C*
    aarch64.h_saddw A, B, C, 0x0e203000
end macro

macro uaddw A*, B*, C*
    aarch64.h_saddw A, B, C, 0x2e201000
end macro

macro usubw A*, B*, C*
    aarch64.h_saddw A, B, C, 0x2e203000
end macro


macro aarch64.h_saddw2 A*, B*, C*, code
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if @op0.type = @type_vn_2d & @op1.type = @type_vn_2d & @op2.type = @type_vn_4s
	dd code + 0x00800000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if @op0.type = @type_vn_4s & @op1.type = @type_vn_4s & @op2.type = @type_vn_8h
	dd code + 0x00400000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if @op0.type = @type_vn_8h & @op1.type = @type_vn_8h & @op2.type = @type_vn_16b
	dd code + 0x00000000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro saddw2 A*, B*, C*
    aarch64.h_saddw2 A, B, C, 0x4e201000
end macro

macro ssubw2 A*, B*, C*
    aarch64.h_saddw2 A, B, C, 0x4e203000
end macro

macro uaddw2 A*, B*, C*
    aarch64.h_saddw2 A, B, C, 0x6e201000
end macro

macro usubw2 A*, B*, C*
    aarch64.h_saddw2 A, B, C, 0x6e203000
end macro


macro aarch64.h_srshl A*, B*, C*, code1, code2
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if (@op0.type and @type_vn_16b_etc_no_1d) & (@op1.type = @op0.type) & (@op2.type = @op0.type)
	dd code1 + (@op0.vfull shl 30) + (@op0.vsize shl 22) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_dn) & (@op1.type = @op0.type) & (@op2.type = @op0.type)
	dd code2 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    end if
end macro

macro srshl A*, B*, C*
    aarch64.h_srshl A, B, C, 0x0e205400, 0x5ee05400
end macro

macro sshl A*, B*, C*
    aarch64.h_srshl A, B, C, 0x0e204400, 0x5ee04400
end macro

macro urshl A*, B*, C*
    aarch64.h_srshl A, B, C, 0x2e205400, 0x7ee05400
end macro

macro ushl A*, B*, C*
    aarch64.h_srshl A, B, C, 0x2e204400, 0x7ee04400
end macro


macro aarch64.h_sqadd A*, B*, C*, code1, code2
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if (@op0.type and @type_vn_16b_etc_no_1d) & (@op1.type = @op0.type) & (@op2.type = @op0.type)
	dd code1 + (@op0.vfull shl 30) + (@op0.vsize shl 22) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_bn_hn_sn_dn) & (@op1.type = @op0.type) & (@op2.type = @op0.type)
	dd code2 + (@op0.vsize shl 22) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    end if
end macro

macro sqadd A*, B*, C*
    aarch64.h_sqadd A, B, C, 0x0e200c00, 0x5e200c00
end macro

macro sqrshl A*, B*, C*
    aarch64.h_sqadd A, B, C, 0x0e205c00, 0x5e205c00
end macro

macro sqsub A*, B*, C*
    aarch64.h_sqadd A, B, C, 0x0e202c00, 0x5e202c00
end macro

macro uqadd A*, B*, C*
    aarch64.h_sqadd A, B, C, 0x2e200c00, 0x7e200c00
end macro

macro uqrshl A*, B*, C*
    aarch64.h_sqadd A, B, C, 0x2e205c00, 0x7e205c00
end macro

macro uqsub A*, B*, C*
    aarch64.h_sqadd A, B, C, 0x2e202c00, 0x7e202c00
end macro
; below instructions do not have scalar version, so cause an overflow on scalar 
macro trn1 A*, B*, C*
    aarch64.h_sqadd A, B, C, 0x0e002800, 0x100000000
end macro

macro trn2 A*, B*, C*
    aarch64.h_sqadd A, B, C, 0x0e006800, 0x100000000
end macro

macro uzp1 A*, B*, C*
    aarch64.h_sqadd A, B, C, 0x0e001800, 0x100000000
end macro

macro uzp2 A*, B*, C*
    aarch64.h_sqadd A, B, C, 0x0e005800, 0x100000000
end macro

macro zip1 A*, B*, C*
    aarch64.h_sqadd A, B, C, 0x0e003800, 0x100000000
end macro

macro zip2 A*, B*, C*
    aarch64.h_sqadd A, B, C, 0x0e007800, 0x100000000
end macro


macro aarch64.h_subhn A*, B*, C*, code*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if @op0.type = @type_vn_2s & @op1.type = @type_vn_2d & @op2.type = @type_vn_2d
	dd code + 0x00800000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if @op0.type = @type_vn_4h & @op1.type = @type_vn_4s & @op2.type = @type_vn_4s
	dd code + 0x00400000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if @op0.type = @type_vn_8b & @op1.type = @type_vn_8h & @op2.type = @type_vn_8h 
	dd code + 0x00000000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro subhn A*, B*, C*
    aarch64.h_subhn A, B, C, 0x0e206000
end macro

macro rsubhn A*, B*, C*
    aarch64.h_subhn A, B, C, 0x2e206000
end macro

macro raddhn A*, B*, C*
    aarch64.h_subhn A, B, C, 0x2e204000
end macro


macro aarch64.h_subhn2 A*, B*, C*, code*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if @op0.type = @type_vn_4s & @op1.type = @type_vn_2d & @op2.type = @type_vn_2d
	dd code + 0x00800000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if @op0.type = @type_vn_8h & @op1.type = @type_vn_4s & @op2.type = @type_vn_4s
	dd code + 0x00400000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if @op0.type = @type_vn_16b & @op1.type = @type_vn_8h & @op2.type = @type_vn_8h 
	dd code + 0x00000000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro subhn2 A*, B*, C*
    aarch64.h_subhn2 A, B, C, 0x4e206000
end macro

macro rsubhn2 A*, B*, C*
    aarch64.h_subhn2 A, B, C, 0x6e206000
end macro

macro raddhn2 A*, B*, C*
    aarch64.h_subhn2 A, B, C, 0x6e204000
end macro


macro aarch64.h_sabal A*, B*, C*, code*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if @op0.type = @type_vn_2d & @op1.type = @type_vn_2s & @op2.type = @type_vn_2s
	dd code + 0x00800000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if @op0.type = @type_vn_4s & @op1.type = @type_vn_4h & @op2.type = @type_vn_4h
	dd code + 0x00400000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if @op0.type = @type_vn_8h & @op1.type = @type_vn_8b & @op2.type = @type_vn_8b 
	dd code + 0x00000000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro sabal A*, B*, C*
    aarch64.h_sabal A, B, C, 0x0e205000   
end macro

macro sabdl A*, B*, C*
    aarch64.h_sabal A, B, C, 0x0e207000   
end macro

macro saddl A*, B*, C*
    aarch64.h_sabal A, B, C, 0x0e200000   
end macro

macro ssubl A*, B*, C*
    aarch64.h_sabal A, B, C, 0x0e202000  
end macro

macro uabal A*, B*, C*
    aarch64.h_sabal A, B, C, 0x2e205000   
end macro

macro uabdl A*, B*, C*
    aarch64.h_sabal A, B, C, 0x2e207000   
end macro

macro uaddl A*, B*, C*
    aarch64.h_sabal A, B, C, 0x2e200000   
end macro

macro usubl A*, B*, C*
    aarch64.h_sabal A, B, C, 0x2e202000  
end macro


macro aarch64.h_sabal2 A*, B*, C*, code*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if @op0.type = @type_vn_2d & @op1.type = @type_vn_4s & @op2.type = @type_vn_4s
	dd code + 0x00800000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if @op0.type = @type_vn_4s & @op1.type = @type_vn_8h & @op2.type = @type_vn_8h
	dd code + 0x00400000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if @op0.type = @type_vn_8h & @op1.type = @type_vn_16b & @op2.type = @type_vn_16b 
	dd code + 0x00000000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro sabal2 A*, B*, C*
    aarch64.h_sabal2 A, B, C, 0x4e205000   
end macro

macro sabdl2 A*, B*, C*
    aarch64.h_sabal2 A, B, C, 0x4e207000   
end macro

macro saddl2 A*, B*, C*
    aarch64.h_sabal2 A, B, C, 0x4e200000   
end macro

macro ssubl2 A*, B*, C*
    aarch64.h_sabal2 A, B, C, 0x4e202000  
end macro

macro uabal2 A*, B*, C*
    aarch64.h_sabal2 A, B, C, 0x6e205000   
end macro

macro uabdl2 A*, B*, C*
    aarch64.h_sabal2 A, B, C, 0x6e207000   
end macro

macro uaddl2 A*, B*, C*
    aarch64.h_sabal2 A, B, C, 0x6e200000   
end macro

macro usubl2 A*, B*, C*
    aarch64.h_sabal2 A, B, C, 0x6e202000  
end macro


macro aarch64.h_sadalp A*, B*, code*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    if @op1.type and @type_vn_8b_16b_4h_8h_2s_4s & @op0.type = 2*@op1.type
	dd code + (@op1.vfull shl 30) + (@op1.vsize shl 22) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro sadalp A*, B*
    aarch64.h_sadalp A, B, 0x0e206800  
end macro

macro saddlp A*, B*
    aarch64.h_sadalp A, B, 0x0e202800	
end macro

macro uadalp A*, B*
    aarch64.h_sadalp A, B, 0x2e206800	
end macro

macro uaddlp A*, B*
    aarch64.h_sadalp A, B, 0x2e202800  
end macro

macro aarch64.h_uaddlv A*, B*, code
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    if @op0.type = @type_dn & @op1.type = @type_vn_4s
	dd code + 0x40800000 + (@op1.data shl 5) + (@op0.data shl 0)
    else if @op0.type = @type_hn & @op1.type = @type_vn_16b
	dd code + 0x40000000 + (@op1.data shl 5) + (@op0.data shl 0)
    else if @op0.type = @type_hn & @op1.type = @type_vn_8b
	dd code + 0x00000000 + (@op1.data shl 5) + (@op0.data shl 0)
    else if @op0.type = @type_sn & @op1.type = @type_vn_4h
	dd code + 0x00400000 + (@op1.data shl 5) + (@op0.data shl 0)
    else if @op0.type = @type_sn & @op1.type = @type_vn_8h
	dd code + 0x40400000 + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro uaddlv A*, B*
    aarch64.h_uaddlv A, B, 0x2e303800
end macro

macro saddlv A*, B*
    aarch64.h_uaddlv A, B, 0x0e303800
end macro

macro xtn A*, B*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    if @op0.type and @type_vn_8b_4h_2s & @op1.type = 32*@op0.type
	dd 0x0e212800 + (@op0.vsize shl 22) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro



macro aarch64.h_smull A*, B*, C*, code1*, code2*, code3*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if (@op0.type and @type_xn_xzr) & (@op1.type and @type_wn_wzr) & (@op2.type and @type_wn_wzr)
	dd code1 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_2d) & (@op1.type = @type_vn_2s) & (@op2.type = @type_vn_si)
	dd code2 + 0x00800000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0) + ((@op2.idx and 1) shl 21) + ((@op2.idx and 2) shl 10)
    else if (@op0.type = @type_vn_4s) & (@op1.type = @type_vn_4h) & (@op2.type = @type_vn_hi)
	aarch64.restrict @op2.data, 0, 15
	dd code2 + 0x00400000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0) + ((@op2.idx and 3) shl 20) + ((@op2.idx and 4) shl 9)
    else if (@op0.type = @type_vn_2d) & (@op1.type = @type_vn_2s) & (@op2.type = @type_vn_2s)
	dd code3 + 0x00800000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_4s) & (@op1.type = @type_vn_4h) & (@op2.type = @type_vn_4h)
	dd code3 + 0x00400000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_8h) & (@op1.type = @type_vn_8b) & (@op2.type = @type_vn_8b)
	dd code3 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro smull A*, B*, C*
    aarch64.h_smull A, B, C,  0x9b207c00, 0x0f00a000, 0x0e20c000
end macro

macro umull A*, B*, C*
    aarch64.h_smull A, B, C,  0x9ba07c00, 0x2f00a000, 0x2e20c000
end macro

macro smlal A*, B*, C*	; this and following dont operand on gpr, so overflow in that case
    aarch64.h_smull A, B, C, 0x100000000, 0x0f002000, 0x0e208000
end macro

macro smlsl A*, B*, C*
    aarch64.h_smull A, B, C, 0x100000000, 0x0f006000, 0x0e20a000
end macro

macro umlal A*, B*, C*
    aarch64.h_smull A, B, C, 0x100000000, 0x2f002000, 0x2e208000
end macro

macro umlsl A*, B*, C*
    aarch64.h_smull A, B, C, 0x100000000, 0x2f006000, 0x2e20a000
end macro


macro aarch64.h_sqdmlal A*, B*, C*, code2*, code3*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if (@op0.type = @type_vn_2d) & (@op1.type = @type_vn_2s) & (@op2.type = @type_vn_si)
	dd code2 + 0x00800000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0) + ((@op2.idx and 1) shl 21) + ((@op2.idx and 2) shl 10)
    else if (@op0.type = @type_vn_4s) & (@op1.type = @type_vn_4h) & (@op2.type = @type_vn_hi)
	aarch64.restrict @op2.data, 0, 15
	dd code2 + 0x00400000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0) + ((@op2.idx and 3) shl 20) + ((@op2.idx and 4) shl 9)
    else if (@op0.type = @type_vn_2d) & (@op1.type = @type_vn_2s) & (@op2.type = @type_vn_2s)
	dd code3 + 0x00800000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_4s) & (@op1.type = @type_vn_4h) & (@op2.type = @type_vn_4h)
	dd code3 + 0x00400000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro sqdmlal A*, B*, C*
    aarch64.h_sqdmlal A, B, C, 0x0f003000, 0x0e209000
end macro

macro sqdmlsl A*, B*, C*
    aarch64.h_sqdmlal A, B, C, 0x0f007000, 0x0e20b000
end macro

macro sqdmull A*, B*, C*
    aarch64.h_sqdmlal A, B, C, 0x0f00b000, 0x0e20d000
end macro


macro aarch64.h_sqdmlal2 A*, B*, C*, code2*, code3*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if (@op0.type = @type_vn_2d) & (@op1.type = @type_vn_4s) & (@op2.type = @type_vn_si)
	dd code2 + 0x00800000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0) + ((@op2.idx and 1) shl 21) + ((@op2.idx and 2) shl 10)
    else if (@op0.type = @type_vn_4s) & (@op1.type = @type_vn_8h) & (@op2.type = @type_vn_hi)
	aarch64.restrict @op2.data, 0, 15
	dd code2 + 0x00400000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0) + ((@op2.idx and 3) shl 20) + ((@op2.idx and 4) shl 9)
    else if (@op0.type = @type_vn_2d) & (@op1.type = @type_vn_4s) & (@op2.type = @type_vn_4s)
	dd code3 + 0x00800000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_4s) & (@op1.type = @type_vn_8h) & (@op2.type = @type_vn_8h)
	dd code3 + 0x00400000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro sqdmlal2 A*, B*, C*
    aarch64.h_sqdmlal2 A, B, C, 0x4f003000, 0x4e209000
end macro

macro sqdmlsl2 A*, B*, C*
    aarch64.h_sqdmlal2 A, B, C, 0x4f007000, 0x4e20b000
end macro

macro sqdmull2 A*, B*, C*
    aarch64.h_sqdmlal2 A, B, C, 0x4f00b000, 0x4e20d000
end macro



macro aarch64.h_umull2 A*, B*, C*, code2*, code3*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if (@op0.type = @type_vn_2d) & (@op1.type = @type_vn_4s) & (@op2.type = @type_vn_si)
	dd code2 + 0x00800000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0) + ((@op2.idx and 1) shl 21) + ((@op2.idx and 2) shl 10)
    else if (@op0.type = @type_vn_4s) & (@op1.type = @type_vn_8h) & (@op2.type = @type_vn_hi)
	aarch64.restrict @op2.data, 0, 15
	dd code2 + 0x00400000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0) + ((@op2.idx and 3) shl 20) + ((@op2.idx and 4) shl 9)
    else if (@op0.type = @type_vn_2d) & (@op1.type = @type_vn_4s) & (@op2.type = @type_vn_4s)
	dd code3 + 0x00800000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_4s) & (@op1.type = @type_vn_8h) & (@op2.type = @type_vn_8h)
	dd code3 + 0x00400000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_vn_8h) & (@op1.type = @type_vn_16b) & (@op2.type = @type_vn_16b)
	dd code3 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro umull2 A*, B*, C*
    aarch64.h_umull2 A, B, C, (0x6fbfa841-0x00bf0841), (0x6ea3c041-0x00830041)
end macro

macro umlsl2 A*, B*, C*
    aarch64.h_umull2 A, B, C, (0x6fbf6841-0x00bf0841), (0x6ea3a041-0x00830041)
end macro

macro umlal2 A*, B*, C*
    aarch64.h_umull2 A, B, C, (0x6fbf2841-0x00bf0841), (0x6ea38041-0x00830041)
end macro

macro smull2 A*, B*, C*
    aarch64.h_umull2 A, B, C, (0x4fbfa841-0x00bf0841), (0x4ea3c041-0x00830041)
end macro

macro smlal2 A*, B*, C*
    aarch64.h_umull2 A, B, C, (0x4fbf2841-0x00bf0841), (0x4ea38041-0x00830041)
end macro

macro smlsl2 A*, B*, C*
    aarch64.h_umull2 A, B, C, (0x4fbf6841-0x00bf0841), (0x4ea3a041-0x00830041)
end macro


macro aarch64.h_sqdmulh A*, B*, C*, code1*, code2*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if (@op0.type = @type_sn) & (@op1.type = @op0.type) & (@op2.type = @op0.type)
	dd code1 + 0x50800000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_2s_4s) & (@op1.type = @op0.type) & (@op2.type = @op0.type)
	dd code1 + 0x00800000 + (@op0.vfull shl 30) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_hn) & (@op1.type = @op0.type) & (@op2.type = @op0.type)
	dd code1 + 0x50400000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_4h_8h) & (@op1.type = @op0.type) & (@op2.type = @op0.type)
	dd code1 + 0x00400000 + (@op0.vfull shl 30) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type = @type_sn) & (@op1.type = @op0.type) & (@op2.type = @type_vn_si)
	dd code2 + 0x50800000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0) + ((@op2.idx and 1) shl 21) + ((@op2.idx and 2) shl 10)
    else if (@op0.type and @type_vn_2s_4s) & (@op1.type = @op0.type) & (@op2.type = @type_vn_si)
	dd code2 + 0x00800000 + (@op0.vfull shl 30) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0) + ((@op2.idx and 1) shl 21) + ((@op2.idx and 2) shl 10)
    else if (@op0.type = @type_hn) & (@op1.type = @type_hn) & (@op2.type = @type_vn_hi)
	aarch64.restrict @op2.data, 0, 15
	dd code2 + 0x50400000 + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0) + ((@op2.idx and 3) shl 20) + ((@op2.idx and 4) shl 9)
    else if (@op0.type and @type_vn_4h_8h) & (@op1.type = @op0.type) & (@op2.type = @type_vn_hi)
	aarch64.restrict @op2.data, 0, 15
	dd code2 + 0x00400000 + (@op0.vfull shl 30) + (@op2.data shl 16) + (@op1.data shl 5) + (@op0.data shl 0) + ((@op2.idx and 3) shl 20) + ((@op2.idx and 4) shl 9)
    else
	err 'invalid operand'
    end if
end macro

macro sqdmulh A*, B*, C*
    aarch64.h_sqdmulh A, B, C, 0x0e20b400, 0x0f00c000
end macro

macro sqrdmulh A*, B*, C*
    aarch64.h_sqdmulh A, B, C, 0x2e20b400, 0x0f00d000
end macro




macro aarch64.h_sqxtn A*, B*, code1*, code2*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    if (@op0.type and @type_bn_hn_sn) & (@op1.type = 2*@op0.type)
	dd code1 + (@op0.vsize shl 22) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_8b_4h_2s) & (@op1.type = 32*@op0.type)
	dd code2 + (@op0.vsize shl 22) + (@op1.data shl 5) + (@op0.data shl 0)
    end if    
end macro

macro sqxtn A*, B*
    aarch64.h_sqxtn A, B, 0x5e214800, 0x0e214800
end macro

macro sqxtun A*, B*
    aarch64.h_sqxtn A, B, 0x7e212800, 0x2e212800
end macro

macro uqxtn A*, B*
    aarch64.h_sqxtn A, B, 0x7e214800, 0x2e214800
end macro


macro aarch64.h_sqshrun A*, B*, C*, code1*, code2*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if (@op0.type and @type_bn_hn_sn) & (@op1.type = 2*@op0.type) & (@op2.type = @type_immc)
	aarch64.restrict @op2.data, 1, (8 shl @op0.vsize)
	dd code1 + (((16 shl @op0.vsize) - @op2.data) shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else if (@op0.type and @type_vn_8b_4h_2s) & (@op1.type = 32*@op0.type) & (@op2.type = @type_immc)
	aarch64.restrict @op2.data, 1, (8 shl @op0.vsize)
	dd code2 + (((16 shl @op0.vsize) - @op2.data) shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    end if    
end macro

macro sqshrun A*, B*, C*
    aarch64.h_sqshrun A, B, C, 0x7f008400, 0x2f008400
end macro

macro uqrshrn A*, B*, C*
    aarch64.h_sqshrun A, B, C, 0x7f009c00, 0x2f009c00
end macro

macro sqrshrn A*, B*, C*
    aarch64.h_sqshrun A, B, C, 0x5f009c00, 0x0f009c00
end macro

macro sqrshrun A*, B*, C*
    aarch64.h_sqshrun A, B, C, 0x7f008c00, 0x2f008c00
end macro

macro sqshrn A*, B*, C*
    aarch64.h_sqshrun A, B, C, 0x5f009400, 0x0f009400
end macro

macro uqshrn A*, B*, C*
    aarch64.h_sqshrun A, B, C, 0x7f009400, 0x2f009400
end macro


macro aarch64.h_rshrn2 A*, B*, C*, code*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if (@op0.type and @type_vn_16b_8h_4s) & (@op1.type = 2*@op0.type) & (@op2.type = @type_immc)
	aarch64.restrict @op2.data, 1, (8 shl @op0.vsize)
	dd code + (((16 shl @op0.vsize) - @op2.data) shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    end if    
end macro

macro rshrn2 A*, B*, C*
    aarch64.h_rshrn2 A, B, C, 0x4f008c00
end macro

macro sqrshrn2 A*, B*, C*
    aarch64.h_rshrn2 A, B, C, 0x4f009c00
end macro

macro sqrshrun2 A*, B*, C*
    aarch64.h_rshrn2 A, B, C, 0x6f008c00
end macro

macro shrn2 A*, B*, C*
    aarch64.h_rshrn2 A, B, C, 0x4f008400
end macro

macro sqshrn2 A*, B*, C*
    aarch64.h_rshrn2 A, B, C, 0x4f009400
end macro

macro sqshrun2 A*, B*, C*
    aarch64.h_rshrn2 A, B, C, 0x6f008400
end macro

macro uqrshrn2 A*, B*, C*
    aarch64.h_rshrn2 A, B, C, 0x6f009c00
end macro

macro uqshrn2 A*, B*, C*
    aarch64.h_rshrn2 A, B, C, 0x6f009400
end macro


macro shll A*, B*, C*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if @op1.type and @type_vn_8b_4h_2s & @op0.type = 32*@op1.type & @op2.type = @type_immc
	aarch64.restrict @op2.data, (8 shl @op1.vsize), (8 shl @op1.vsize)
	dd 0x2e213800 + (@op1.vsize shl 22) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro aarch64.h_sshll A*, B*, C*, code*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if (@op1.type and @type_vn_8b_4h_2s) & (@op0.type = 32*@op1.type) & (@op2.type = @type_immc)
	aarch64.restrict @op2.data, 0, (8 shl @op1.vsize) - 1
	dd code + (((8 shl @op1.vsize) + @op2.data) shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if    
end macro

macro sshll A*, B*, C*
    aarch64.h_sshll A, B, C, 0x0f00a400
end macro

macro sxtl A*, B*
    aarch64.h_sshll A, B, 0, 0x0f00a400
end macro

macro ushll A*, B*, C*
    aarch64.h_sshll A, B, C, 0x2f00a400
end macro

macro uxtl A*, B*
    aarch64.h_sshll A, B, 0, 0x2f00a400
end macro


macro shll2 A*, B*, C*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if @op1.type and @type_vn_16b_8h_4s & @op0.type = 2*@op1.type & @op2.type = @type_immc
	aarch64.restrict @op2.data, (8 shl @op1.vsize), (8 shl @op1.vsize)
	dd 0x6e213800 + (@op1.vsize shl 22) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if
end macro

macro aarch64.h_sshll2 A*, B*, C*, code*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if (@op1.type and @type_vn_16b_8h_4s) & (@op0.type = 2*@op1.type) & (@op2.type = @type_immc)
	aarch64.restrict @op2.data, 0, (8 shl @op1.vsize) - 1
	dd code + (((8 shl @op1.vsize) + @op2.data) shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if    
end macro

macro sshll2 A*, B*, C*
    aarch64.h_sshll2 A, B, C, 0x4f00a400
end macro

macro sxtl2 A*, B*
    aarch64.h_sshll2 A, B, 0, 0x4f00a400
end macro

macro ushll2 A*, B*, C*
    aarch64.h_sshll2 A, B, C, 0x6f00a400
end macro

macro uxtl2 A*, B*
    aarch64.h_sshll2 A, B, 0, 0x6f00a400
end macro


macro aarch64.h_rshrn A*, B*, C*, code*
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    aarch64.parse_operand @op2, C
    if (@op0.type and @type_vn_8b_4h_2s) & (@op1.type = 32*@op0.type) & (@op2.type = @type_immc)
	aarch64.restrict @op2.data, 1, (8 shl @op0.vsize)
	dd code + (((16 shl @op0.vsize) - @op2.data) shl 16) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if    
end macro

macro rshrn A*, B*, C*
    aarch64.h_rshrn A, B, C, 0x0f008c00
end macro

macro shrn A*, B*, C*
    aarch64.h_rshrn A, B, C, 0x0f008400
end macro



macro aarch64.h_uqxtn2 A*, B*, code
    aarch64.parse_operand @op0, A
    aarch64.parse_operand @op1, B
    if (@op0.type and @type_vn_16b_8h_4s) & (@op1.type = 2*@op0.type)
	dd code + (@op0.vsize shl 22) + (@op1.data shl 5) + (@op0.data shl 0)
    else
	err 'invalid operand'
    end if    
end macro

macro uqxtn2 A*, B*
    aarch64.h_uqxtn2 A, B, 0x6e214800
end macro

macro xtn2 A*, B*
    aarch64.h_uqxtn2 A, B, 0x4e212800
end macro

macro sqxtn2 A*, B*
    aarch64.h_uqxtn2 A, B, 0x4e214800
end macro

macro sqxtun2 A*, B*
    aarch64.h_uqxtn2 A, B, 0x6e212800
end macro
