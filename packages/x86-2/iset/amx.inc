
define AMX AMX

element AMX.reg

repeat 8, i:0
	element tmm#i? : AMX.reg + i
end repeat

iterate context, @dest,@src,@src2,@aux

	namespace context

		calminstruction AMX.parse_operand#context operand

			call	x86.parse_operand#context, operand

			check	type = 'imm' & size = 0
			jno	ready

			check	imm eq 1 elementof imm & 1 metadataof imm relativeto AMX.reg
			jyes	amx_register

		    ready:
			exit

		    amx_register:
			compute rm, 1 metadataof imm - AMX.reg
			compute type, 'tmmreg'
			compute mod, 11b
			exit

		end calminstruction

		calminstruction AMX.parse_sib_operand#context operand&

			local	b,i
			match	[b=,i], operand
			jyes	mpx_syntax

			take	b, x86.usesib
			compute x86.usesib, 1
			call	AVX_512.parse_operand#context, operand
			take	x86.usesib, b

			check	type <> 'mem' | rm <> 4
			jyes	invalid_operand
			exit

		    invalid_operand:
			err	'invalid operand'

		    mpx_syntax:
			local	m
			initsym m, MPX.parse_sib_operand context,
			arrange i, m operand
			assemble i

		end calminstruction

	end namespace

end iterate

iterate <instr,vex_mpw>, ldtilecfg,VEX_0F38_W0, sttilecfg,VEX_66_0F38_W0

	macro instr? src*
		require AMX_TILE
		AMX.parse_operand@src src
		if @src.type = 'mem'
			if @src.size and not 64
				err 'invalid operand size'
			end if
			AVX.store_instruction@src 16,vex_mpw,49h,0
		else
			err 'invalid operand'
		end if
	end macro

end iterate

iterate <instr,vex_mpw>, tileloadd,VEX_F2_0F38_W0, tileloaddt1,VEX_66_0F38_W0

	macro instr? dest*,src*&
		require AMX_TILE
		AMX.parse_operand@dest dest
		AMX.parse_sib_operand@src src
		if @dest.type = 'tmmreg' & @src.type = 'mem'
			AVX.store_instruction@src 16,vex_mpw,4Bh,@dest.rm
		else
			err 'invalid operand'
		end if
	end macro

end iterate

macro tilestored? dest*,src*&
	require AMX_TILE
	match dtail]=,stail, src
		AMX.parse_sib_operand@dest dest,dtail]
		AMX.parse_operand@src stail
	else
		AMX.parse_sib_operand@dest dest
		AMX.parse_operand@src src
	end match
	if @dest.type = 'mem' & @src.type = 'tmmreg'
		AVX.store_instruction@dest 16,VEX_F3_0F38_W0,4Bh,@src.rm
	else
		err 'invalid operand'
	end if
end macro

macro tilezero? dest*
	require AMX_TILE
	AMX.parse_operand@dest dest
	if @dest.type = 'tmmreg'
		x86.parse_operand@src al
		AVX.store_instruction@src 16,VEX_F2_0F38_W0,49h,@dest.rm
	else
		err 'invalid operand'
	end if
end macro

macro tilerelease?
	require AMX_TILE
	x86.parse_operand@src al
	AVX.store_instruction@src 16,VEX_0F38_W0,49h,0
end macro

iterate <instr,vex_mpw>, tdpbssd,VEX_F2_0F38_W0, tdpbsud,VEX_F3_0F38_W0, tdpbusd,VEX_66_0F38_W0, tdpbuud,VEX_0F38_W0

	macro instr? dest*,src*,src2*
		require AMX_INT8
		AMX.parse_operand@dest dest
		AMX.parse_operand@src src
		AMX.parse_operand@src2 src2
		if @dest.type = 'tmmreg' & @src.type = 'tmmreg' & @src2.type = 'tmmreg'
			AVX.store_instruction@src 16,vex_mpw,5Eh,@dest.rm,@src2.rm
		else
			err 'invalid operand'
		end if
	end macro

end iterate

macro tdpbf16ps? dest*,src*,src2*
	require AMX_BF16
	AMX.parse_operand@dest dest
	AMX.parse_operand@src src
	AMX.parse_operand@src2 src2
	if @dest.type = 'tmmreg' & @src.type = 'tmmreg' & @src2.type = 'tmmreg'
		AVX.store_instruction@src 16,VEX_F3_0F38_W0,5Ch,@dest.rm,@src2.rm
	else
		err 'invalid operand'
	end if
end macro
