
; Universal implementation of ALIGN.

; Basic use:
;	align 4

; Ensure specific offset relative to an aligned address:
;	align 4 | 1

; Provide information for the alignment of relocatable symbols:
;	align.assume rbp, 16

;	element RELOCATABLE_BASE
;	org RELOCATABLE_BASE
;	align.assume RELOCATABLE_BASE, 10000h

; Use a custom fill:
;	macro nops? length
;		db length dup 90h
;	end macro
;	align 16, nops

define align? align

align?.count = 0

calminstruction align?.assume? address*, alignment*
	local index, symbol
	compute address, address
	compute alignment, alignment
	check address relativeto 0
	jyes absolute
	compute index, 0
    find:
	check index = align.count
	jyes update
	arrange symbol, align.=address#index
	check address relativeto symbol
	jyes update
	compute index, index + 1
	jump find
    update:
	arrange symbol, align.=address#index
	publish symbol, address
	arrange symbol, align.=alignment#index
	publish symbol, alignment
	compute align.count, align.count + 1
	exit
    absolute:
	check address mod alignment = 0
	jyes ok
	arrange symbol, =err 'false assumption about absolute address'
	assemble symbol
    ok:
end calminstruction

calminstruction align? alignment*, filler:rb
	local index, address, offset
	compute offset, 0
	match alignment | offset, alignment
	compute alignment, alignment
	check alignment >= 0
	jyes alignment_ok
	arrange symbol, =err 'invalid alignment value'
	assemble symbol
	exit
    alignment_ok:
	compute address, $
	check address relativeto 0
	jyes align
	compute index, 0
    find:
	check index = align.count
	jyes fail
	arrange symbol, align.=address#index
	check address relativeto symbol
	jyes found
	compute index, index + 1
	jump find
    found:
	compute address, address - symbol
	arrange symbol, align.=alignment#index
	check symbol > 0 & symbol mod alignment = 0
	jyes align
    fail:
	arrange symbol, =err 'variable portion of the address is not aligned enough'
	assemble symbol
	exit
    align:
	compute alignment, (alignment-1) - (address-offset+alignment-1) mod alignment
	arrange alignment, filler alignment
	assemble alignment
end calminstruction
