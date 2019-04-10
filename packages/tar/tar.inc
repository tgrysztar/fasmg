
; Tarball generator that emulates $%, $%% and POSTPONE for the encapsulated file sources.

macro tar_number? value*,length:8
	local d
	repeat length-1
		d = value shr ((length-1-%)*3)
		if d > 0
			db '0' + d and 111b
		else
			db 20h
		end if
	end repeat
	db 20h
end macro

macro tar_record? name

	local postponed,data,size,checksum_field,checksum,byte

	org 0

	db string name,(100 - lengthof string name) dup 0
	tar_number 10077o	; file mode
	tar_number 0		; owner id
	tar_number 0		; group id
	tar_number size,12	; file size
	tar_number %t,12	; last modification time
	checksum_field db 8 dup 20h
	db '0',100 dup 0	; normal file

	checksum = 0
	repeat $
		load byte : 1 from $-%
		checksum = checksum + byte
	end repeat
	repeat 6
		byte = checksum shr ((6-%)*3)
		store '0' + byte and 111b : 1 at checksum_field + % - 1
	end repeat
	store 0:1 at checksum_field + 7

	db 512-$ dup 0

	org 0
	data = $%
	define $% ($%-data)
	define $%% ($%%-data)

	macro postponed
	end macro

	macro postpone?!
	    esc macro postponed 
	end macro  

	macro end?.postpone?!  
		postponed 
	    esc end macro  
	end macro

	macro end?.tar_record?
		postponed
		size = $%
		db (512 - size and 511) dup 0
		restore $%,$%%
		purge postpone?,end?.postpone?,end?.tar_record?
	end macro

end macro
