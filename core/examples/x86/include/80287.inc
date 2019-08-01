
if ~ definite i80287

	i80287 := 1

	include '8087.inc'

	purge feni?,fneni?,fdisi?,fndisi?

	macro fsetpm?
		db 0DBh,0E4h
	end macro

end if