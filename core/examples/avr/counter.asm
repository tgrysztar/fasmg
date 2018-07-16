
; This example upon each reset increases the 8-bit counter stored in EEPROM
; and then presents the bits of this value on the pins of port A.

include "avr.inc"
include "..\8051\hex.inc"

include "m16def.inc"

.org 0
	rjmp	start

start:
	ldi	r16,RAMEND and $ff
	out	spl,r16
	ldi	r16,RAMEND shr 8
	out	sph,r16

	ldi	r16,$37
	call	eeprom_read_byte
	mov	r17,r16
	inc	r17
	ldi	r16,$37
	call	eeprom_write_byte

	ldi	r18,11111111b
	out	DDRA,r18
	out	PORTA,r17

hang:	jmp	hang

eeprom_read_byte:
; r16 = EEPROM address
; returns: r16 = byte data
	sbic	EECR,EEWE
	jmp	eeprom_read_byte
	out	EEARL,r16
	sbi	EECR,EERE
	in	r16,EEDR
	ret

eeprom_write_byte:
; r16 = EEPROM address
; r17 = byte data
	cli
    while_eeprom_busy:
	sbic	EECR,EEWE
	jmp	while_eeprom_busy
	out	EEARL,r16
	out	EEDR,r17
	sbi	EECR,EEMWE
	sbi	EECR,EEWE
	sei
	ret
