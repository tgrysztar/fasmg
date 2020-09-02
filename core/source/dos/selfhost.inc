
include '../../examples/x86/include/80386.inc'

macro format?.MZ?
	format binary as 'exe'
	include '../../examples/x86/include/format/mz.inc'
end macro

macro struct? name
	macro ends?!
			end namespace
		esc end struc
		virtual at 0
			name name
			sizeof.name = $
		end virtual
		purge ends?
	end macro
	esc struc name
		label . : sizeof.name
		namespace .
end macro
