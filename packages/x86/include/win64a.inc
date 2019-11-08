
macro api names&
	iterate name, names
		if used name
			label name:qword at name#A
		end if
	end iterate
end macro

struc? TCHAR args:?&
	. db args
end struc

macro TCHAR args:?&
	db args
end macro

sizeof.TCHAR = 1

include 'format/format.inc'

include 'encoding/utf8.inc'

include 'macro/@@.inc'
include 'macro/struct.inc'
include 'macro/proc64.inc'
include 'macro/com64.inc'
include 'macro/import64.inc'
include 'macro/export.inc'
include 'macro/resource.inc'

include 'equates/kernel64.inc'
include 'equates/user64.inc'
include 'equates/gdi64.inc'
include 'equates/comctl64.inc'
include 'equates/comdlg64.inc'
include 'equates/shell64.inc'
include 'equates/wsock32.inc'
