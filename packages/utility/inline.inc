
; Simple preprocessor for inline macros.

; Example of use:
;
;	inlinemacro oddflip(number)
;		return = (number) xor 1
;	end inlinemacro
;
;	db oddflip(3), oddflip(oddflip(0) shl 1)

include 'xcalm.inc'

define inlinemacro? inlinemacro

calminstruction inlinemacro?! declaration&
	local	name
	match	name(arguments?), declaration
	jyes	define
	match	name= arguments?, declaration
	jyes	define
	match	name arguments?, declaration
    define:
	arrange tmp, =__inline__.name
	arrange name, =inlinemacro.name
	publish name, tmp
	arrange tmp, =struc (=return?) name arguments
	assemble tmp
end calminstruction

macro end?.inlinemacro?!
	end struc
end macro

calminstruction ?! text&
	local	head, tail, name, arguments, more, i
	init	i, 0
	match	=inlinemacro? more, text
	jyes	ready
	transform text, inlinemacro
	jno	ready
	match	=else? more, text
	jno	preprocess
	compute i, i+1
	arrange text, =__inline__.(=else =if 1) =__inline__.(=__return__.i==1) text =__inline__.(=end =if) =__inline__.(=if ~=definite =__return__.i)
    preprocess:
	match	head? =__inline__.name?(tail?, text
	jno	ready
	match	arguments?) tail?, tail
	jno	ready	; syntax error
    collect:
	match	arguments?, arguments, ()
	jyes	inline
	match	more?) tail?, tail
	jno	ready	; syntax error
	arrange arguments, arguments) more
	jump	collect
    inline:
	match	, name
	jyes	special
	local	tmp, return
	compute i, i+1
	arrange return, =__return__.i
	arrange tmp, return =inlinemacro.name arguments
	arrange text, head return tail
	take	text, tmp
	jump	preprocess
    special:
	arrange text, head tail
	take	text, arguments
	jump	preprocess
    ready:
	assemble text
	take	, text
	take	text, text
	jyes	preprocess
end calminstruction
