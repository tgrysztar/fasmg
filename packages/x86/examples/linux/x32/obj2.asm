
format ELFx32

section '.text' executable

    public writemsg

    writemsg:
	xor	   edx,edx
      find_end:
	cmp	   byte [rsi+rdx],0
	je	  end_found
	inc	   edx
	jmp	   find_end
      end_found:
	mov	   edi,1		; STDOUT
	mov	   eax,1		; sys_write
	syscall
	ret
