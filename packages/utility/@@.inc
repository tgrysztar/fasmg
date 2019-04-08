
macro @INIT name,prefix

	macro name tail
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