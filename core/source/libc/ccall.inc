
macro ccall proc*,[arg]
{
    common
	local size,count
	mov	ebp,esp
	if size
		sub	esp,size
	end if
	and	esp,0FFFFFFF0h
	count = 0
	if ~ arg eq
    forward
	mov	dword [esp+count*4],arg
	count = count + 1
    common
	end if
	size = count*4
	call	proc
	mov	esp,ebp
}

macro cinvoke proc*,arg& { ccall [proc],arg }
