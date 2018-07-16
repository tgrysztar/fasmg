
include 'win32w.inc'

include 'macro/if.inc'

define _winx
define _winx.code? :_code
define _winx.data? :_data
define _winx.end? :_end

struc (name) ? a&
	match :cmd, _winx#name?
		cmd a
	else
		name a
	end match
end struc

macro _data
	section '.data' data readable writeable
end macro

macro _code
	section '.text' code readable executable
end macro

macro _end label

	entry label

	section '.idata' import data readable writeable

	library kernel32,'KERNEL32.DLL',\
		user32,'USER32.DLL',\
		gdi32,'GDI32.DLL',\
		advapi32,'ADVAPI32.DLL',\
		comctl32,'COMCTL32.DLL',\
		comdlg32,'COMDLG32.DLL',\
		shell32,'SHELL32.DLL',\
		wsock32,'WSOCK32.DLL'

	include! 'api/kernel32.inc'
	include! 'api/user32.inc'
	include! 'api/gdi32.inc'
	include! 'api/advapi32.inc'
	include! 'api/comctl32.inc'
	include! 'api/comdlg32.inc'
	include! 'api/shell32.inc'
	include! 'api/wsock32.inc'

end macro

define x86
restore x86
if ~ defined x86
	format PE GUI 4.0
end if
