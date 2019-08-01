
include 'avx.inc'

iterate <instr,lcode>, vfmaddsub,6, vfmsubadd,7, vfmadd,8, vfmsub,0Ah, vfnmadd,0Ch, vfnmsub,0Eh

	iterate <order,hcode>, 132,90h, 213,0A0h, 231,0B0h

		macro instr#order#pd? dest*,src*,src2*
			AVX.basic_instruction VEX_66_0F38_W1,hcode+lcode,0,dest,src,src2
		end macro

		macro instr#order#ps? dest*,src*,src2*
			AVX.basic_instruction VEX_66_0F38_W0,hcode+lcode,0,dest,src,src2
		end macro

		if lcode > 7

			macro instr#order#sd? dest*,src*,src2*
				AVX.basic_instruction VEX_66_0F38_W1,hcode+lcode+1,8,dest,src,src2
			end macro

			macro instr#order#ss? dest*,src*,src2*
				AVX.basic_instruction VEX_66_0F38_W0,hcode+lcode+1,4,dest,src,src2
			end macro

		end if

	end iterate

end iterate
