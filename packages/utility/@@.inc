
; Extended implementation of anonymous labels.

; Classic use like:
;	@@:			; A
;		je	@f	; -> B
;		loop	@b	; -> A
;	@@:			; B

; Additional sets of labels are available for more flexibility:
;	@1:			; A
;		je	@1f	; -> C
;		jg	@2f	; -> B
;		loop	@1b	; -> A
;	@2:			; B
;		loop	@2b	; -> B
;	@1:			; C

macro @INIT name,prefix

	macro name tail&
		match label, prefix#f?
			label tail
			prefix#b? equ prefix#f?
			prefix#r? equ prefix#f?
		end match
		local anonymous
		prefix#f? equ anonymous
	end macro

	define prefix#f?
	name

end macro

@INIT @@,@

repeat 10, i:0
	@INIT @#i,@#i
end repeat