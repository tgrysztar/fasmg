
; this is a simple map used primarily for the source cache

struct Map
	hash_mask dd ?
	linked_blocks dd ?
	free_space dd ?
	free_space_length dd ?
ends

struct MapEntry
	name dd ?
	name_length dd ?
	value dd ?
	next_entry dd ?
ends

create_string_map:
; in: cl = number of hash bits
; out: ebx - new map
; preserves: esi
	mov	ebx,1
	shl	ebx,cl
	shl	ebx,2
	lea	ecx,[sizeof.Map+ebx*4]
	call	malloc_fixed
	xchg	ebx,eax
	mov	ecx,eax
	dec	eax
	mov	[ebx+Map.hash_mask],eax
	lea	edi,[ebx+sizeof.Map]
	xor	eax,eax
	rep	stosd
	mov	ecx,1000h
	call	malloc_fixed
	mov	[ebx+Map.linked_blocks],eax
	xor	edx,edx
	mov	[eax],edx
	add	eax,10h
	mov	[ebx+Map.free_space],eax
	mov	[ebx+Map.free_space_length],1000h-10h
	retn

destroy_string_map:
; in: ebx - map
; preserves: esi, edi
	mov	eax,ebx
	mov	ebx,[ebx+Map.linked_blocks]
	call	mfree
    free_map_blocks:
	test	ebx,ebx
	jz	string_map_destroyed
	mov	eax,ebx
	mov	ebx,[ebx]
	call	mfree
	jmp	free_map_blocks
    string_map_destroyed:
	retn

get_from_map:
; in:
;  ebx - map
;  esi - string
;  ecx = string length, zero for ASCIIZ string
; out:
;  eax = value
;  cf set when no entry found
; preserves: ebx, [esi], edi
; note: when entry is found, esi is replaced with pointer to the same string in persistent storage
	call	get_bucket
	test	eax,eax
	jz	not_found_in_map
	call	find_map_entry
	jc	not_found_in_map
	mov	eax,[eax+MapEntry.value]
	retn
    get_bucket:
	call	hash_string
	and	edx,[ebx+Map.hash_mask]
	mov	eax,[ebx+sizeof.Map+edx*4]
	retn
    find_map_entry:
	cmp	[eax+MapEntry.name_length],ecx
	jne	next_map_entry
	push	edi
	mov	edi,[eax+MapEntry.name]
	test	edi,edi
	jz	not_this_map_entry
	push	ecx esi
	repe	cmpsb
	pop	esi ecx
	jne	not_this_map_entry
	mov	esi,edi
	sub	esi,ecx
	pop	edi
	clc
	retn
    not_this_map_entry:
	pop	edi
    next_map_entry:
	mov	eax,[eax+MapEntry.next_entry]
	test	eax,eax
	jnz	find_map_entry
    not_found_in_map:
	stc
	retn

put_into_map:
; in:
;  ebx - map
;  esi - string
;  ecx = string length, zero for ASCIIZ string
;  eax = value
; preserves: ebx, [esi], edi
; note:
;  esi is replaced with pointer to the same string in persistent storage,
;  an ASCIIZ string is a key with length including the terminating zero
;  and when it is put into persistent storage, final zero is copied as well
	push	eax
	call	get_bucket
	test	eax,eax
	jz	new_bucket
	call	find_map_entry
	jnc	put_value_into_map_entry
	mov	eax,[ebx+sizeof.Map+edx*4]
    find_free_map_entry:
	cmp	[eax+MapEntry.name],0
	je	fill_map_entry
	mov	edx,eax
	mov	eax,[eax+MapEntry.next_entry]
	test	eax,eax
	jnz	find_free_map_entry
	call	allocate_map_entry
	mov	[edx+MapEntry.next_entry],eax
	jmp	new_map_entry
    new_bucket:
	call	allocate_map_entry
	mov	[ebx+sizeof.Map+edx*4],eax
    new_map_entry:
	mov	[eax+MapEntry.next_entry],0
    fill_map_entry:
	mov	[eax+MapEntry.name_length],ecx
	push	eax
	call	store_string
	pop	eax
	mov	[eax+MapEntry.name],esi
    put_value_into_map_entry:
	pop	[eax+MapEntry.value]
	retn
    allocate_map_entry:
	mov	eax,[ebx+Map.free_space]
	add	[ebx+Map.free_space],sizeof.MapEntry
	sub	[ebx+Map.free_space_length],sizeof.MapEntry
	jc	map_out_of_free_space
	retn
    map_out_of_free_space:
	push	ecx edx
	mov	ecx,1000h
	call	malloc_fixed
	mov	edx,eax
	xchg	[ebx+Map.linked_blocks],edx
	mov	[eax],edx
	add	eax,10h
	mov	[ebx+Map.free_space],eax
	mov	[ebx+Map.free_space_length],1000h-10h
	pop	edx ecx
	jmp	allocate_map_entry

remove_from_map:
; in:
;  ebx - map
;  esi - string
;  ecx = string length, zero for ASCIIZ string
; preserves: ebx, [esi], edi
	call	get_bucket
	test	eax,eax
	jz	not_found_in_map
	call	find_map_entry
	jc	not_found_in_map
	mov	dword [eax+MapEntry.name],0
	retn

iterate_through_map:
; in:
;  ebx - map
;  edi - callback function
; callback:
;  eax = value
;  esi - string
;  ecx = string length
;  edx - MapEntry
	mov	ecx,[ebx+Map.hash_mask]
	inc	ecx
	add	ebx,sizeof.Map
    iterate_through_hash_table:
	mov	edx,[ebx]
    iterate_through_bucket:
	test	edx,edx
	jz	end_of_bucket
	push	ebx ecx edx edi
	mov	eax,[edx+MapEntry.value]
	mov	esi,[edx+MapEntry.name]
	mov	ecx,[edx+MapEntry.name_length]
	call	edi
	pop	edi edx ecx ebx
	mov	edx,[edx+MapEntry.next_entry]
	jmp	iterate_through_bucket
    end_of_bucket:
	add	ebx,4
	loop	iterate_through_hash_table
	retn

hash_string:
; in: esi - string, ecx = string length, zero for ASCIIZ string
; out: ecx = string length, edx = 32-bit hash
; preserves: ebx, esi, edi
	mov	edx,FNV_OFFSET
	jecxz	hash_asciiz
	mov	eax,ecx
    hash_known_length:
	xor	dl,[esi]
	inc	esi
	imul	edx,FNV_PRIME
	loop	hash_known_length
	mov	ecx,eax
	sub	esi,ecx
	retn
    hash_asciiz:
	inc	ecx
	lodsb
	xor	dl,al
	imul	edx,FNV_PRIME
	test	al,al
	jnz	hash_asciiz
    hash_ready:
	sub	esi,ecx
	retn
