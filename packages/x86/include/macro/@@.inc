
macro @@ tail&
	match label, @f?
		label tail
		@b?. equ @f?
	end match
	local anonymous
	@f?. equ anonymous
end macro

define @f?
@@

