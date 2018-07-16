
; Windows 1251

macro define
	local encoding
	virtual at 0
		encoding::
		repeat 80h
			dw %-1
		end repeat
		dw 401h,403h,201Ah,453h,201Eh,2026h,2020h,2021h,20ACh,2030h,409h,2039h,40Ah,40Ch,40Bh,40Fh
		dw 452h,2018h,2019h,201Ch,201Dh,2022h,2013h,2014h,?,2122h,459h,203Ah,45Ah,45Ch,45Bh,45Fh
		dw 0A0h,40Eh,45Eh,408h,0A4h,490h,0A6h,0A7h,401h,0A9h,404h,0ABh,0ACh,0ADh,0AEh,407h
		dw 0B0h,0B1h,406h,456h,491h,0B5h,0B6h,0B7h,451h,2116h,454h,0BBh,458h,405h,455h,457h
		repeat 40h
			dw 410h+%-1
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

macro du? definitions&
    iterate value,definitions
	match ?, value
	    dw ?
	else match n =dup? ?, value
	    dw n dup ?
	else match n =dup? (?), value
	    dw n dup ?
	else match n =dup? v, value
	    repeat n
		WCHAR v
	    end repeat
	else
	    WCHAR value
	end match
    end iterate
end macro

struc du? definitions&
    label . : word
    iterate value,definitions
	match ?, value
	    dw ?
	else match n =dup? ?, value
	    dw n dup ?
	else match n =dup? (?), value
	    dw n dup ?
	else match n =dup? v, value
	    repeat n
		WCHAR v
	    end repeat
	else
	    WCHAR value
	end match
    end iterate
end struc
