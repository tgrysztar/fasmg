
macro library? definitions&
	PE.Imports:
	iterate <name,string>, definitions
	      if ~ name.redundant
		      dd RVA name.lookup,0,0,RVA name.str,RVA name.address
	      end if
	      name.referred = 1
	end iterate
	dd 0,0,0,0,0
	iterate <name,string>, definitions
	      if ~ name.redundant
		      name.str db string,0
			       align 2
	      end if
	end iterate
end macro

macro import? name,definitions&
	align 8
	if defined name.referred
		name.lookup:
		iterate <label,string>, definitions
			if used label
				if string eqtype ''
					dq RVA name.label
				else
					dq 8000000000000000h + string
				end if
			end if
		end iterate
		if $ > name.lookup
			name.redundant = 0
			dq 0
		else
			name.redundant = 1
		end if
		name.address:
		iterate <label,string>, definitions
			if used label
				if string eqtype ''
					label dq RVA name.label
				else
					label dq 8000000000000000h + string
				end if
			end if
		end iterate
		if ~ name.redundant
			dq 0
		end if
		iterate <label,string>, definitions
			if used label & string eqtype ''
				name.label dw 0
					   db string,0
					   align 2
			end if
		end iterate
	end if
end macro
