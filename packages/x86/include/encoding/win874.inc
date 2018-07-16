
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
