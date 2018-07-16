
macro cominvk Object,proc,args&
	if ~ defined Object#.com.object
		err `Object,' is not a COM object'
	end if
	macro call dummy
		mov rax,[rcx]
		Call [rax+Object.proc]
	end macro
	match any, args
		fastcall -,[Object],args
	else
		fastcall -,[Object]
	end match
	purge call
end macro

macro comcall handle,Interface,proc,args&
	if ~ defined Interface#.com.interface
		err `Interface,' is not a COM interface'
	end if
	macro call dummy
		mov rax,[rcx]
		Call [rax+Interface.proc]
	end macro
	match any, args
		fastcall -,handle,args
	else
		fastcall -,handle
	end match
	purge call
end macro

macro interface name,methods&
	struc name
		. dq ?
		virtual at 0
		iterate method, methods
			.method dq ?
		end iterate
		end virtual
		.com.object = name.com.interface
	end struc
	virtual at 0
	iterate method, methods
		name.method dq ?
	end iterate
	define name
	name.com.interface = $ shr 3
	end virtual
end macro
