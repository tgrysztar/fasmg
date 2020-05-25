
macro cominvk Object,proc,args&
	iterate arg, args
		indx 1+%%-%
		pushd arg
	end iterate
	if ~ defined Object#.com.object
		err `Object,' is not a COM object'
	end if
	mov eax,[Object]
	push eax
	mov eax,[eax]
	call [eax+Object.proc]
end macro

macro comcall handle,Interface,proc,args&
	iterate arg, args
		indx 1+%%-%
		pushd arg
	end iterate
	if ~ defined Interface#.com.interface
		err `Interface,' is not a COM interface'
	end if
	match prefix [addr], :handle
		mov eax,handle
		push eax
		mov eax,[eax]
	else
		push handle
		local _handle
		_handle := handle
		mov eax,[_handle]
	end match
	call [eax+Interface.proc]
end macro

macro interface name,methods&
	struc name
		. dd ?
		virtual at 0
		iterate method, methods
			.method dd ?
		end iterate
		end virtual
		.com.object = name.com.interface
	end struc
	virtual at 0
	iterate method, methods
		name.method dd ?
	end iterate
	define name
	name.com.interface = $ shr 2
	end virtual
end macro
