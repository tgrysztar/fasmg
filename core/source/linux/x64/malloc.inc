
; this is a simplified memory allocator
; based on code contributed by Jeff Marrison <jeff@2ton.com.au>

MALLOC_MINIMUM_SIZE = 8
assert MALLOC_MINIMUM_SIZE = 1 shl (bsr MALLOC_MINIMUM_SIZE)	; must be a power of 2
MALLOC_MINIMUM_MASK = MALLOC_MINIMUM_SIZE - 1

MALLOC_SYSTEM_SIZE = 262144
assert MALLOC_SYSTEM_SIZE = 1 shl (bsr MALLOC_SYSTEM_SIZE)	; must be a power of 2
MALLOC_SYSTEM_MASK = MALLOC_SYSTEM_SIZE - 1

assert MALLOC_SYSTEM_SIZE mod MALLOC_MINIMUM_SIZE = 0

malloc:
malloc_fixed:
malloc_growable:
; in: ecx = requested size
; out: eax - allocated block, ecx = allocated size, on error jumps to out_of_memory (does not return)
; preserves: ebx, esi, edi
; note:
;  use of malloc_fixed hints that block will be kept as is until the end of assembly
;  use of malloc_growable hints that block is likely to be resized
	mov	eax,8
	cmp	ecx,eax 		; minimum 8 bytes for actual storage space
	cmovb	ecx,eax 		; (so that when it is freed we can maintain our freelist)
	mov	r12d,edi
	mov	r13d,esi
	mov	r14d,ecx
	mov	edx,[malloc_ffirst]
	test	edx,edx
	jz	malloc_brk
	add	ecx,MALLOC_MINIMUM_MASK+4
	and	ecx,not MALLOC_MINIMUM_MASK
	mov	eax,-1
	xor	edi,edi
    malloc_traverse:
	mov	esi,[edx-4]		; this block's size
	and	esi,not 1		; remove the freelist bit
	mov	r8d,[edx+4]
	cmp	ecx,esi
	je	malloc_use_entire_block
	ja	malloc_traverse_next
	cmp	esi,eax
	cmovb	eax,esi
	cmovb	edi,edx
    malloc_traverse_next:
	mov	edx,r8d
	test	r8d,r8d
	jnz	malloc_traverse
	test	edi,edi
	jz	malloc_brk
    malloc_use_block:
    ; in:
    ;  edi - free block to use for allocation
    ;  eax = size of the free block, including the preceding header
    ;  ecx = requested size, including the preceding header (must be aligned to MALLOC_MINIMUM_SIZE and in range from 12 to eax)
    ; out:
    ;  eax - allocated block (the same address as the free block that was given)
    ;  ecx = allocated size (preceding header not included)
    ;  edi = what was in r12d
    ;  esi = what was in r13d
    ; preserves: rbx
    ; note: for internal use only, it does not validate the parameters
	mov	edx,edi
	mov	esi,eax
	sub	esi,ecx
	cmp	esi,MALLOC_MINIMUM_SIZE+4
	jb	malloc_use_entire_block
	mov	edi,[malloc_ffirst]
	lea	r8d,[edx+ecx]		; new block's pointer (freelist is already offset by 4)
	or	esi,1
	mov	[edx+ecx-4],esi 	; size of new block with freelist bit set
	mov	dword [edx+ecx],0	; its fprev = 0
	mov	[edx+ecx+4],edi 	; its fnext = ffirst, which may be edx
	mov	[edx-4],ecx		; set new size of original block
	mov	[edi],r8d		; old ffirst's fprev = new block
	mov	[malloc_ffirst],r8d	; ffirst = new block
    malloc_use_entire_block:
	and	dword [edx-4],not 1	; clear the freelist bit
	call	unlink_from_freelist
	mov	eax,edx
	mov	ecx,[edx-4]
	sub	ecx,4
	mov	edi,r12d
	mov	esi,r13d
	ret
    malloc_brk:
    ; in:
    ;  r14d = requested size
    ; out:
    ;  eax - allocated block, equal to previous value of [malloc_lbrk] as long as it was non-zero
    ;  ecx = allocated size
    ;  edi = what was in r12d
    ;  esi = what was in r13d
    ;  on error jumps to out_of_memory (does not return)
    ; preserves: rbx
    ; note: for internal use only
	lea	ecx,[4+r14d+MALLOC_SYSTEM_MASK]
	mov	edi,[malloc_lbrk]
	test	edi,edi
	jz	malloc_mmap
	and	ecx,not MALLOC_SYSTEM_MASK
	add	edi,ecx
	jc	out_of_memory
	mov	eax,12			; sys_brk
	syscall
	mov	edx,eax
	sub	edx,[malloc_lbrk]
	jz	out_of_memory
	xchg	[malloc_lbrk],eax
      malloc_brk_use:
	lea	ecx,[4+r14d+MALLOC_MINIMUM_MASK]
	and	ecx,not MALLOC_MINIMUM_MASK
	mov	[eax],ecx		; the size of block to return
	sub	edx,ecx
	jz	malloc_brk_return
	cmp	edx,MALLOC_MINIMUM_SIZE+4
	jb	malloc_brk_extra	; extra space not enough for another small block
	mov	edi,[malloc_flast]
	lea	r8d,[eax+ecx+4]
	or	edx,1
	mov	[eax+ecx],edx		; store the size of the free block with freelist bit set
	mov	[r8d],edi		; fprev = flast
	mov	dword [r8d+4],0 	; fnext = 0
	mov	[malloc_flast],r8d
	cmp	dword [malloc_ffirst],0
	je	malloc_brk_set_first
	mov	[edi+4],r8d		; old flast's fnext = this one
	jmp	malloc_brk_return
      malloc_brk_set_first:
	mov	[malloc_ffirst],r8d
	jmp	malloc_brk_return
      malloc_brk_extra:
	add	[eax],edx
      malloc_brk_return:
	mov	ecx,[eax]
	sub	ecx,4
	add	eax,4
	mov	edi,r12d
	mov	esi,r13d
	ret
      malloc_mmap:
	add	ecx,4
	and	ecx,not MALLOC_SYSTEM_MASK
	xor	r9d,r9d
	or	r8,-1
	mov	r10d,62h		; MAP_PRIVATE + MAP_ANONYMOUS + MAP_32BIT
	mov	edx,3			; PROT_READ + PROT_WRITE
	mov	esi,ecx
	xor	edi,edi
	mov	eax,9			; sys_mmap
	syscall
	cmp	eax,-1
	je	malloc_mmap_with_hint
	mov	ecx,eax
	cmp	rcx,rax
	jne	malloc_mmap_unusable
	add	ecx,esi
	jnc	malloc_mmap_use
      malloc_mmap_unusable:
	mov	rdi,rax
	mov	eax,11			; sys_munmap
	syscall
      malloc_mmap_with_hint:
	mov	r10d,22h		; MAP_PRIVATE + MAP_ANONYMOUS
	mov	edx,3			; PROT_READ + PROT_WRITE
	mov	edi,[malloc_hint]
	mov	eax,9			; sys_mmap
	syscall
	cmp	eax,-1
	je	out_of_memory
	mov	ecx,eax
	cmp	rcx,rax
	jne	out_of_memory
	add	ecx,esi
	jc	out_of_memory
      malloc_mmap_use:
	lea	edx,[esi-4]
	and	dword [ecx-4],0 	; dummy non-freelist block
	mov	[malloc_hint],ecx
	jmp	malloc_brk_use
    unlink_from_freelist:
    ; in: edx - block in freelist to unlink
    ; preserves: rbx, rdx, rsi, rdi, r12, r13, r14
	mov	eax,[edx]		; fprev
	mov	ecx,[edx+4]		; fnext
	cmp	edx,[malloc_ffirst]
	je	unlink_first
	cmp	edx,[malloc_flast]
	je	unlink_last_not_first
	mov	[eax+4],ecx		; fprev's fnext = fnext
	mov	[ecx],eax		; fnext's fprev = fprev
	ret
      unlink_last_not_first:
	mov	[malloc_flast],eax	; flast = fprev
	mov	dword [eax+4],0 	; flast's fnext = 0
	ret
      unlink_first:
	cmp	edx,[malloc_flast]
	je	unlink_first_and_last
	mov	[malloc_ffirst],ecx	; ffirst = fnext
	mov	dword [ecx],0		; ffirst's fprev = 0
	ret
      unlink_first_and_last:
	mov	dword [malloc_ffirst],0
	mov	dword [malloc_flast],0
	ret

mfree:
; in: eax - memory block
; out: cf set on error
; preserves: ebx, esi, edi, r12, r13, r14
; note: eax may have value 0 or -1, it should be treated as invalid input then
	test	eax,eax
	jz	mfree_error
	cmp	eax,-1
	je	mfree_error
	mov	ecx,[eax-4]		; size of this block
    mfree_coalesce:
	lea	edx,[eax+ecx-4] 	; pointer to next block size
	cmp	edx,[malloc_lbrk]	; is the next block the end of our program break?
	je	add_to_freelist
	test	dword [edx],1		; is the next block in the freelist?
	jz	add_to_freelist
	mov	r8d,[edx]
	and	r8d,not 1		; clear the freelist bit
	add	ecx,r8d
	mov	[eax-4],ecx		; store the new size of the block
	add	edx,4
	push	rax rcx
	call	unlink_from_freelist
	pop	rcx rax
	jmp	mfree_coalesce
    add_to_freelist:
	mov	r8d,[malloc_ffirst]
	mov	dword [eax],0		; our block's fprev = 0
	mov	[eax+4],r8d		; our block's fnext = ffirst
	test	r8d,r8d
	jz	add_to_empty_freelist
	mov	[malloc_ffirst],eax	; new ffirst = our block
	mov	[r8d],eax		; old ffirst's fprev = our block
	or	dword [eax-4],1 	; set the freelist bit
	clc
	ret
    add_to_empty_freelist:
	mov	[malloc_ffirst],eax	; ffirst = our block
	mov	[malloc_flast],eax	; flast = our block
	or	dword [eax-4],1 	; set the freelist bit
	clc
	ret
    mfree_error:
	stc
	ret

realloc:
; in: eax - memory block, ecx = requested size
; out: eax - resized block, ecx = allocated size, on error jumps to out_of_memory (does not return)
; preserves: ebx, esi, edi
; note: this implementation does not shrink allocations
	mov	edx,[eax-4]
	sub	edx,4
	cmp	ecx,edx
	jbe	realloc_already_satisfied
	lea	r8d,[eax+edx]
	cmp	r8d,[malloc_lbrk]
	je	realloc_brk
	lea	r8d,[edx+12]
	cmp	ecx,r8d
	cmovb	ecx,r8d
	lea	r9d,[4+ecx+MALLOC_MINIMUM_MASK]
	and	r9d,not MALLOC_MINIMUM_MASK
	mov	r8d,[eax+edx]
	btr	r8d,0			; freelist bit set?
	jnc	realloc_and_copy
	add	edx,4
	add	r8d,edx
	cmp	r8d,r9d
	jb	realloc_and_copy	; for simplicity, no coalescing here
	push	rax
	mov	r12d,edi
	mov	r13d,esi
	lea	edi,[eax+edx]
	mov	eax,[edi-4]
	and	eax,not 1
	mov	ecx,r9d
	sub	ecx,edx
	call	malloc_use_block
	pop	rax
	add	ecx,4
	add	ecx,[eax-4]
	mov	[eax-4],ecx
	sub	ecx,4
	ret
   realloc_brk:
	mov	r14d,ecx
	sub	r14d,edx
	mov	r12d,edi
	mov	r13d,esi
	push	rax
	call	malloc_brk
	pop	rax
	mov	edx,[eax-4]
	mov	ecx,edx
	add	ecx,[eax+edx-4]
	mov	[eax-4],ecx
	sub	ecx,4
	ret
   realloc_and_copy:
	push	rax rdi rsi
	call	malloc
	pop	rsi rdi r14
	mov	r12d,edi
	mov	r13d,esi
	mov	edi,eax
	mov	esi,r14d
	mov	edx,[r14d-4]
	sub	edx,4
	add	edi,edx
	add	esi,edx
	neg	edx
      realloc_copy32:
	cmp	edx,-32
	jg	realloc_copy16
	mov	r8,[esi+edx]
	mov	r9,[esi+edx+8]
	mov	[edi+edx],r8
	mov	[edi+edx+8],r9
	mov	r8,[esi+edx+16]
	mov	r9,[esi+edx+24]
	mov	[edi+edx+16],r8
	mov	[edi+edx+24],r9
	add	edx,0x20
	jmp	realloc_copy32
      realloc_copy16:
	cmp	edx,-16
	jg	realloc_copy8
	mov	r8,[esi+edx]
	mov	r9,[esi+edx+8]
	mov	[edi+edx],r8
	mov	[edi+edx+8],r9
	add	edx,0x10
	jz	realloc_copy_done
      realloc_copy8:
	cmp	edx,-8
	jg	realloc_copy4
	mov	r8,[esi+edx]
	mov	[edi+edx],r8
	add	edx,8
	jz	realloc_copy_done
      realloc_copy4:
	cmp	edx,-4
	jg	realloc_copy2
	mov	r8d,[esi+edx]
	mov	[edi+edx],r8d
	add	edx,4
	jz	realloc_copy_done
      realloc_copy2:
	cmp	edx,-2
	jg	realloc_copy1
	movzx	r8d,word [esi+edx]
	mov	[edi+edx],r8w
	add	edx,2
	jz	realloc_copy_done
      realloc_copy1:
	cmp	edx,-1
	jg	realloc_copy_done
	movzx	r8d,byte [esi+edx]
	mov	[edi+edx],r8b
      realloc_copy_done:
	push	rax rcx
	mov	eax,r14d
	call	mfree
	pop	rcx rax
	mov	edi,r12d
	mov	esi,r13d
	ret
    realloc_already_satisfied:
	mov	ecx,edx
	ret

minit:
	mov	eax,12			; sys_brk
	xor	edi,edi
	syscall
	mov	ecx,eax
	cmp	rcx,rax
	jne	minit_done
	mov	[malloc_fbrk],eax
	mov	[malloc_lbrk],eax
    minit_done:
	retn

if used mcheck

    mcheck:
	push	rax rsi
	mov	esi,[malloc_fbrk]
	test	esi,esi
	jz	mcheck_ok
      mcheck_walk:
	cmp	esi,[malloc_lbrk]
	je	mcheck_ok
	ja	mcheck_break
	mov	eax,[esi]
	and	eax,not 1
	test	eax,eax
	jz	mcheck_break
	add	esi,eax
	jmp	mcheck_walk
      mcheck_break:
	jmp	internal_error
      mcheck_ok:
	pop	rsi rax
	ret

end if