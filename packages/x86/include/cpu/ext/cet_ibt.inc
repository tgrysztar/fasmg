
iterate <instr,modrm>, endbr32,0FBh, endbr64,0FAh

	macro instr?
		db 0F3h,0Fh,1Eh,modrm
	end macro

end iterate