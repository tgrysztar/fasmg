define struct?
define union?

struct?.counter = -1
struct?.subcounter = -1

struc struct?.init_field name,field,value&
	match =struct type, name.field.__type
		.field struct?.init type,value
	else match =substruct type, name.field.__type
		. struct?.init name.type,value
	else match first=,rest, value
		local data
		virtual at 0
			emit sizeof .field : value
			load data : $ from 0
		end virtual
		if lengthof data <= name.field.__length
			store data : lengthof data at .field
		else
			err 'data too long for ',`name,'.',`field
		end if
	else if value eqtype ''
		if lengthof value <= name.field.__length
			store value : lengthof value at .field
		else
			err 'data too long for ',`name,'.',`field
		end if
	else
		store value at .field
	end if
end struc

struc struct?.init name,args&
	iterate definition, args
		match field:value, definition
			. struct?.init_field name,field,value
		else match field==value, definition
			. struct?.init_field name,field,value
		else match value, definition
			match field, name.__field#%
				match root.sub, name
					. struct?.init_field root,field,value
				else
					. struct?.init_field name,field,value
				end match
			end match
		end match
	end iterate
end struc

macro struct? name
	struc name args&
		label . : sizeof.name
		namespace .
			struct?.name
		end namespace
		. struct?.init name,args
		if struct?.counter > 0
			repeat 1, i:struct?.counter
				match field, __field#i
					define field.__type struct name
				end match
			end repeat
		end if
	end struc
	macro struct?!
		struct?.begin
		esc macro struct?.current
	end macro
	macro struct?.end
		purge struct?.end
		virtual at 0
			namespace name
				struct?.counter = 0
				struct?.subcounter = 0
				define struct?.substruct
				struc (field) ? def&
					match ==v, def
						field def
					else match :v, def
						field def
					else if struct?.counter < 0
						field def
					else
						struct?.counter = struct?.counter + 1
						repeat 1, i:struct?.counter
							match sub, struct?.substruct
								define sub.__field#i field
							else
								define __field#i field
							end match
						end repeat
						field def
						if defined field
							field.__length := $ - field
						end if
					end if
				end struc
				struct?.name
				restruc ?
				struct?.counter = -1
				struct?.subcounter = -1
			end namespace
			label name:$ at $
			sizeof.name = sizeof name
		end virtual
		purge struct?
	end macro
	esc macro struct?.name
end macro

macro struct?.container
	struct?.current
end macro

macro struct?.begin
	macro struct?.end
		purge struct?.end
		if struct?.subcounter >= 0
			struct?.subcounter = struct?.subcounter + 1
			repeat 1, j:struct?.subcounter
				if struct?.counter >= 0
					struct?.counter = struct?.counter + 1
					repeat 1, i:struct?.counter
						match sub, struct?.substruct
							define sub.__field#i __substruct#j
						else
							define __field#i __substruct#j
						end match
					end repeat
				end if
				define struct?.substruct __substruct#j
				define __substruct#j.__type substruct __substruct#j
				struct?.counter =: 0
			end repeat
		end if
		struct?.container
		if struct?.subcounter > 0
			purge struct?.current
			restore struct?.substruct,struct?.counter
		end if
	end macro
end macro

macro union?.begin
	local instance
	label instance
	macro struct?!
		struct?.begin
		esc macro struct?.current
	end macro
	macro struct?.end
		purge struct?.end
		macro struct?.container
			instance !--
		end macro
		struc (field) ? def&
			if $-instance = 0
				match !--, def
					struct?.current
				else
					field def
				end match
				if $-instance > 0
					struct?.counter =: -1
				end if
			else
				virtual at instance
					match !--, def
						struct?.current
					else
						field def
					end match
					local size
					size = $-instance
				end virtual
				if size > $-instance
					rb size-($-instance)
				end if
			end if
		end struc
		union?.current
		restruc ?
		purge union?.current,struct?.container
		if $-instance > 0
			restore struct?.counter
		end if
		purge struct?
	end macro
end macro

macro union?!
	union?.begin
	esc macro union?.current
end macro

macro ends?!
	esc end macro
	struct?.end
end macro

macro struct? definition
	match name base, definition
		macro struct?.launcher
			purge struct?.launcher
			struct name
			struct?.base
		end macro
	else
		macro struct?.launcher
			purge struct?.launcher
			struct definition
		end macro
	end match
	struct?.launcher
end macro
