
; Windows 1256

macro define
	local encoding
	virtual at 0
		encoding::
		repeat 80h
			dw %-1
		end repeat
		dw 20ACh,67Eh,201Ah,192h,201Eh,2026h,2020h,2021h,2C6h,2030h,679h,2039h,152h,686h,698h,688h
		dw 6AFh,2018h,2019h,201Ch,201Dh,2022h,2013h,2014h,6A9h,2122h,691h,203Ah,153h,200Ch,200Dh,6BAh
		dw 0A0h,60Ch,0A2h,0A3h,0A4h,0A5h,0A6h,0A7h,0A8h,0A9h,6BEh,0ABh,0ACh,0ADh,0AEh,0AFh
		dw 0B0h,0B1h,0B2h,0B3h,0B4h,0B5h,0B6h,0B7h,0B8h,0B9h,0BAh,0BBh,0BCh,0BDh,0BEh,0BFh
		dw 6C1h,621h,622h,623h,624h,625h,626h,627h,628h,629h,62Ah,62Bh,62Ch,62Dh,62Eh,62Fh
		dw 630h,631h,632h,633h,634h,635h,636h,0D7h,637h,638h,639h,63Ah,640h,641h,642h,643h
		dw 0E0h,644h,0E2h,645h,646h,647h,648h,0E7h,0E8h,0E9h,0EAh,0EBh,649h,64Ah,0EEh,0EFh
		dw 64Bh,64Ch,64Dh,64Eh,0F4h,64Fh,650h,0F7h,651h,0F9h,652h,0FBh,0FCh,200Eh,200Fh,6D2h
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
