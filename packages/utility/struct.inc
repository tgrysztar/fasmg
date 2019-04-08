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
		end struc
		virtual at 0
			name name
			sizeof.name = $
		end virtual
		purge ends?
	end macro
	struc name args@struct&
		label . : sizeof.name
		namespace .
end macro

macro union?
	union?. = $
	macro endu?!
		purge ?,endu?
	end macro
	macro ? line&
		if $-union = 0
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
