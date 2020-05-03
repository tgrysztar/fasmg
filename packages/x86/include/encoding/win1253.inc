
; Windows 1253

macro define
	local encoding
	virtual at 0
		encoding::
		repeat 80h
			dw %-1
		end repeat
		dw 20ACh,?,201Ah,192h,201Eh,2026h,2020h,2021h,?,2030h,?,2039h,?,?,?,?
		dw ?,2018h,2019h,201Ch,201Dh,2022h,2013h,2014h,?,2122h,?,203Ah,?,?,?,?
		dw 0A0h,385h,386h,0A3h,0A4h,0A5h,0A6h,0A7h,0A8h,0A9h,?,0ABh,0ACh,0ADh,0AEh,2015h
		dw 0B0h,0B1h,0B2h,0B3h,384h,0B5h,0B6h,0B7h,288h,389h,38Ah,0BBh,38Ch,0BDh,38Eh,38Fh
		repeat 40h
			dw 390h+%-1
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
