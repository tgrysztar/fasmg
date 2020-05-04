
include 'win64w.inc'

include 'macro/if.inc'

define _winx
define _winx.code? _code
define _winx.data? _data
define _winx.end? _end

calminstruction (name) ? a&
	local cmd
	match .cmd, name
	jno pass
	transform cmd, _winx
	jno pass
	arrange cmd, cmd a
	assemble cmd
	exit
    pass:
	arrange cmd, name a
	assemble cmd
end calminstruction

macro _data
	section '.data' data readable writeable
end macro

macro _code
	section '.text' code readable executable
	entry $
	sub rsp,8
	if defined _winx.entry & ~ _winx.entry eq $
		jmp _winx.entry
	end if
end macro

macro _end label

	_winx.entry := label

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

if ~ definite PE
	format PE64 GUI 5.0
end if
