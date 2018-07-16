
include 'wasm.inc'

function i32 sum (i32 x,i32 y)
	get_local x
	get_local y
	i32.add
	return
end function


function i32 sqrsum (i32 x,i32 y)

	var i32 s

	get_local x
	get_local y
	call sum
	set_local s

	get_local s
	get_local s
	i32.mul

	return

end function

