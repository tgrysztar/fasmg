
; Windows 1250

macro define
	local encoding
	virtual at 0
		encoding::
		repeat 80h
			dw %-1
		end repeat
		dw 20ACh,?,201Ah,?,201Eh,2026h,2020h,2021h,?,2030h,160h,2039h,15Ah,164h,17Dh,179h
		dw ?,2018h,2019h,201Ch,201Dh,2022h,2013h,2014h,?,2122h,161h,203Ah,15Bh,165h,17Eh,17Ah
		dw 0A0h,2C7h,2D8h,141h,0A4h,104h,0A6h,0A7h,0A8h,0A9h,15Eh,0ABh,0ACh,0ADh,0AEh,17Bh
		dw 0B0h,0B1h,2DBh,142h,0B4h,0B5h,0B6h,0B7h,0B8h,105h,15Fh,0BBh,13Dh,2DDh,13Eh,17Ch
		dw 154h,0C1h,0C2h,102h,0C4h,139h,106h,0C7h,10Ch,0C9h,118h,0CBh,11Ah,0CDh,0CEh,10Eh
		dw 110h,143h,147h,0D3h,0D4h,150h,0D6h,0D7h,158h,16Eh,0DAh,170h,0DCh,0DDh,162h,0DFh
		dw 155h,0E1h,0E2h,103h,0E4h,13Ah,107h,0E7h,10Dh,0E9h,119h,0EBh,11Bh,0EDh,0EEh,10Fh
		dw 111h,144h,148h,0F3h,0F4h,151h,0F6h,0F7h,159h,16Fh,0FAh,171h,0FCh,0FDh,163h,2D9h
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
