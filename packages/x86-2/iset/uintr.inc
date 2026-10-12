
iterate <instr,opcode>, uiret,0ECh, testui,0EDh, clui,0EEh, stui,0EFh

	macro instr?
		require UINTR
		require bits64
		db 0F3h,0Fh,1,opcode
	end macro

end iterate

macro senduipi? src*
	require UINTR
	require bits64
	x86.parse_operand@src src
	if @src.type = 'reg' & @src.size = 8
		@src.opcode_prefix = 0F3h
		x86.store_instruction@src <0Fh,0C7h>,6
	else
		err 'invalid operand'
	end if
end macro
