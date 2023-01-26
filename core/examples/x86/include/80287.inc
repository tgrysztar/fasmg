
if ~ defined i80287

	restore i80287	; this ensures that symbol cannot be forward-referenced
	i80287 = 1

	include '8087.inc'

	purge feni?,fneni?,fdisi?,fndisi?

	calminstruction fsetpm?
		emit	1, 0DBh
		emit	1, 0E4h
	end calminstruction

end if