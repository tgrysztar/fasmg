
define HEX
format binary as 'hex'

virtual at 0
	HEX.digits:: db '0123456789ABCDEF'
end virtual

macro HEX.byte value
	HEX.checksum = (HEX.checksum + (value)) and 0FFh
	local digit
	load digit:byte from HEX.digits:(value) shr 4
	db digit
	load digit:byte from HEX.digits:(value) and 0Fh
	db digit
end macro

macro HEX.line length,address,type,value
	HEX.checksum = 0
	db ':'
	HEX.byte length
	HEX.byte (address) shr 8
	HEX.byte (address) and 0FFh
	HEX.byte type
	HEX.data = value
	repeat length
		HEX.byte HEX.data and 0FFh
		HEX.data = HEX.data shr 8
	end repeat
	HEX.data = (-HEX.checksum) and 0FFh
	HEX.byte HEX.data
	db 13,10
end macro

macro HEX.seg address:0
	virtual at address
end macro

macro HEX.endseg
	local code,address,high,size,bytes
	code:: address = $$
	       size = $-$$
	end virtual
	high = 0
	while size
		if address shr 16 <> high
			high = address shr 16
			HEX.line 2,0,4,high bswap 2
		end if
		if size>10h
			load bytes:10h from code:address
			HEX.line 10h,address and 0FFFFh,0,bytes
			address = address + 10h
			size = size - 10h
		else
			load bytes:size from code:address
			HEX.line size,address and 0FFFFh,0,bytes
			break
		end if
	end while
end macro

macro ORG? address
	if $ <> address
		HEX.endseg
		HEX.seg address
	end if
end macro

HEX.seg

postpone
	HEX.endseg
	HEX.line 0,0,1,0
end postpone
