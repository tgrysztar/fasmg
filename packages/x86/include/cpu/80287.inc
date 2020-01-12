
if ~ defined i80287

	restore i80287	; this ensures that symbol cannot be forward-referenced
	i80287 = 1

	include '8087.inc'

	purge feni?,fneni?,fdisi?,fndisi?

	define x87.fsetpm? db 0DBh,0E4h

	calminstruction fsetpm?
		assemble x87.fsetpm?
	end calminstruction

end if