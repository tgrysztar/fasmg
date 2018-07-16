
iterate N, 8,16,32

	macro uint#N value
		if value >= 0
			emit N/8: value
		else
			err 'value out of unsigned range'
		end if
	end macro

end iterate

iterate N, 1,7,32

	macro varuint#N value
		if value >= 0 & value < 1 shl N
			local tmp
			tmp = value
			while tmp >= 80h
				db (tmp and 7Fh) or 80h
				tmp = tmp shr 7
			end while
			db tmp
		else
			err 'value out of unsigned range'
		end if
	end macro

end iterate

iterate N, 7,32,64

	macro varint#N value
		if value >= -1 shl (N-1) & value < 1 shl (N-1)
			local tmp
			tmp = value
			while tmp >= 80h | tmp < -80h
				db (tmp and 7Fh) or 80h
				tmp = tmp shr 7
			end while
			db tmp and 7Fh
		else
			err 'value out of signed range'
		end if

	end macro

end iterate

; Instruction set:

iterate <instruction,opcode>, get_local,0x20, set_local,0x21, tee_local,0x22, get_global,0x23, set_global,0x24, call,0x10

	macro instruction index*
		db opcode
		varuint32 index
	end macro

end iterate

i32 = -0x01 and 7Fh
i64 = -0x02 and 7Fh
f32 = -0x03 and 7Fh
f64 = -0x04 and 7Fh

iterate <instruction,opcode>, i32.clz,0x67, i32.ctz,0x68, i32.popcnt,0x69, i32.add,0x6a, i32.sub,0x6b, i32.mul,0x6c, i32.div_s,0x6d, i32.div_u,0x6e, \
			      i32.rem_s,0x6f, i32.rem_u,0x70, i32.and,0x71, i32.or,0x72, i32.xor,0x73, i32.shl,0x74, i32.shr_s,0x75, i32.shr_u,0x76, i32.rotl,0x77, i32.rotr,0x78, \
			      i64.clz,0x79, i64.ctz,0x7a, i64.popcnt,0x7b, i64.add,0x7c, i64.sub,0x7d, i64.mul,0x7e, i64.div_s,0x7f, i64.div_u,0x80, i64.rem_s,0x81, i64.rem_u,0x82, \
			      i64.and,0x83, i64.or,0x84, i64.xor,0x85, i64.shl,0x86, i64.shr_s,0x87, i64.shr_u,0x88, i64.rotl,0x89, i64.rotr,0x8a, f32.abs,0x8b, f32.neg,0x8c, \
			      f32.ceil,0x8d, f32.floor,0x8e, f32.trunc,0x8f, f32.nearest,0x90, f32.sqrt,0x91, f32.add,0x92, f32.sub,0x93, f32.mul,0x94, f32.div,0x95, f32.min,0x96, \
			      f32.max,0x97, f32.copysign,0x98, f64.abs,0x99, f64.neg,0x9a, f64.ceil,0x9b, f64.floor,0x9c, f64.trunc,0x9d, f64.nearest,0x9e, f64.sqrt,0x9f, \
			      f64.add,0xa0, f64.sub,0xa1, f64.mul,0xa2, f64.div,0xa3, f64.min,0xa4, f64.max,0xa5, f64.copysign,0xa6

	macro instruction
		db opcode
	end macro

end iterate

macro return
	db 0x0f
end macro
