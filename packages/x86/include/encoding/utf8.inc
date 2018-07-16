
; UTF-8

macro WCHAR arg
  local current,__input,char,wide
  if arg eqtype ''
    virtual at 0
      __input::
      db arg
      count = $
    end virtual
    current = 0
    while current < count
      load char byte from __input:current
      wide = char
      current = current + 1
      if char > 0C0h
	if char < 0E0h
	  wide = char and 11111b
	  load char byte from __input:current
	  wide = wide shl 6 + (char and 111111b)
	  current = current + 1
	else if char < 0F0h
	  wide = char and 1111b
	  load char byte from __input:current
	  wide = wide shl 6 + (char and 111111b)
	  load char byte from __input:current+1
	  wide = wide shl 6 + (char and 111111b)
	  current = current + 2
	else if char < 0F8h
	  wide = char and 111b
	  load char byte from __input:current
	  wide = wide shl 6 + (char and 111111b)
	  load char byte from __input:current+1
	  wide = wide shl 6 + (char and 111111b)
	  load char byte from __input:current+2
	  wide = wide shl 6 + (char and 111111b)
	  current = current + 3
	else if char < 0FCh
	  wide = char and 11b
	  load char byte from __input:current
	  wide = wide shl 6 + (char and 111111b)
	  load char byte from __input:current+1
	  wide = wide shl 6 + (char and 111111b)
	  load char byte from __input:current+2
	  wide = wide shl 6 + (char and 111111b)
	  load char byte from __input:current+3
	  wide = wide shl 6 + (char and 111111b)
	  current = current + 4
	else
	  wide = char and 1
	  load char byte from __input:current
	  wide = wide shl 6 + (char and 111111b)
	  load char byte from __input:current+1
	  wide = wide shl 6 + (char and 111111b)
	  load char byte from __input:current+2
	  wide = wide shl 6 + (char and 111111b)
	  load char byte from __input:current+3
	  wide = wide shl 6 + (char and 111111b)
	  load char byte from __input:current+4
	  wide = wide shl 6 + (char and 111111b)
	  current = current + 5
	end if
      end if
      if wide < 10000h
	dw wide
      else
	dw 0D7C0h + wide shr 10,0DC00h or (wide and 3FFh)
      end if
    end while
  else
    wide = arg
    if wide < 10000h
      dw wide
    else
      dw 0D7C0h + wide shr 10,0DC00h or (wide and 3FFh)
    end if
  end if
end macro

macro du? definitions&
    iterate value,definitions
	match ?, value
	    dw ?
	else match n =dup? ?, value
	    dw n dup ?
	else match n =dup? (?), value
	    dw n dup ?
	else match n =dup? v, value
	    repeat n
		WCHAR v
	    end repeat
	else
	    WCHAR value
	end match
    end iterate
end macro

struc du? definitions&
    label . : word
    iterate value,definitions
	match ?, value
	    dw ?
	else match n =dup? ?, value
	    dw n dup ?
	else match n =dup? (?), value
	    dw n dup ?
	else match n =dup? v, value
	    repeat n
		WCHAR v
	    end repeat
	else
	    WCHAR value
	end match
    end iterate
end struc

