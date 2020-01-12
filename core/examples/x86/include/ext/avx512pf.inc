
if ~ defined AVX_512

	include 'avx512f.inc'

end if

iterate <instr,postbyte>, gatherpf0,1 ,gatherpf1,2 ,scatterpf0,5, scatterpf1,6

	macro v#instr#dps? src*
		AVX_512.parse_vsib_operand_k1 @src,src
		if @src.type = 'mem' & @src.mask
			if @src.size and not 4 | @src.visize <> 64
				err 'invalid operand size'
			end if
			@src.memsize = 4
			AVX_512.store_instruction 64,VEX_66_0F38_W0,EVEX_REQUIRED,0C6h,@src,@src.mask,postbyte,@src.index and 10000b
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro v#instr#qps? src*
		AVX_512.parse_vsib_operand_k1 @src,src
		if @src.type = 'mem' & @src.mask
			if @src.size and not 8 | @src.visize <> 64
				err 'invalid operand size'
			end if
			@src.memsize = 4
			AVX_512.store_instruction 64,VEX_66_0F38_W0,EVEX_REQUIRED,0C7h,@src,@src.mask,postbyte,@src.index and 10000b
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro v#instr#dpd? src*
		AVX_512.parse_vsib_operand_k1 @src,src
		if @src.type = 'mem' & @src.mask
			if @src.size and not 4 | @src.visize <> 32
				err 'invalid operand size'
			end if
			@src.memsize = 8
			AVX_512.store_instruction 64,VEX_66_0F38_W1,EVEX_REQUIRED,0C6h,@src,@src.mask,postbyte,@src.index and 10000b
		else
			err 'invalid combination of operands'
		end if
	end macro

	macro v#instr#qpd? src*
		AVX_512.parse_vsib_operand_k1 @src,src
		if @src.type = 'mem' & @src.mask
			if @src.size and not 8	| @src.visize <> 64
				err 'invalid operand size'
			end if
			@src.memsize = 8
			AVX_512.store_instruction 64,VEX_66_0F38_W1,EVEX_REQUIRED,0C7h,@src,@src.mask,postbyte,@src.index and 10000b
		else
			err 'invalid combination of operands'
		end if
	end macro

end iterate
