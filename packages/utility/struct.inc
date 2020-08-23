
; Basic implementation of STRUCT and UNION.

; A structure should be declared like:
;	struct POINT
;		x dd ?
;		y dd ?
;	ends
; This defines field offsets like POINT.x and POINT.y and allows to instantiate structure
; either in default form:
;	first POINT
; or with some (or all) of the fields initialized with specified values:
;	second POINT x:10

; A union can be used as a part of structure or standalone:
;	union
;		name dq ?
;		initial db ?
;	endu

macro struct? name
	macro ends?!
			end namespace
			iterate definition, args@struct
				match name:value, definition
					store value at .name
				else match name==value, definition
					store value at .name
				else match value, definition
					err 'unnamed values not supported'
				end match
			end iterate
		esc end struc
		virtual at 0
			name name
			sizeof.name = $
		end virtual
		purge ends?
	end macro
	esc struc name args@struct&
		label . : sizeof.name
		namespace .
end macro

macro union?
	local union
	union = $
	macro ? line&
		match =endu?, line
			purge ?
		else if $-union = 0
			line
		else
			virtual at union
				line
				local size
				size = $-union
			end virtual
			if size > $-union
				rb size-($-union)
			end if
		end if
	end macro
end macro
