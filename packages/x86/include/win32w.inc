
macro api names&
	iterate name, names
		if used name
			label name:dword at name#W
		end if
	end iterate
end macro

struc? TCHAR args:?&
	. du args
end struc

macro TCHAR args:?&
	du args
end macro

sizeof.TCHAR = 2

include 'format/format.inc'

include 'encoding/utf8.inc'

include 'macro/@@.inc'
include 'macro/struct.inc'
include 'macro/proc32.inc'
include 'macro/com32.inc'
include 'macro/import32.inc'
include 'macro/export.inc'
include 'macro/resource.inc'

include 'equates/kernel32.inc'
include 'equates/user32.inc'
include 'equates/gdi32.inc'
include 'equates/comctl32.inc'
include 'equates/comdlg32.inc'
include 'equates/shell32.inc'
include 'equates/wsock32.inc'
