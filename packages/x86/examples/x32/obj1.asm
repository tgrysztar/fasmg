
format ELFx32

section '.text' executable

    public _start
    _start:

    extrn writemsg

	lea	   esi,[msg]
	call	    writemsg

	xor	   edi,edi
	mov	   eax,60
	syscall

section '.data' writeable

     msg db "We are good to go!",0xA,0
