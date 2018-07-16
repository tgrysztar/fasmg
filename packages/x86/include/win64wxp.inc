
include 'win64wx.inc'

include 'pcount/kernel32.inc'
include 'pcount/user32.inc'
include 'pcount/gdi32.inc'
include 'pcount/advapi32.inc'
include 'pcount/comctl32.inc'
include 'pcount/comdlg32.inc'
include 'pcount/shell32.inc'
include 'pcount/wsock32.inc'

macro pcountcheck? proc*,count*
	local pname,pcount
	match p, pcountsuffix
		match [name], proc
			define pcount name#p
		else match name, proc
			define pcount name#p
		end match
	end match
	if defined pcount & count <> pcount
		repeat 1, found:count, expected:pcount
			match [name], proc
				err 'invalid count of parameters for ',`name,' (',`found,' instead of ',`expected,')'
			else
				err 'invalid count of parameters for ',`proc,' (',`found,' instead of ',`expected,')'
			end match
		end repeat
	end if
end macro