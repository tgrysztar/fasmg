
; This is a very basic implementation of 8051 instruction set, which treats
; all types of addresses as plain numeric values and therefore is not able to
; detect whether symbol has been used in context it was not intended for.

element R

repeat 8 i:0
  element R#i? : R + i
end repeat

element @

element @R0? : @
element @R1? : @ + 1

macro AJMP? addr
	local value
	value = +addr
	if value and not 7FFh = ($+2) and not 7FFh
		db 01h + value shr 3 and 11100000b,value and 0FFh
	else
		err "address out of range"
	end if
end macro

macro ACALL? addr
	local value
	value = +addr
	if value and not 7FFh = ($+2) and not 7FFh
		db 11h + value shr 3 and 11100000b,value and 0FFh
	else
		err "address out of range"
	end if
end macro

macro LCALL? addr
	local value
	value = +addr
	db 12h,value shr 8,value and 0FFh
end macro

macro LJMP? addr
	local value
	value = +addr
	db 02h,value shr 8,value and 0FFh
end macro

macro SJMP? addr
	local offset
	offset = -($+2)+addr
	if offset>=-80h & offset<80h
		db 80h,offset
	else
		err "relative jump out of range"
	end if
end macro

macro CALL? addr
	local value
	value = +addr
	if value and not 7FFh = ($+2) and not 7FFh
		db 11h + value shr 3 and 11100000b
	else
		db 12h,value shr 8
	end if
	db value and 0FFh
end macro

macro JMP? addr
	local value,offset
	match =@A? + =DPTR?, addr
		db 73h
	else
		value = +addr
		offset = value-($+2)
		if offset>=-80h & offset<80h
			db 80h,offset
		else
			if value and not 7FFh = ($+2) and not 7FFh
				db 01h + value shr 3 and 11100000b
			else
				db 02h,value shr 8
			end if
			db value and 0FFh
		end if
	end match
end macro

macro CJNE? operand1,operand2,addr
	local value,offset
	offset = -($+3)+addr
	if offset>=-80h & offset<80h
		match =A?, operand1
			match #data, operand2
				value = +data
				db 0B4h,value
			else
				value = +operand2
				db 0B5h,value
			end match
		else match #data,operand2
			value = +operand1
			if value eq value element 1
				if value metadata 1 relativeto @
					db 0B6h + value metadata 1 - @
				else if value metadata 1 relativeto R
					db 0B8h + value metadata 1 - R
				else
					err "invalid operand"
				end if
				db +data
			else
				err "invalid operand"
			end if
		else
			err 'invalid operand'
		end match
		db offset
	else
		err "relative jump out of range"
	end if
end macro

macro DJNZ? operand,addr
	local value,offset
	value = +operand
	if value relativeto 0
		offset = -($+3)+addr
		if offset>=-80h & offset<80h
			db 0D5h,value,offset
		else
			err "relative jump out of range"
		end if
	else if value eq value element 1 & value metadata 1 relativeto R
		offset = -($+2)+addr
		if offset>=-80h & offset<80h
			db 0D8h + value metadata 1 - R,offset
		else
			err "relative jump out of range"
		end if
	else
		err "invalid operand"
	end if
end macro

macro JBC? operand,addr
	local offset
	offset = -($+3)+addr
	if offset>=-80h & offset<80h
		db 10h,operand,offset
	else
		err "relative jump out of range"
	end if
end macro

macro JB? operand,addr
	local offset
	offset = -($+3)+addr
	if offset>=-80h & offset<80h
		db 20h,operand,offset
	else
		err "relative jump out of range"
	end if
end macro

macro JNB? operand,addr
	local offset
	offset = -($+3)+addr
	if offset>=-80h & offset<80h
		db 30h,operand,offset
	else
		err "relative jump out of range"
	end if
end macro

macro JC? addr
	local offset
	offset = -($+2)+addr
	if offset>=-80h & offset<80h
		db 40h,offset
	else
		err "relative jump out of range"
	end if
end macro

macro JNC? addr
	local offset
	offset = -($+2)+addr
	if offset>=-80h & offset<80h
		db 50h,offset
	else
		err "relative jump out of range"
	end if
end macro

macro JZ? addr
	local offset
	offset = -($+2)+addr
	if offset>=-80h & offset<80h
		db 60h,offset
	else
		err "relative jump out of range"
	end if
end macro

macro JNZ? addr
	local offset
	offset = -($+2)+addr
	if offset>=-80h & offset<80h
		db 70h,offset
	else
		err "relative jump out of range"
	end if
end macro

macro ADD? accu,operand
	local value
	match =A?, accu
		match #data, operand
			value = +data
			db 24h,value
		else
			value = +operand
			if value relativeto 0
				db 25h,value
			else if value eq value element 1
				if value metadata 1 relativeto @
					db 26h + value metadata 1 - @
				else if value metadata 1 relativeto R
					db 28h + value metadata 1 - R
				else
					err "invalid operand"
				end if
			else
				err "invalid operand"
			end if
		end match
	else
		err 'invalid operand'
	end match
end macro

macro ADDC? accu,operand
	local value
	match =A?, accu
		match #data, operand
			value = +data
			db 34h,value
		else
			value = +operand
			if value relativeto 0
				db 35h,value
			else if value eq value element 1
				if value metadata 1 relativeto @
					db 36h + value metadata 1 - @
				else if value metadata 1 relativeto R
					db 38h + value metadata 1 - R
				else
					err "invalid operand"
				end if
			else
				err "invalid operand"
			end if
		end match
	else
		err 'invalid operand'
	end match
end macro

macro SUBB? accu,operand
	local value
	match =A?, accu
		match #data, operand
			value = +data
			db 94h,value
		else
			value = +operand
			if value relativeto 0
				db 95h,value
			else if value eq value element 1
				if value metadata 1 relativeto @
					db 96h + value metadata 1 - @
				else if value metadata 1 relativeto R
					db 98h + value metadata 1 - R
				else
					err "invalid operand"
				end if
			else
				err "invalid operand"
			end if
		end match
	else
		err 'invalid operand'
	end match
end macro

macro ANL? dest,src
	local value,data_value
	match =A?, dest
		match #data, src
			value = +data
			db 54h,value
		else
			value = +src
			if value relativeto 0
				db 55h,value
			else if value eq value element 1
				if value metadata 1 relativeto @
					db 56h + value metadata 1 - @
				else if value metadata 1 relativeto R
					db 58h + value metadata 1 - R
				else
					err "invalid operand"
				end if
			else
				err "invalid operand"
			end if
		end match
	else match =C?, dest
		match /addr,src
			db 0B0h,addr
		else
			db 82h,src
		end match
	else
		match =A?, src
			value = +dest
			db 52h,value
		else match #data, src
			value = +dest
			data_value = +data
			db 53h,value,data_value
		else
			err 'invalid operand'
		end match
	end match
end macro

macro ORL? dest,src
	local value,data_value
	match =A?, dest
		match #data, src
			value = +data
			db 44h,value
		else
			value = +src
			if value relativeto 0
				db 45h,value
			else if value eq value element 1
				if value metadata 1 relativeto @
					db 46h + value metadata 1 - @
				else if value metadata 1 relativeto R
					db 48h + value metadata 1 - R
				else
					err "invalid operand"
				end if
			else
				err "invalid operand"
			end if
		end match
	else match =C?, dest
		match /addr,src
			db 0A0h,addr
		else
			db 72h,src
		end match
	else
		match =A?, src
			value = +dest
			db 42h,value
		else match #data, src
			value = +dest
			data_value = +data
			db 43h,value,data_value
		else
			err 'invalid operand'
		end match
	end match
end macro

macro XRL? dest,src
	local value,data_value
	match =A?, dest
		match #data, src
			value = +data
			db 64h,value
		else
			value = +src
			if value relativeto 0
				db 65h,value
			else if value eq value element 1
				if value metadata 1 relativeto @
					db 66h + value metadata 1 - @
				else if value metadata 1 relativeto R
					db 68h + value metadata 1 - R
				else
					err "invalid operand"
				end if
			else
				err "invalid operand"
			end if
		end match
	else
		match =A?, src
			value = +dest
			db 62h,value
		else match #data, src
			value = +dest
			data_value = +data
			db 63h,value,data_value
		else
			err 'invalid operand'
		end match
	end match
end macro

macro CLR? operand
	match =A?, operand
		db 0E4h
	else match =C?, operand
		db 0C3h
	else
		db 0C2h,operand
	end match
end macro

macro CPL? operand
	match =A?, operand
		db 0F4h
	else match =C?, operand
		db 0B3h
	else
		db 0B2h,operand
	end match
end macro

macro SETB? operand
	match =C?, operand
		db 0D3h
	else
		db 0D2h,operand
	end match
end macro

macro DEC? operand
	local value
	match =A?, operand
		db 14h
	else
		value = +operand
		if value relativeto 0
			db 15h,value
		else if value eq value element 1
			if value metadata 1 relativeto @
				db 16h + value metadata 1 - @
			else if value metadata 1 relativeto R
				db 18h + value metadata 1 - R
			else
				err "invalid operand"
			end if
		else
			err "invalid operand"
		end if
	end match
end macro

macro INC? operand
	local value
	match =A?, operand
		db 04h
	else match =DPTR?, operand
		db 0A3h
	else
		value = +operand
		if value relativeto 0
			db 05h,value
		else if value eq value element 1
			if value metadata 1 relativeto @
				db 06h + value metadata 1 - @
			else if value metadata 1 relativeto R
				db 08h + value metadata 1 - R
			else
				err "invalid operand"
			end if
		else
			err "invalid operand"
		end if
	end match
end macro

macro MOV? dest,src
	local value,data_value
	match =A?, dest
		match #data, src
			value = +data
			db 74h,value
		else
			value = +src
			if value relativeto 0
				db 0E5h,value
			else if value eq value element 1
				if value metadata 1 relativeto @
					db 0E6h + value metadata 1 - @
				else if value metadata 1 relativeto R
					db 0E8h + value metadata 1 - R
				else
					err "invalid operand"
				end if
			else
				err "invalid operand"
			end if
		end match
	else match =C?, dest
		db 0A2h,src
	else match =C?, src
		db 92h,dest
	else match =DPTR?, dest
		value = src
		db 90h
		if value eqtype ''
		      dw +src
		else
		      db value shr 8,value and 0FFh
		end if
	else
		value = +dest
		if value relativeto 0
			match =A?, src
				db 0F5h,value
			else match #data, src
				data_value = +data
				db 75h,value,data_value
			else
				@value2 = +src
				if @value2 relativeto 0
					db 85h,@value2,value
				else if @value2 eq @value2 element 1
					if @value2 metadata 1 relativeto @
						db 86h + @value2 metadata 1 - @,value
					else if @value2 metadata 1 relativeto R
						db 88h + @value2 metadata 1 - R,value
					end if
				else if
					err "invalid operand"
				end if
			end match
		else if value eq value element 1
			if value metadata 1 relativeto @
				match =A?, src
					db 0F6h + value metadata 1 - @
				else match #data, src
					data_value = +data
					db 76h + value metadata 1 - @,data_value
				else
					data_value = +src
					db 0A6h + value metadata 1 - @,data_value
				end match
			else if value metadata 1 relativeto R
				match =A?, src
					db 0F8h + value metadata 1 - R
				else match #data, src
					data_value = +data
					db 78h + value metadata 1 - R,data_value
				else
					data_value = +src
					db 0A8h + value metadata 1 - R,data_value
				end match
			else
				err "invalid operand"
			end if
		else
			err "invalid operand"
		end if
	end match
end macro

macro MOVC? operands&
	match =A?=, =@A? + =DPTR?, operands
		db 93h
	else match =A?=, =@A? + =PC?, operands
		db 83h
	else
		err "invalid operand"
	end match
end macro

macro MOVX? dest,src
	local value
	match =A?, dest
		match =@DPTR?, src
			db 0E0h
		else
			value = +src
			if value eq value element 1 & value metadata 1 relativeto @
				db 0E2h + value metadata 1 - @
			else
				err "invalid operand"
			end if
		end match
	else match =A?, src
		match =@DPTR?, dest
			db 0F0h
		else
			value = +dest
			if value eq value element 1 & value metadata 1 relativeto @
				db 0F2h + value metadata 1 - @
			else
				err "invalid operand"
			end if
		end match
	else
		err "invalid operand"
	end match
end macro

macro SWAP? operand
	match =A?, operand
		db 0C4h
	else
		err 'invalid operand'
	end match
end macro

macro DA? operand
	match =A?, operand
		db 0D4h
	else
		err 'invalid operand'
	end match
end macro

macro RR? operand
	match =A?, operand
		db 03h
	else
		err 'invalid operand'
	end match
end macro

macro RRC? operand
	match =A?, operand
		db 13h
	else
		err 'invalid operand'
	end match
end macro

macro RL? operand
	match =A?, operand
		db 23h
	else
		err 'invalid operand'
	end match
end macro

macro RLC? operand
	match =A?, operand
		db 33h
	else
		err 'invalid operand'
	end match
end macro

macro DIV? operand
	match =AB?, operand
		db 84h
	else
		err "invalid operand"
	end match
end macro

macro MUL? operand
	match =AB?, operand
		db 0A4h
	else
		err "invalid operand"
	end match
end macro

macro NOP?
	db 0
end macro

macro POP? addr
	local value
	value = +addr
	db 0D0h,value
end macro

macro PUSH? addr
	local value
	value = +addr
	db 0C0h,value
end macro

macro RET?
	db 22h
end macro

macro RETI?
	db 32h
end macro

macro XCH? accu,operand
	local value
	match =A?, accu
		value = +operand
		if value relativeto 0
			db 0C5h,value
		else if value eq value element 1
			if value metadata 1 relativeto @
				db 0C6h + value metadata 1 - @
			else if value metadata 1 relativeto R
				db 0C8h + value metadata 1 - R
			else
				err "invalid operand"
			end if
		else
			err "invalid operand"
		end if
	else
		err 'invalid operand'
	end match
end macro

macro XCHD? accu,src
	local value
	match =A?, accu
		value = +src
		if value eq value element 1 & value metadata 1 relativeto @
			db 0D6h + value metadata 1 - @
		else
			err "invalid operand"
		end if
	else
		err "invalid operand"
	end match
end macro

struc EQU? value
	. = value
end struc

DSEG?.$ = 0
DSEG?.open = 0

macro DSEG? @:at DSEG?.$
	if ~ DSEG?.open
		virtual @
		DSEG?.open = 1
	else
		match =AT? addr, @
			org addr
		else
			err 'invalid argument'
		end match
	end if
end macro

macro CSEG? @
	if DSEG?.open
		DSEG?.$ = $
		end virtual
		DSEG?.open = 0
	end if
	match =AT? addr, @
		org addr
	else match any, @
		err 'invalid argument'
	end match
end macro

macro bitslabel definition
	match name =at? address, definition
		label name at address
		repeat 8, i:0
			label name.i at address+i
		end repeat
	else
		err 'syntax error'
	end match
end macro

; Data addresses:

bitslabel P0	at 080h
bitslabel SP	at 081h
bitslabel DPL	at 082h
bitslabel DPH	at 083h
bitslabel PCON	at 087h
bitslabel TCON	at 088h
bitslabel TMOD	at 089h
bitslabel TL0	at 08Ah
bitslabel TL1	at 08Bh
bitslabel TH0	at 08Ch
bitslabel TH1	at 08Dh
bitslabel P1	at 090h
bitslabel SCON	at 098h
bitslabel SBUF	at 099h
bitslabel P2	at 0A0h
bitslabel IE	at 0A8h
bitslabel P3	at 0B0h
bitslabel IP	at 0B8h
bitslabel PSW	at 0D0h
bitslabel ACC	at 0E0h
bitslabel B	at 0F0h

; Bit addresses:

label IT0	at 088h
label IE0	at 089h
label IT1	at 08Ah
label IE1	at 08Bh
label TR0	at 08Ch
label TF0	at 08Dh
label TR1	at 08Eh
label TF1	at 08Fh
label RI	at 098h
label TI	at 099h
label RB8	at 09Ah
label TB8	at 09Bh
label REN	at 09Ch
label SM2	at 09Dh
label SM1	at 09Eh
label SM0	at 09Fh
label EX0	at 0A8h
label ET0	at 0A9h
label EX1	at 0AAh
label ET1	at 0ABh
label ES	at 0ACh
label EA	at 0AFh
label RXD	at 0B0h
label TXD	at 0B1h
label INT0	at 0B2h
label INT1	at 0B3h
label T0	at 0B4h
label T1	at 0B5h
label WR	at 0B6h
label RD	at 0B7h
label PX0	at 0B8h
label PT0	at 0B9h
label PX1	at 0BAh
label PT1	at 0BBh
label PS	at 0BCh
label P 	at 0D0h
label OV	at 0D2h
label RS0	at 0D3h
label RS1	at 0D4h
label F0	at 0D5h
label AC	at 0D6h
label CY	at 0D7h

; Code addresses:

label RESET	at 000h
label EXTI0	at 003h
label TIMER0	at 00Bh
label EXTI1	at 013h
label TIMER1	at 01Bh
label SINT	at 023h
