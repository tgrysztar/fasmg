
; a very basic implementation of malloc/realloc
; for porting fasmg to systems that do not have such API natively

struct MemoryHeader
	size dd ?		; total size of this block, the lowest bit set for blocks in use
	preceding_size dd ?	; total size of the block that precedes this one in memory (zero if this is an initial block of address range)
ends

struct FreeMemory
	header MemoryHeader
	right dd ?		; next free block in cyclic list
	left dd ?		; previous free block in cyclic list
ends

VALLOC_MIN = 40000h

valloc:
; in: ecx = requested minimum size
; out: eax - allocated block, ecx = allocated size, zero if failed
; preserves: ebx, esi, edi
; note:
;  this function requests raw memory from the OS;
;  it may allocate much more memory than requested (even entire available memory),
;  the obtained memory is kept indefinitely in the pool for malloc
;  and should be released by OS automatically when the process ends;
;  if the OS does not do it automatically, additional list of the memory areas
;  may need to be maintained to release them before exit
	cmp	ecx,VALLOC_MIN
	jbe	valloc_size_minimum
	dec	ecx
	and	ecx,(-1) shl 12
	add	ecx,1 shl 12
	jmp	valloc_size_ready
    valloc_size_minimum:
	mov	ecx,VALLOC_MIN
    valloc_size_ready:
	push	ebx esi edi
	push	ecx
	mov	ebx,ecx
	shr	ebx,16
	mov	ax,501h
	int	31h
	movzx	eax,cx
	pop	ecx
	jc	valloc_failed
	shl	ebx,16
	or	eax,ebx
	mov	edx,main
	shl	edx,4
	sub	eax,edx
	pop	edi esi ebx
	ret
    valloc_failed:
	xor	ecx,ecx
	pop	edi esi ebx
	retn

malloc:
malloc_fixed:
malloc_growable:
; in: ecx = requested size
; out: eax - allocated block, ecx = allocated size, on error jumps to out_of_memory (does not return)
; preserves: ebx, esi, edi
	add	ecx,sizeof.MemoryHeader-1
	jc	out_of_memory
	and	ecx,(-1) shl 2
	add	ecx,1 shl 2
	jc	out_of_memory
	cmp	ecx,sizeof.FreeMemory
	jae	malloc_size_ok
	mov	ecx,sizeof.FreeMemory
    malloc_size_ok:
	mov	eax,[malloc_freelist]
	test	eax,eax
	jz	malloc_new
    find_fit:
	cmp	ecx,[eax+MemoryHeader.size]
	jbe	malloc_use_block
	mov	eax,[eax+FreeMemory.left]
	cmp	eax,[malloc_freelist]
	jne	find_fit
    malloc_new:
	push	ecx
	add	ecx,sizeof.MemoryHeader
	jc	out_of_memory
	call	valloc
	test	ecx,ecx
	jz	out_of_memory
	xor	edx,edx
	mov	[eax+MemoryHeader.preceding_size],edx
	pop	edx
	push	eax
	sub	ecx,edx
	cmp	ecx,sizeof.FreeMemory+sizeof.MemoryHeader
	jb	no_space_for_free_block
	mov	[eax+MemoryHeader.size],edx
	add	eax,edx
	mov	[eax+MemoryHeader.preceding_size],edx
	mov	edx,ecx
	sub	edx,sizeof.MemoryHeader
	dec	edx
	and	edx,(-1) shl 2
	add	edx,1 shl 2
	mov	[eax+MemoryHeader.size],edx
	mov	ecx,[malloc_freelist]
	jecxz	freelist_empty
	mov	[eax+FreeMemory.left],ecx
	mov	edx,eax
	xchg	edx,[ecx+FreeMemory.right]
	mov	[eax+FreeMemory.right],edx
	mov	[edx+FreeMemory.left],eax
	mov	edx,[eax+MemoryHeader.size]
	jmp	free_block_ready
    no_space_for_free_block:
	sub	ecx,sizeof.MemoryHeader
	add	edx,ecx
	mov	[eax+MemoryHeader.size],edx
	jmp	append_limiter
    freelist_empty:
	mov	[eax+FreeMemory.left],eax
	mov	[eax+FreeMemory.right],eax
    free_block_ready:
	mov	[malloc_freelist],eax
    append_limiter:
	add	eax,edx
	mov	[eax+MemoryHeader.preceding_size],edx
	mov	[eax+MemoryHeader.size],sizeof.MemoryHeader or 1	; cannot be freed
	pop	eax
    finish_malloc:
	mov	ecx,[eax+MemoryHeader.size]
	or	[eax+MemoryHeader.size],1
	add	eax,sizeof.MemoryHeader
	sub	ecx,sizeof.MemoryHeader
	retn
    malloc_use_block:
	mov	edx,[eax+MemoryHeader.size]
	sub	edx,ecx
	cmp	edx,sizeof.FreeMemory
	jb	use_whole_block
	mov	[eax+MemoryHeader.size],ecx
	mov	[eax+ecx+MemoryHeader.preceding_size],ecx
	add	ecx,eax
	mov	[malloc_freelist],ecx
	mov	[ecx+MemoryHeader.size],edx
	mov	[ecx+edx+MemoryHeader.preceding_size],edx
	mov	edx,[eax+FreeMemory.right]
	cmp	edx,eax
	je	update_free_singleton
	mov	[ecx+FreeMemory.right],edx
	mov	[edx+FreeMemory.left],ecx
	mov	edx,[eax+FreeMemory.left]
	mov	[ecx+FreeMemory.left],edx
	mov	[edx+FreeMemory.right],ecx
	jmp	finish_malloc
    update_free_singleton:
	mov	[ecx+FreeMemory.left],ecx
	mov	[ecx+FreeMemory.right],ecx
	jmp	finish_malloc
    use_whole_block:
	mov	edx,[eax+FreeMemory.right]
	cmp	edx,eax
	je	depleted_freelist
	mov	ecx,[eax+FreeMemory.left]
	mov	[ecx+FreeMemory.right],edx
	mov	[edx+FreeMemory.left],ecx
	mov	[malloc_freelist],edx
    depleted_freelist:
	and	[malloc_freelist],0
	jmp	finish_malloc

mfree:
; in: eax - memory block
; out: cf set on error
; preserves: ebx, esi, edi
; note: eax may have value 0 or -1, it should be treated as invalid input then
	test	eax,eax
	jz	mfree_error
	cmp	eax,-1
	je	mfree_error
	sub	eax,sizeof.MemoryHeader
	mov	ecx,[eax+MemoryHeader.size]
	btr	ecx,0
	jnc	mfree_error
	cmp	ecx,sizeof.FreeMemory
	jb	mfree_error
	cmp	[eax+ecx+MemoryHeader.preceding_size],ecx
	jne	mfree_error
	mov	[eax+MemoryHeader.size],ecx
	mov	edx,eax
	sub	edx,[eax+MemoryHeader.preceding_size]
	cmp	edx,eax
	je	no_preceding_block
	test	[edx+MemoryHeader.size],1
	jz	coalesce_with_preceding_block
    no_preceding_block:
	test	[eax+ecx+MemoryHeader.size],1
	jz	coalesce_with_following_block
	mov	ecx,[malloc_freelist]
	jecxz	mfree_init_freelist
	mov	edx,[ecx+FreeMemory.right]
	mov	[eax+FreeMemory.left],ecx
	mov	[edx+FreeMemory.left],eax
	mov	[eax+FreeMemory.right],edx
	mov	[ecx+FreeMemory.right],eax
    mfree_ok:
	mov	[malloc_freelist],eax
	clc
	retn
    mfree_init_freelist:
	mov	[eax+FreeMemory.left],eax
	mov	[eax+FreeMemory.right],eax
	jmp	mfree_ok
    mfree_error:
	stc
	retn
    coalesce_with_preceding_block:
	add	ecx,[edx+MemoryHeader.size]
	test	[edx+ecx+MemoryHeader.size],1
	jz	coalesce_on_both_ends
	mov	[edx+MemoryHeader.size],ecx
	mov	[edx+ecx+MemoryHeader.preceding_size],ecx
	clc
	retn
    coalesce_on_both_ends:
	lea	eax,[edx+ecx]
	add	ecx,[eax+MemoryHeader.size]
	mov	[edx+MemoryHeader.size],ecx
	mov	[edx+ecx+MemoryHeader.preceding_size],ecx
	mov	[malloc_freelist],edx
	mov	ecx,[eax+FreeMemory.left]
	mov	edx,[eax+FreeMemory.right]
	mov	[edx+FreeMemory.left],ecx
	mov	[ecx+FreeMemory.right],edx
	clc
	retn
    coalesce_with_following_block:
	push	ebx
	lea	ebx,[eax+ecx]
	add	ecx,[ebx+MemoryHeader.size]
	mov	[eax+MemoryHeader.size],ecx
	mov	[eax+ecx+MemoryHeader.preceding_size],ecx
	mov	ecx,[ebx+FreeMemory.left]
	mov	edx,[ebx+FreeMemory.right]
	mov	[ecx+FreeMemory.right],eax
	mov	[edx+FreeMemory.left],eax
	mov	ecx,[ebx+FreeMemory.left]
	mov	edx,[ebx+FreeMemory.right]
	mov	[eax+FreeMemory.left],ecx
	mov	[eax+FreeMemory.right],edx
	pop	ebx
	jmp	mfree_ok

realloc:
; in: eax - memory block, ecx = requested size
; out: eax - resized block, ecx = allocated size, on error jumps to out_of_memory (does not return)
; preserves: ebx, esi, edi
	add	ecx,sizeof.MemoryHeader-1
	jc	out_of_memory
	and	ecx,(-1) shl 2
	add	ecx,1 shl 2
	jc	out_of_memory
	sub	eax,sizeof.MemoryHeader
	mov	edx,[eax+MemoryHeader.size]
	and	edx,not 1
	cmp	ecx,edx
	jbe	realloc_retain
	test	[eax+edx+MemoryHeader.size],1
	jnz	realloc_and_copy
	add	edx,[eax+edx+MemoryHeader.size]
	cmp	ecx,edx
	ja	realloc_and_copy
	sub	edx,ecx
	cmp	edx,sizeof.FreeMemory
	jb	append_whole_block
	push	esi edi
	push	ecx edx
	lea	edi,[eax+ecx]
	xchg	ecx,[eax+MemoryHeader.size]
	and	ecx,not 1
	lea	esi,[eax+ecx]
	mov	ecx,[esi+FreeMemory.left]
	mov	edx,[esi+FreeMemory.right]
	mov	[edx+FreeMemory.left],edi
	mov	[ecx+FreeMemory.right],edi
	mov	ecx,[esi+FreeMemory.left]
	mov	edx,[esi+FreeMemory.right]
	mov	[edi+FreeMemory.left],ecx
	mov	[edi+FreeMemory.right],edx
	mov	[malloc_freelist],edi
	pop	edx ecx
	mov	[edi+MemoryHeader.size],edx
	mov	[edi+edx+MemoryHeader.preceding_size],edx
	mov	[edi+MemoryHeader.preceding_size],ecx
	pop	edi esi
	jmp	finish_malloc
    append_whole_block:
	add	edx,ecx
	mov	[eax+edx+MemoryHeader.preceding_size],edx
	xchg	edx,[eax+MemoryHeader.size]
	and	edx,not 1
	add	edx,eax
	mov	ecx,[edx+FreeMemory.left]
	cmp	ecx,edx
	je	depleted_freelist
	mov	edx,[edx+FreeMemory.right]
	mov	[ecx+FreeMemory.right],edx
	mov	[edx+FreeMemory.left],ecx
	mov	[malloc_freelist],ecx
	jmp	finish_malloc
    realloc_retain:
	and	[eax+MemoryHeader.size],not 1
	jmp	finish_malloc
    realloc_and_copy:
	push	esi edi
	lea	esi,[eax+sizeof.MemoryHeader]
	call	malloc_growable
	push	eax ecx
	mov	edi,eax
	mov	eax,esi
	mov	ecx,[esi-sizeof.MemoryHeader+MemoryHeader.size]
	shr	ecx,2
	rep	movsd
	call	mfree
	pop	ecx eax
	pop	edi esi
	retn

if used mcheck

    mcheck:
	pushfd
	pushad
	mov	eax,[malloc_freelist]
	test	eax,eax
	jz	integrity_verified
      verify_freelist:
	mov	ebx,eax
      verify_preceding_blocks:
	mov	ecx,[ebx+MemoryHeader.preceding_size]
	jecxz	preceding_blocks_ok
	sub	ebx,ecx
	mov	edx,[ebx+MemoryHeader.size]
	and	edx,not 1
	cmp	ecx,edx
	je	verify_preceding_blocks
	jmp	internal_error
      preceding_blocks_ok:
	mov	ebx,eax
      verify_following_blocks:
	mov	ecx,[ebx+MemoryHeader.size]
	and	ecx,not 1
	cmp	ecx,sizeof.MemoryHeader
	je	following_blocks_ok
	add	ebx,ecx
	cmp	ecx,[ebx+MemoryHeader.preceding_size]
	je	verify_following_blocks
	jmp	internal_error
      following_blocks_ok:
	mov	edx,[eax+FreeMemory.right]
	cmp	eax,[edx+FreeMemory.left]
	je	verify_next
	jmp	internal_error
      verify_next:
	mov	eax,edx
	cmp	eax,[malloc_freelist]
	jne	verify_freelist
      integrity_verified:
	popad
	popfd
	retn

end if
