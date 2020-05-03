
; Windows 874

macro define
	local encoding
	virtual at 0
		encoding::
		repeat 80h
			dw %-1
		end repeat
		dw 20ACh,?,?,?,?,2026h,?,?,?,?,?,?,?,?,?,?
		dw ?,2018h,2019h,201Ch,201Dh,2022h,2013h,2014h,?,?,?,?,?,?,?,?
		repeat 60h
			dw 0E00h+%-1
		end repeat
	end virtual
	macro WCHAR arg
		if arg eqtype ''
			local data,char
			data db arg
			rb ($-data)
			repeat ($-data)/2
				load char:byte from data+%%-%
				load char:word from encoding:char*2
				store char:word at data+(%%-%)*2
			end repeat
		else
			dw arg
		end if
	end macro
end macro

define
purge define

calminstruction du? definitions&
	local	value, n
	transform definitions
    start:
	match	value=,definitions, definitions, ()
	jyes	recognize
	match	value, definitions
	arrange definitions,
    recognize:
	match	n =dup? value, value, ()
	jyes	duplicate
	match	?, value
	jyes	reserve
	arrange value, =WCHAR value
	assemble value
    next:
	match	, definitions
	jno	start
	take	, definitions
	take	definitions, definitions
	jyes	next
	exit
    reserve:
	arrange value, =dw ?
	assemble value
	jump	next
    duplicate:
	match	(value), value
    stack:
	check	n
	jno	next
	take	definitions, value
	arrange value, definitions
	compute n, n - 1
	jump	stack
end calminstruction

calminstruction (label) du? definitions&
	local	cmd
	arrange cmd, =label label : =word
	assemble cmd
	arrange cmd, =du definitions
	assemble cmd
end calminstruction
