
struct SymbolTree_Root
	attributes db ? 		; SYMTREE_#
	flags db ?			; NAMESPACE_#
	parameters db ? 		; SPECPARM_#
	reserved db ?
	parent_branch dd ?		; pointer to namespace SymbolTree_Foliage
	current_label dd ?		; pointer to selected SymbolTree_Foliage
	chain dd ?			; pointer to another SymbolTree_Root
      ; root_node SymbolTree_Node
ends

struct SymbolTree_Node
	branches rd 1 shl TREE_NODE_BITS
ends

struct SymbolTree_LocalNode
	branches rd 1 shl LOCAL_TREE_NODE_BITS
ends

struct SymbolTree_Foliage
	name_kind db ?			; NAME_#
	flags db ?			; FOLIAGE_#
	reserved dw ?
	name_length dd ?
	name_data dd ?
	root dd ?			; pointer to SymbolTree_Root
	next dd ?			; pointer to another SymbolTree_Foliage
	child_namespace dd ?		; pointer to descendant SymbolTree_Root
      ; first_leaf SymbolTree_Leaf
ends

struct SymbolTree_Leaf
	class db ?			; SYMCLASS_#
	flags db ?			; SYM_#
	reserved dw ?
	definition dd ? 		; pointer to ValueDefinition
	retired_definition dd ?
	last_use_pass dd ?
	next dd ?			; pointer to another SymbolTree_Leaf within SymbolTree_Foliage
	branch dd ?			; pointer to SymbolTree_Foliage
	fallback_neighbour dd ? 	; pointer to another SymbolTree_Leaf
	fallback_parent dd ?		; pointer to another SymbolTree_Leaf
ends

struct ValueDefinition
	type db ?			; VALTYPE_#
	flags db ?			; VAL_#
	attribute db ?
	reserved db ?
	pass dd ?
	value dd ?
	value_length dd ?
	block_length dd ?
	reference_count dd ?		; number of distinct references to current value
	previous dd ?			; pointer to another ValueDefinition
	interlink dd ?
ends

TREE_HEIGHT = 4
TREE_NODE_BITS = 5
LOCAL_TREE_HEIGHT = 1
LOCAL_TREE_NODE_BITS = 6

FNV_PRIME = 16777619
FNV_OFFSET = 2166136261

SYMTREE_LOCAL = 1
SYMTREE_WITH_CASEINSENSITIVE_PARAMETERS = 2

NAMESPACE_UNATTACHED = 1
NAMESPACE_CURRENT_LABEL_TO_CONFIRM = 2
NAMESPACE_CALM = 10h
NAMESPACE_LOCAL = 20h

SPECPARM_COUNTER = 1
SPECPARM_LIMIT = 2

FOLIAGE_WITH_TOKEN = 1

NAME_CASESENSITIVE = 0
NAME_CASEINSENSITIVE = 1
NAME_NUMERIC = 2
NAME_ABSTRACT = 3

SYMCLASS_EXPRESSION = 0
SYMCLASS_INSTRUCTION = 1
SYMCLASS_STRUCTURE = 2
SYMCLASS_PARAMETER = 3
SYMCLASS_CALM_LOCATION = 10h

SYM_CONSTANT = 1
SYM_VARIABLE = 2
SYM_PREDICTED = 4
SYM_PREDICTED_DEFINED = 8
SYM_USAGE_PREDICTED = 10h
SYM_PREDICTED_USED = 20h
SYM_LINK = 40h
SYM_LINK_PREDICTED = 80h

VALTYPE_RESERVED = 0
VALTYPE_SYMBOLIC = 1
VALTYPE_NUMERIC = 2
VALTYPE_STRING = 3
VALTYPE_FLOAT = 4
VALTYPE_ELEMENT = 5
VALTYPE_AREA = 6
VALTYPE_CALM = 7
VALTYPE_SYMBOLIC_SEQUENCE = 11h
VALTYPE_NUMERIC_SEQUENCE = 12h
VALTYPE_PLAIN = 20h
VALTYPE_NATIVE_COMMAND = 40h
VALTYPE_NATIVE_COMPARATOR = 41h
VALTYPE_NATIVE_FUNCTION = 42h
VALTYPE_NATIVE_PREPOSITION = 43h

VAL_INTERNAL = 1
VAL_IN_USE = 2
VAL_UNCONDITIONAL = 4			; for SYMCLASS_INSTRUCTION and SYMCLASS_STRUCTURE
VAL_UNARY = 4				; for SYMCLASS_EXPRESSION
VAL_NONRECURSIVE = 8
VAL_SHIFTABLE = 10h
VAL_DETACHED = 20h

RECOGNIZE_CASE_INSENSITIVE = 1
RECOGNIZE_DEFINITION = 2

recognize_symbol:
; in:
;  ebx - SymbolTree_Root (namespace), null for standard recognition regime
;  ecx = name length
;  esi - name followed by two hashes (as in name token)
;  dl = SYMCLASS_#
;  dh = any combination of RECOGNIZE_# flags
;  [recognition_context.base_namespace] - namespace for standard recognition regime
;  [name_volatile] = non-zero when name is provided in a temporary storage
;  [name_token] = null or pointer to the contents of a preprocessed token
; out:
;  ebx - SymbolTree_Leaf
;  edx - SymbolTree_Foliage
;  edi - SymbolTree_Root
;  [name_volatile] zeroed when name has been moved to persistent storage
; note:
;  when RECOGNIZE_DEFINITION option is not selected and no defined symbol of requested class is found,
;  the provided result is as if RECOGNIZE_DEFINITION option was specified and expression class requested
	mov	[symbol_class],dl
	test	dh,RECOGNIZE_DEFINITION
	setnz	[symbol_expected]
	or	[symbol_required],1
	test	dh,RECOGNIZE_CASE_INSENSITIVE
	mov	al,NAME_CASEINSENSITIVE
	mov	edx,[esi+ecx+4]
	mov	[case_insensitive_hash],edx
	jnz	name_kind_ok
	mov	al,NAME_CASESENSITIVE
	mov	edx,[esi+ecx]
	mov	[case_sensitive_hash],edx
    name_kind_ok:
	mov	[name_kind],al
	test	ebx,ebx
	jz	standard_recognition
	call	scan_namespace
	jnc	symbol_recognized
	mov	[kept_symbol],ebx
	mov	[kept_symbol_flags],al
	mov	edi,[symbol_root]
	cmp	[name_kind],NAME_CASEINSENSITIVE
	je	no_defined_symbol
	mov	[name_kind],NAME_CASEINSENSITIVE
	mov	edx,[case_insensitive_hash]
	mov	ebx,edi
	call	scan_namespace
	jc	no_defined_symbol
    symbol_recognized:
	mov	edi,[symbol_root]
	retn
    standard_recognition:
	mov	ebx,[recognition_context.base_namespace]
	call	scan_namespace
	jnc	symbol_recognized
	mov	[kept_symbol],ebx
	mov	[kept_symbol_flags],al
	mov	edi,[symbol_root]
    find_in_wider_scope:
	mov	edx,[case_insensitive_hash]
	mov	[current_symbol],ebx
	cmp	[name_kind],NAME_CASEINSENSITIVE
	je	find_in_namespace_chain
	mov	ebx,[ebx+SymbolTree_Leaf.fallback_neighbour]
	test	ebx,ebx
	jnz	check_fallback_neighbour
	mov	[name_kind],NAME_CASEINSENSITIVE
	mov	ebx,[symbol_root]
	call	scan_namespace
	mov	eax,[current_symbol]
	mov	[eax+SymbolTree_Leaf.fallback_neighbour],ebx
	jnc	symbol_recognized
    no_neighbour_found:
	mov	ebx,[current_symbol]
	mov	[name_kind],NAME_CASESENSITIVE
	mov	edx,[case_sensitive_hash]
    find_in_namespace_chain:
	mov	ebx,[ebx+SymbolTree_Leaf.fallback_parent]
	test	ebx,ebx
	jnz	check_fallback_parent
	mov	edi,[symbol_root]
	mov	ebx,[edi+SymbolTree_Root.parent_branch]
	test	ebx,ebx
	jz	no_defined_symbol
	mov	ebx,[ebx+SymbolTree_Foliage.root]
	call	scan_namespace
	mov	eax,[current_symbol]
	mov	[eax+SymbolTree_Leaf.fallback_parent],ebx
	jc	find_in_wider_scope
	mov	edi,[symbol_root]
	retn
    check_fallback_neighbour:
	call	get_available_value
	jc	no_neighbour_found
	mov	edx,[ebx+SymbolTree_Leaf.branch]
	mov	edi,[edx+SymbolTree_Foliage.root]
	retn
    check_fallback_parent:
	call	get_available_value
	mov	edx,[ebx+SymbolTree_Leaf.branch]
	mov	edi,[edx+SymbolTree_Foliage.root]
	mov	[symbol_root],edi
	jc	find_in_wider_scope
	retn
    no_defined_symbol:
	cmp	[symbol_class],SYMCLASS_EXPRESSION
	je	return_kept_symbol
	mov	[symbol_class],SYMCLASS_EXPRESSION
	or	[symbol_expected],1
	mov	ebx,[kept_symbol]
	mov	ebx,[ebx+SymbolTree_Leaf.branch]
	mov	edi,[ebx+SymbolTree_Foliage.root]
	jmp	scan_symbol_branch
    return_kept_symbol:
	mov	ebx,[kept_symbol]
	mov	edx,[ebx+SymbolTree_Leaf.branch]
	mov	edi,[edx+SymbolTree_Foliage.root]
	mov	al,[kept_symbol_flags]
	mov	[ebx+SymbolTree_Leaf.flags],al
	retn

scan_namespace:
; in:
;  ebx - SymbolTree_Root
;  edx = hash
;  ecx = name length
;  esi - name
;  [name_kind] = NAME_#
;  [name_volatile] = non-zero when name is provided in a temporary storage
;  [name_token] = null or pointer to the contents of a preprocessed token
;  [symbol_class] = SYMCLASS_#
;  [symbol_required] = non-zero when symbol needs to be added if not found
;  [symbol_expected] = non-zero when symbol needs not be checked for its value availability
; out:
;  ebx - SymbolTree_Leaf, null when nothing found
;  edx - SymbolTree_Foliage, null when no such branch exists
;  [symbol_root] - SymbolTree_Root
;  [symbol_branch] - SymbolTree_Foliage
;  [name_volatile] zeroed when name has been moved to persistent storage
;  when [symbol_expected] = 0:
;   cf set when symbol not found or has no defined value
;   al = copy of symbol prediction flags before they were affected by this function
;  when [symbol_expected] = 1:
;   cf set when symbol not found
; preserves: ecx, [esi]
	mov	[symbol_root],ebx
	test	[ebx+SymbolTree_Root.attributes],SYMTREE_LOCAL
	jnz	scan_local_namespace
	add	ebx,sizeof.SymbolTree_Root
    if TREE_HEIGHT > 1
	mov	edi,TREE_HEIGHT
    follow_tree:
	mov	eax,edx
	and	eax,(1 shl TREE_NODE_BITS)-1
	shr	edx,TREE_NODE_BITS
	lea	ebx,[ebx+eax*4]
    else
	and	edx,(1 shl TREE_NODE_BITS)-1
	lea	ebx,[ebx+edx*4]
    end if
	mov	eax,[ebx]
	test	eax,eax
	jz	unregistered_hash
	mov	ebx,eax
    if TREE_HEIGHT > 1
	dec	edi
	jnz	follow_tree
    end if
    scan_foliage_branches:
	movzx	eax,[name_kind]
	cmp	[ebx+SymbolTree_Foliage.name_kind],al
	jne	next_foliage_branch
	cmp	[ebx+SymbolTree_Foliage.name_length],ecx
	jne	next_foliage_branch
	mov	edi,[ebx+SymbolTree_Foliage.name_data]
	cmp	esi,edi
	je	scan_symbol_branch
	cmp	al,NAME_ABSTRACT
	je	next_foliage_branch
	jecxz	scan_symbol_branch
	repe	cmpsb
	jne	names_not_identical
	mov	esi,edi
	mov	ecx,[ebx+SymbolTree_Foliage.name_length]
	sub	esi,ecx
	and	[name_volatile],0
	mov	edx,[name_token]
	test	edx,edx
	jz	scan_symbol_branch
	test	[ebx+SymbolTree_Foliage.flags],FOLIAGE_WITH_TOKEN
	jz	use_token_for_foliage
	lea	eax,[esi-4]
	xchg	[edx],eax
	lea	edx,[esi-4]
	mov	[eax+4+ecx+8],edx
	jmp	scan_symbol_branch
    use_token_for_foliage:
	mov	esi,[edx]
	add	esi,4
	mov	[ebx+SymbolTree_Foliage.name_data],esi
	or	[ebx+SymbolTree_Foliage.flags],FOLIAGE_WITH_TOKEN
	jmp	scan_symbol_branch
    hash_collision:
	add	esi,ecx
	mov	ecx,[ebx+SymbolTree_Foliage.name_length]
	sub	esi,ecx
    next_foliage_branch:
	mov	eax,[ebx+SymbolTree_Foliage.next]
	test	eax,eax
	jz	unregistered_name
	mov	ebx,eax
	jmp	scan_foliage_branches
    names_not_identical:
	cmp	al,NAME_CASEINSENSITIVE
	jne	hash_collision
	dec	esi
	dec	edi
	inc	ecx
    case_insensitive_compare:
	lodsb
	mov	dl,[characters+eax]
	mov	al,[edi]
	inc	edi
	dec	ecx
	cmp	dl,[characters+eax]
	jne	hash_collision
	test	ecx,ecx
	jnz	case_insensitive_compare
	mov	ecx,[ebx+SymbolTree_Foliage.name_length]
	sub	esi,ecx
    scan_symbol_branch:
    ; in:
    ;  ebx - SymbolTree_Foliage
    ;  [symbol_class] = SYMCLASS_#
    ;  [symbol_required] = non-zero when symbol needs to be added if not found
    ;  [symbol_expected] = non-zero when symbol needs not be checked for its value availability
    ; out:
    ;  ebx - SymbolTree_Leaf, null when nothing found
    ;  edx - SymbolTree_Foliage
    ;  [symbol_branch] - SymbolTree_Foliage
    ;  when [symbol_expected] = 0:
    ;	cf set when symbol not found or has no defined value
    ;	al = copy of symbol prediction flags before they were affected by this function
    ;  when [symbol_expected] = 1:
    ;	cf set when symbol not found
    ; preserves: ecx, esi, edi
	mov	[symbol_branch],ebx
	add	ebx,sizeof.SymbolTree_Foliage
	mov	dl,[symbol_class]
    scan_leaves:
	mov	al,[ebx+SymbolTree_Leaf.class]
	cmp	al,dl
	je	leaf_found
	mov	eax,[ebx+SymbolTree_Leaf.next]
	test	eax,eax
	jz	leaves_scanned
	mov	ebx,eax
	jmp	scan_leaves
    leaf_found:
	cmp	[symbol_expected],0
	jne	symbol_found
	call	get_available_value
	jnc	symbol_found
    no_defined_symbol_found:
	mov	edx,[symbol_branch]
	stc
	retn
    symbol_found:
	mov	edx,[symbol_branch]
	clc
	retn
    leaves_scanned:
	mov	al,[symbol_required]
	or	al,[symbol_expected]
	jnz	attach_symbol_leaf
	xor	ebx,ebx
	jmp	no_defined_symbol_found
    unregistered_name:
	cmp	[symbol_required],0
	je	name_not_found
	push	ecx
	lea	ebx,[ebx+SymbolTree_Foliage.next]
	jmp	attach_foliage_branch
    name_not_found:
	xor	ebx,ebx
	xor	edx,edx
	stc
	retn
    unregistered_hash:
	cmp	[symbol_required],0
	je	name_not_found
	push	ecx
    if TREE_HEIGHT > 1
    expand_tree:
	dec	edi
	jz	attach_foliage_branch
	mov	ecx,sizeof.SymbolTree_Node
	call	create_tree_element
	mov	[ebx],eax
	mov	ebx,eax
	mov	eax,edx
	and	eax,(1 shl TREE_NODE_BITS)-1
	shr	edx,TREE_NODE_BITS
	lea	ebx,[ebx+eax*4]
	jmp	expand_tree
    end if
    attach_foliage_branch:
	mov	ecx,sizeof.SymbolTree_Foliage + sizeof.SymbolTree_Leaf
	call	create_tree_element
	mov	[ebx],eax
	mov	[symbol_branch],eax
	mov	ebx,eax
	pop	ecx
	mov	eax,[symbol_root]
	mov	[ebx+SymbolTree_Foliage.root],eax
	mov	al,[name_kind]
	mov	[ebx+SymbolTree_Foliage.name_kind],al
	mov	[ebx+SymbolTree_Foliage.name_length],ecx
	cmp	al,NAME_ABSTRACT
	je	symbol_name_stored
	mov	edx,[name_token]
	test	edx,edx
	jnz	symbol_name_from_token
	cmp	[name_volatile],0
	je	symbol_name_stored
	call	store_string
	and	[name_volatile],0
	jmp	symbol_name_stored
    symbol_name_from_token:
	or	[ebx+SymbolTree_Foliage.flags],FOLIAGE_WITH_TOKEN
	mov	esi,[edx]
	add	esi,4
    symbol_name_stored:
	mov	[ebx+SymbolTree_Foliage.name_data],esi
	lea	ebx,[ebx+sizeof.SymbolTree_Foliage]
	jmp	fill_new_symbol_leaf
    attach_symbol_leaf:
	push	ecx
	mov	ecx,sizeof.SymbolTree_Leaf
	call	create_tree_element
	mov	[ebx+SymbolTree_Leaf.next],eax
	mov	ebx,eax
	pop	ecx
    fill_new_symbol_leaf:
	mov	al,[symbol_class]
	mov	[ebx+SymbolTree_Leaf.class],al
	mov	edx,[symbol_branch]
	cmp	al,SYMCLASS_PARAMETER
	jne	namespace_flags_updated
	cmp	[name_kind],NAME_CASEINSENSITIVE
	jne	namespace_flags_updated
	mov	eax,[edx+SymbolTree_Foliage.root]
	or	[eax+SymbolTree_Root.attributes],SYMTREE_WITH_CASEINSENSITIVE_PARAMETERS
    namespace_flags_updated:
	mov	[ebx+SymbolTree_Leaf.branch],edx
	cmp	[symbol_expected],0
	jne	symbol_found
	xor	al,al
	or	[ebx+SymbolTree_Leaf.flags],SYM_PREDICTED
	jmp	no_defined_symbol_found
    scan_local_namespace:
	add	ebx,sizeof.SymbolTree_Root
    if LOCAL_TREE_HEIGHT > 1
	mov	edi,LOCAL_TREE_HEIGHT
    follow_local_tree:
	mov	eax,edx
	and	eax,(1 shl LOCAL_TREE_NODE_BITS)-1
	shr	edx,LOCAL_TREE_NODE_BITS
	lea	ebx,[ebx+eax*4]
    else
	and	edx,(1 shl LOCAL_TREE_NODE_BITS)-1
	lea	ebx,[ebx+edx*4]
    end if
	mov	eax,[ebx]
	test	eax,eax
	jz	local_unregistered_hash
	mov	ebx,eax
    if LOCAL_TREE_HEIGHT > 1
	dec	edi
	jnz	follow_local_tree
    end if
	jmp	scan_foliage_branches
    local_unregistered_hash:
	cmp	[symbol_required],0
	je	name_not_found
	push	ecx
    if LOCAL_TREE_HEIGHT > 1
    expand_local_tree:
	dec	edi
	jz	attach_foliage_branch
	mov	ecx,sizeof.SymbolTree_LocalNode
	call	create_tree_element
	mov	[ebx],eax
	mov	ebx,eax
	mov	eax,edx
	and	eax,(1 shl LOCAL_TREE_NODE_BITS)-1
	shr	edx,LOCAL_TREE_NODE_BITS
	lea	ebx,[ebx+eax*4]
	jmp	expand_local_tree
    else
	jmp	attach_foliage_branch
    end if

get_abstract_symbol:
; in:
;  eax:ecx = symbol identifier
;  ebx - SymbolTree_Root, null for local scope
;  dl = SYMCLASS_#
; out:
;  ebx - SymbolTree_Leaf
;  edx - SymbolTree_Foliage
;  edi - SymbolTree_Root
	mov	[symbol_class],dl
	test	ebx,ebx
	jnz	scope_selected
	mov	ebx,[current_context.base_namespace]
    scope_selected:
	mov	esi,eax
	mov	edx,FNV_OFFSET
	mov	[minor_identifier],ecx
	mov	ecx,4
    hash_major_identifier:
	xor	dl,al
	imul	edx,FNV_PRIME
	shr	eax,8
	loop	hash_major_identifier
	mov	eax,[minor_identifier]
	mov	ecx,4
    hash_minor_identifier:
	xor	dl,al
	imul	edx,FNV_PRIME
	shr	eax,8
	loop	hash_minor_identifier
	mov	ecx,[minor_identifier]
	mov	[name_kind],NAME_ABSTRACT
	or	[symbol_required],1
	or	[symbol_expected],1
	call	scan_namespace
	mov	edi,[symbol_root]
	retn

create_tree_element:
; in: ecx = length of element
; out: eax - pointer to allocated and zeroed memory
; preserves: ebx, ecx, edx, esi, edi
	sub	[tree_reserve_length],ecx
	jc	expand_tree_reserve
	mov	eax,[tree_reserve]
	add	[tree_reserve],ecx
	retn
    expand_tree_reserve:
	push	ecx edx edi
	mov	ecx,10000h
	call	malloc_fixed
	mov	edx,eax
	xchg	edx,[tree_blocks]
	mov	[eax],edx
	lea	edi,[eax+4]
	sub	ecx,4
	mov	[tree_reserve],edi
	mov	[tree_reserve_length],ecx
	mov	edx,eax
	xor	eax,eax
	shr	ecx,2
	rep	stosd
	mov	eax,edx
	pop	edi edx ecx
	jmp	create_tree_element

store_string:
; in: esi - string, ecx = length
; out: esi - stored string
; preserves: ebx, ecx, edi
	cmp	ecx,[storage_free_space_length]
	ja	expand_storage
    storage_ready:
	push	ecx edi
	mov	edi,[storage_free_space]
	add	[storage_free_space],ecx
	sub	[storage_free_space_length],ecx
    move_to_storage:
	mov	eax,edi
	shr	ecx,1
	jnc	movsb_ok
	movsb
    movsb_ok:
	shr	ecx,1
	jnc	movsw_ok
	movsw
    movsw_ok:
	rep	movsd
	mov	esi,eax
	pop	edi ecx
	retn
    expand_storage:
	cmp	ecx,100h
	jae	store_long_string
	push	ecx
	mov	ecx,10000h
	call	malloc_fixed
	mov	edx,eax
	xchg	edx,[storage_blocks]
	mov	[eax],edx
	add	eax,4
	sub	ecx,4
	mov	[storage_free_space],eax
	mov	[storage_free_space_length],ecx
	pop	ecx
	jmp	storage_ready
    store_long_string:
	push	ecx
	add	ecx,4
	call	malloc_fixed
	mov	edx,[storage_blocks]
	test	edx,edx
	jz	long_first_string
	mov	ecx,eax
	xchg	ecx,[edx]
	mov	[eax],ecx
    long_string_storage_ready:
	add	eax,4
	mov	ecx,[esp]
	push	edi
	mov	edi,eax
	jmp	move_to_storage
    long_first_string:
	mov	[storage_blocks],eax
	mov	[eax],edx
	jmp	long_string_storage_ready

get_symbol_namespace:
; in:
;  edx - SymbolTree_Foliage
; out:
;  ebx - SymbolTree_Root of child namespace
; preserves: edx, esi, edi
	mov	ebx,[edx+SymbolTree_Foliage.child_namespace]
	test	ebx,ebx
	jnz	symbol_namespace_ok
	mov	ecx,sizeof.SymbolTree_Root + sizeof.SymbolTree_Node
	call	create_tree_element
	mov	[eax+SymbolTree_Root.parent_branch],edx
	mov	[edx+SymbolTree_Foliage.child_namespace],eax
	mov	ebx,eax
    symbol_namespace_ok:
	retn
get_local_namespace:
; same as get_symbol_namespace
	mov	ebx,[edx+SymbolTree_Foliage.child_namespace]
	test	ebx,ebx
	jnz	symbol_namespace_ok
	mov	ecx,sizeof.SymbolTree_Root + sizeof.SymbolTree_LocalNode
	call	create_tree_element
	or	[eax+SymbolTree_Root.attributes],SYMTREE_LOCAL
	or	[eax+SymbolTree_Root.flags],NAMESPACE_LOCAL
	mov	[edx+SymbolTree_Foliage.child_namespace],eax
	mov	ebx,eax
	retn

get_local_anchor:
; in:
;  eax = instruction identifier
;  ecx = context identifier
; out:
;  edx - SymbolTree_Foliage where a local namespace can be anchored
	xor	ebx,ebx
	mov	dl,SYMCLASS_PARAMETER
	call	get_abstract_symbol
	mov	eax,[ebx+SymbolTree_Leaf.definition]
	test	eax,eax
	jz	create_anchor_counter
	mov	ecx,[current_pass]
	cmp	ecx,[eax+ValueDefinition.pass]
	je	increment_anchor_counter
	mov	[eax+ValueDefinition.pass],ecx
	and	[eax+ValueDefinition.value],0
	jmp	increment_anchor_counter
    create_anchor_counter:
	mov	ecx,sizeof.ValueDefinition
	call	create_tree_element
	mov	[ebx+SymbolTree_Leaf.definition],eax
	inc	[eax+ValueDefinition.reference_count]
	mov	[eax+ValueDefinition.type],VALTYPE_PLAIN
	mov	ecx,[current_pass]
	mov	[eax+ValueDefinition.pass],ecx
    increment_anchor_counter:
	mov	ecx,[eax+ValueDefinition.value]
	inc	[eax+ValueDefinition.value]
	jecxz	local_anchor_ready
	mov	eax,ebx
	xor	ebx,ebx
	mov	dl,SYMCLASS_PARAMETER
	call	get_abstract_symbol
    local_anchor_ready:
	retn

get_available_value:
; in:
;  ebx - SymbolTree_Leaf
; out:
;  cf set if symbol is not considered defined
;  edx - ValueDefinition
;  al = copy of symbol prediction flags before they were affected by this function
; preserves: ebx, ecx, esi, edi
	mov	edx,[ebx+SymbolTree_Leaf.definition]
	test	[ebx+SymbolTree_Leaf.flags],SYM_VARIABLE
	jnz	get_variable_value
	test	edx,edx
	jz	symbol_predicted_undefined
	mov	al,[edx+ValueDefinition.flags]
	test	al,VAL_INTERNAL
	jnz	symbol_defined
	not	al
	test	al,VAL_IN_USE + VAL_NONRECURSIVE
	jz	symbol_undefined
	mov	eax,[current_pass]
	sub	eax,[edx+ValueDefinition.pass]
	jz	symbol_defined
	cmp	eax,1
	ja	symbol_predicted_undefined
    symbol_predicted_defined:
	mov	al,[ebx+SymbolTree_Leaf.flags]
	or	[ebx+SymbolTree_Leaf.flags],SYM_PREDICTED + SYM_PREDICTED_DEFINED
       ; clc
	retn
    symbol_defined:
	mov	al,[ebx+SymbolTree_Leaf.flags]
	clc
	retn
    symbol_predicted_undefined:
	mov	al,[ebx+SymbolTree_Leaf.flags]
	or	al,SYM_PREDICTED
	and	al,not SYM_PREDICTED_DEFINED
	xchg	[ebx+SymbolTree_Leaf.flags],al
	stc
	retn
    get_variable_value:
	test	edx,edx
	jz	symbol_undefined
    inspect_definition:
	test	[edx+ValueDefinition.flags],VAL_INTERNAL
	jnz	symbol_defined
	test	[edx+ValueDefinition.flags],VAL_IN_USE
	jnz	try_previous_definition
	mov	eax,[current_pass]
	cmp	eax,[edx+ValueDefinition.pass]
	je	symbol_defined
	cmp	edx,[ebx+SymbolTree_Leaf.definition]
	je	retire_outdated_definition
      try_previous_definition:
	mov	edx,[edx+ValueDefinition.previous]
	test	edx,edx
	jnz	inspect_definition
    symbol_undefined:
	mov	al,[ebx+SymbolTree_Leaf.flags]
	stc
	retn
    retire_outdated_definition:
	mov	eax,edx
	dec	[edx+ValueDefinition.reference_count]
	xchg	edx,[ebx+SymbolTree_Leaf.retired_definition]
	xchg	edx,[eax+ValueDefinition.previous]
	mov	[ebx+SymbolTree_Leaf.definition],edx
	jmp	get_variable_value

mark_symbol_as_used:
; in:
;  ebx - SymbolTree_Leaf
;  edx - ValueDefinition, null when value is unavailable but used nonetheless
; preserves: ebx, ecx, edx, esi, edi
; note:
;  normally setting SymbolTree_Leaf.last_use_pass is enough, but this function
;  improves prediction quality of USED operator;
	mov	eax,[current_pass]
	mov	[ebx+SymbolTree_Leaf.last_use_pass],eax
	test	edx,edx
	jz	mark_used_fallbacks
	retn
    mark_used_fallbacks:
	push	ebx edx
      mark_fallback_neighbour:
	mov	edx,[ebx+SymbolTree_Leaf.fallback_neighbour]
	test	edx,edx
	jz	mark_fallback_parent
	mov	[edx+SymbolTree_Leaf.last_use_pass],eax
      mark_fallback_parent:
	mov	ebx,[ebx+SymbolTree_Leaf.fallback_parent]
	test	ebx,ebx
	jz	used_fallbacks_marked
	mov	[ebx+SymbolTree_Leaf.last_use_pass],eax
	jmp	mark_fallback_neighbour
      used_fallbacks_marked:
	pop	edx ebx
	retn

use_available_value:
; same as get_available_value, but also includes operation of mark_symbol_as_used
	mov	eax,[current_pass]
	mov	[ebx+SymbolTree_Leaf.last_use_pass],eax
	call	get_available_value
	jc	use_unavailable_value
	retn
    use_unavailable_value:
	call	mark_used_fallbacks
	stc
	retn

create_constant_value_definition:
; in:
;  ebx - SymbolTree_Leaf
; out:
;  edx - ValueDefinition possibly containing value to update, null if definition forbidden
; preserves: ebx, esi, edi
	test	[ebx+SymbolTree_Leaf.flags],SYM_VARIABLE
	jnz	symbol_already_defined
	or	[ebx+SymbolTree_Leaf.flags],SYM_CONSTANT
	mov	eax,[ebx+SymbolTree_Leaf.definition]
	test	eax,eax
	jz	add_value_definition
	test	[eax+ValueDefinition.flags],VAL_INTERNAL
	jnz	symbol_already_defined
	mov	ecx,[eax+ValueDefinition.pass]
	cmp	ecx,[current_pass]
	jne	reset_value
    symbol_already_defined:
	mov	edx,_conflicting_definition
	call	register_error
	xor	edx,edx
	retn
update_value_definition:
; in:
;  ebx - SymbolTree_Leaf
; out:
;  edx - ValueDefinition possibly containing value to update, null if definition forbidden
; preserves: ebx, esi, edi
	mov	eax,[ebx+SymbolTree_Leaf.definition]
	test	eax,eax
	jz	add_value_definition
	test	[eax+ValueDefinition.flags],VAL_INTERNAL
	jnz	value_redefinition
	mov	ecx,[eax+ValueDefinition.pass]
	cmp	ecx,[current_pass]
	jne	reset_value
	test	[ebx+SymbolTree_Leaf.flags],SYM_CONSTANT or SYM_LINK
	jnz	constant_redefined
	mov	edx,eax
	or	[ebx+SymbolTree_Leaf.flags],SYM_VARIABLE
	test	[ebx+SymbolTree_Leaf.flags],SYM_PREDICTED
	jz	dynamic_value_definition_ready
	and	[ebx+SymbolTree_Leaf.flags],not SYM_PREDICTED
	test	[ebx+SymbolTree_Leaf.flags],SYM_PREDICTED_DEFINED
	jz	dynamic_value_definition_ready
	or	[next_pass_needed],1
    dynamic_value_definition_ready:
	cmp	[edx+ValueDefinition.reference_count],1
	ja	dynamic_value_in_use
	retn
    dynamic_value_in_use:
	mov	eax,[edx+ValueDefinition.previous]
	mov	[ebx+SymbolTree_Leaf.definition],eax
	dec	[edx+ValueDefinition.reference_count]
	mov	eax,edx
	xchg	eax,[ebx+SymbolTree_Leaf.retired_definition]
	mov	[edx+ValueDefinition.previous],eax
create_value_definition:
; in:
;  ebx - SymbolTree_Leaf
; out:
;  edx - ValueDefinition possibly containing value to update, null if definition forbidden
; preserves: ebx, esi, edi
	mov	eax,[ebx+SymbolTree_Leaf.definition]
	test	eax,eax
	jz	add_value_definition
	test	[eax+ValueDefinition.flags],VAL_INTERNAL
	jnz	value_redefinition
	mov	ecx,[eax+ValueDefinition.pass]
	cmp	ecx,[current_pass]
	je	value_redefinition
    reset_value:
	test	[ebx+SymbolTree_Leaf.flags],SYM_LINK
	jnz	reset_link_value
	mov	edx,[ebx+SymbolTree_Leaf.retired_definition]
      retire_previous_values:
	dec	[eax+ValueDefinition.reference_count]
	xchg	edx,[eax+ValueDefinition.previous]
	xchg	eax,edx
	test	eax,eax
	jz	previous_values_retired
	test	[eax+ValueDefinition.flags],VAL_INTERNAL
	jz	retire_previous_values
    previous_values_retired:
	mov	[ebx+SymbolTree_Leaf.definition],edx
	xchg	eax,[edx+ValueDefinition.previous]
	mov	[ebx+SymbolTree_Leaf.retired_definition],eax
	cmp	[edx+ValueDefinition.reference_count],0
	jne	add_value_definition
	inc	[edx+ValueDefinition.reference_count]
	test	[ebx+SymbolTree_Leaf.flags],SYM_VARIABLE
	jnz	reset_value_type
	retn
    reset_link_value:
	and	[ebx+SymbolTree_Leaf.flags],not (SYM_LINK or SYM_LINK_PREDICTED)
	mov	eax,[ebx+SymbolTree_Leaf.last_use_pass]
	cmp	eax,[current_pass]
	jne	reset_current_link_value
	or	[next_pass_needed],1
      reset_current_link_value:
	xor	eax,eax
	xchg	eax,[ebx+SymbolTree_Leaf.definition]
	dec	[eax+ValueDefinition.reference_count]
	jnz	reset_previous_link_value
	test	[eax+ValueDefinition.flags],VAL_DETACHED
	jz	reset_previous_link_value
	mov	ecx,eax
	xchg	eax,[retired_definition]
	mov	[ecx+ValueDefinition.previous],eax
      reset_previous_link_value:
	xor	eax,eax
	xchg	eax,[ebx+SymbolTree_Leaf.retired_definition]
	test	eax,eax
	jz	add_value_definition
	dec	[eax+ValueDefinition.reference_count]
	jnz	add_value_definition
	test	[eax+ValueDefinition.flags],VAL_DETACHED
	jz	add_value_definition
	mov	ecx,eax
	xchg	eax,[retired_definition]
	mov	[ecx+ValueDefinition.previous],eax
	jmp	add_value_definition
    constant_redefined:
	mov	edx,_conflicting_definition
	call	register_error
	xor	edx,edx
	retn
    value_redefinition:
	cmp	[eax+ValueDefinition.type],VALTYPE_ELEMENT
	je	constant_redefined
	test	[ebx+SymbolTree_Leaf.flags],SYM_CONSTANT or SYM_LINK
	jnz	constant_redefined
	test	[ebx+SymbolTree_Leaf.flags],SYM_VARIABLE
	jnz	add_value_definition
	or	[ebx+SymbolTree_Leaf.flags],SYM_VARIABLE
	test	[ebx+SymbolTree_Leaf.flags],SYM_PREDICTED
	jz	add_value_definition
	and	[ebx+SymbolTree_Leaf.flags],not SYM_PREDICTED
	test	[ebx+SymbolTree_Leaf.flags],SYM_PREDICTED_DEFINED
	jz	add_value_definition
	or	[next_pass_needed],1
    add_value_definition:
	lea	ecx,[ebx+SymbolTree_Leaf.retired_definition]
    retrieve_retired_value:
	mov	eax,[ecx]
	test	eax,eax
	jz	new_value_definition
	cmp	[eax+ValueDefinition.reference_count],0
	jne	retired_value_immutable
	inc	[eax+ValueDefinition.reference_count]
	mov	edx,[eax+ValueDefinition.previous]
	mov	[ecx],edx
	mov	[eax+ValueDefinition.type],VALTYPE_RESERVED
	jmp	adopt_value_definition
    retired_value_immutable:
	lea	ecx,[eax+ValueDefinition.previous]
	jmp	retrieve_retired_value
    new_value_definition:
	mov	ecx,sizeof.ValueDefinition
	call	create_tree_element
	mov	ecx,eax
	xchg	ecx,[value_definition_chain]
	mov	[eax+ValueDefinition.interlink],ecx
	assert	VALTYPE_RESERVED = 0
	inc	[eax+ValueDefinition.reference_count]
    adopt_value_definition:
	mov	edx,eax
	xchg	eax,[ebx+SymbolTree_Leaf.definition]
	mov	[edx+ValueDefinition.previous],eax
	test	[ebx+SymbolTree_Leaf.flags],SYM_VARIABLE
	jnz	reset_value_type
	test	eax,eax
	jz	value_definition_ready
	mov	ecx,eax
	xchg	ecx,[ebx+SymbolTree_Leaf.retired_definition]
	xchg	ecx,[eax+ValueDefinition.previous]
	mov	[edx+ValueDefinition.previous],ecx
       ; test	 ecx,ecx
       ; jnz	 internal_error
	mov	ecx,[eax+ValueDefinition.pass]
	mov	[edx+ValueDefinition.pass],ecx
	mov	cl,[eax+ValueDefinition.type]
	mov	[edx+ValueDefinition.type],cl
	mov	ecx,[eax+ValueDefinition.value_length]
	mov	[edx+ValueDefinition.value_length],ecx
	jecxz	value_definition_ready
	push	esi edi
	mov	esi,[eax+ValueDefinition.value]
	mov	edi,[edx+ValueDefinition.value]
	cmp	ecx,[edx+ValueDefinition.block_length]
	jbe	duplicate_value
	push	edx
	cmp	[edx+ValueDefinition.block_length],0
	je	reallocate_for_duplicate
	push	ecx
	xor	eax,eax
	xchg	eax,[edx+ValueDefinition.value]
	call	mfree
	pop	ecx
    reallocate_for_duplicate:
	mov	edi,ecx
	call	malloc
	pop	edx
	mov	[edx+ValueDefinition.value],eax
	mov	[edx+ValueDefinition.block_length],ecx
	mov	ecx,edi
	mov	edi,eax
    duplicate_value:
	mov	eax,ecx
	shr	ecx,2
	rep	movsd
	mov	cl,al
	and	cl,11b
	rep	movsb
	pop	edi esi
    value_definition_ready:
	retn
    reset_value_type:
	mov	[edx+ValueDefinition.type],VALTYPE_RESERVED
	retn

remove_value_definition:
; in:
;  ebx - SymbolTree_Leaf
;  edx - ValueDefinition, null to remove the present value
;  edi - SymbolTree_Leaf where to move the value, null for plain remove
; out:
;  cf set if there was no value to remove
;  when cf = 0:
;   edx - ValueDefinition that got removed
; preserves: ebx, esi, edi
	test	[ebx+SymbolTree_Leaf.flags],SYM_VARIABLE
	jnz	variable_ready
	test	[ebx+SymbolTree_Leaf.flags],SYM_CONSTANT or SYM_LINK
	jnz	cannot_apply_to_constant
	or	[ebx+SymbolTree_Leaf.flags],SYM_VARIABLE
	test	[ebx+SymbolTree_Leaf.flags],SYM_PREDICTED
	jz	variable_ready
	and	[ebx+SymbolTree_Leaf.flags],not SYM_PREDICTED
	test	[ebx+SymbolTree_Leaf.flags],SYM_PREDICTED_DEFINED
	jz	variable_ready
	or	[next_pass_needed],1
    variable_ready:
	lea	ecx,[ebx+SymbolTree_Leaf.definition]
	test	edx,edx
	jnz	find_definition_to_remove
	mov	edx,[ecx]
	test	edx,edx
	jz	no_definition_to_remove
	test	[edx+ValueDefinition.flags],VAL_INTERNAL
	jnz	no_definition_to_remove
	cmp	[edx+ValueDefinition.type],VALTYPE_RESERVED
	je	no_definition_to_remove
	mov	eax,[edx+ValueDefinition.pass]
	cmp	eax,[current_pass]
	jne	no_definition_to_remove
    remove_definition:
	mov	eax,[edx+ValueDefinition.previous]
	test	edi,edi
	jnz	move_definition
	mov	[ecx],eax
	mov	eax,[ebx+SymbolTree_Leaf.retired_definition]
	mov	[edx+ValueDefinition.previous],eax
	dec	[edx+ValueDefinition.reference_count]
	mov	[ebx+SymbolTree_Leaf.retired_definition],edx
	clc
	retn
    move_definition:
	cmp	edi,edx
	je	definition_moved
	test	[edi+SymbolTree_Leaf.flags],SYM_VARIABLE
	jnz	append_moved_definition
	test	[edi+SymbolTree_Leaf.flags],SYM_CONSTANT or SYM_LINK
	jnz	cannot_apply_to_constant
	or	[edi+SymbolTree_Leaf.flags],SYM_VARIABLE
	test	[edi+SymbolTree_Leaf.flags],SYM_PREDICTED
	jz	append_moved_definition
	and	[ebx+SymbolTree_Leaf.flags],not SYM_PREDICTED
	test	[ebx+SymbolTree_Leaf.flags],SYM_PREDICTED_DEFINED
	jz	append_moved_definition
	or	[next_pass_needed],1
    append_moved_definition:
	mov	[ecx],eax
	mov	eax,[edi+SymbolTree_Leaf.definition]
	mov	[edx+ValueDefinition.previous],eax
	mov	[edi+SymbolTree_Leaf.definition],edx
    definition_moved:
	clc
	retn
    find_definition_to_remove:
	mov	eax,[ecx]
	cmp	eax,edx
	je	remove_definition
	test	eax,eax
	jz	no_definition_to_remove
	lea	ecx,[eax+ValueDefinition.previous]
	jmp	find_definition_to_remove
    cannot_apply_to_constant:
	mov	edx,_cannot_apply_to_constant
	call	register_error
    no_definition_to_remove:
	stc
	retn

assign_value:
; in:
;  ebx - SymbolTree_Leaf
;  edx - ValueDefinition prepared to receive the new value
;  esi - value
;  ecx = length of value
;  [value_type] = VALTYPE_#
; preserves: ebx, edx
	and	[edx+ValueDefinition.flags],0
	mov	eax,[current_pass]
	sub	eax,[edx+ValueDefinition.pass]
	add	[edx+ValueDefinition.pass],eax
	test	[ebx+SymbolTree_Leaf.flags],SYM_VARIABLE
	jnz	reuse_value_block
	cmp	eax,[current_pass]
	je	reuse_value_block
	mov	edi,[ebx+SymbolTree_Leaf.last_use_pass]
	cmp	edi,[current_pass]
	jne	reuse_value_block
	cmp	ecx,[edx+ValueDefinition.value_length]
	je	update_value
	or	[next_pass_needed],1
    reuse_value_block:
	mov	[edx+ValueDefinition.value_length],ecx
	mov	edi,[edx+ValueDefinition.value]
	cmp	ecx,[edx+ValueDefinition.block_length]
	jbe	write_value
	push	edx
	push	ecx
	cmp	[edx+ValueDefinition.block_length],0
	je	new_value
	xor	eax,eax
	xchg	eax,[edx+ValueDefinition.value]
	call	mfree
    new_value:
	pop	ecx
	call	malloc
	pop	edx
	mov	[edx+ValueDefinition.value],eax
	mov	[edx+ValueDefinition.block_length],ecx
	mov	edi,eax
    write_value:
	mov	al,[value_type]
	mov	[edx+ValueDefinition.type],al
	mov	eax,[edx+ValueDefinition.value_length]
	mov	ecx,eax
	shr	ecx,2
	rep	movsd
	mov	cl,al
	and	cl,11b
	rep	movsb
    value_written:
	retn
    update_value:
	mov	edi,[edx+ValueDefinition.value]
	cmp	eax,1
	ja	rewrite_value
	mov	al,[value_type]
	cmp	[edx+ValueDefinition.type],al
	jne	rewrite_value
	jecxz	value_written
	cmp	al,VALTYPE_SYMBOLIC
	je	update_symbolic_value
	mov	eax,[edx+ValueDefinition.value_length]
	mov	ecx,eax
	shr	ecx,2
	repe	cmpsd
	jne	value_dword_changed
	mov	cl,al
	and	cl,11b
	repe	cmpsb
	je	value_updated
	inc	ecx
	dec	esi
	dec	edi
	or	[next_pass_needed],1
	rep	movsb
    value_updated:
	retn
    value_dword_changed:
	or	[next_pass_needed],1
	inc	ecx
	sub	esi,4
	sub	edi,4
	rep	movsd
	mov	cl,al
	and	cl,11b
	rep	movsb
	retn
    update_symbolic_value:
	mov	ecx,[edx+ValueDefinition.value_length]
	push	edx
	call	compare_symbolic_values
	pop	edx
	jecxz	value_updated
    value_changed:
	or	[next_pass_needed],1
	mov	eax,ecx
	shr	ecx,2
	rep	movsd
	mov	cl,al
	and	cl,11b
	rep	movsb
	retn
    rewrite_value:
	or	[next_pass_needed],1
	jmp	write_value

expand_value:
; in:
;  edx - ValueDefinition
;  ebx = number of additional zero bytes to append to the value
; preserves: ebx, edx, esi
	mov	eax,[edx+ValueDefinition.value_length]
	mov	edi,[edx+ValueDefinition.value]
	add	edi,eax
	add	eax,ebx
	jc	out_of_memory
	cmp	eax,[edx+ValueDefinition.block_length]
	jbe	append_zero_bytes
	push	eax
	bsr	ecx,eax
	dec	cl
	shr	eax,cl
	inc	eax
	shl	eax,cl
	mov	ecx,eax
	pop	eax
	cmp	ecx,eax
	jbe	out_of_memory
	mov	eax,[edx+ValueDefinition.value]
	push	edx
	call	realloc
	pop	edx
	mov	[edx+ValueDefinition.value],eax
	mov	[edx+ValueDefinition.block_length],ecx
	mov	edi,eax
	add	edi,[edx+ValueDefinition.value_length]
    append_zero_bytes:
	add	[edx+ValueDefinition.value_length],ebx
	xor	eax,eax
	mov	ecx,ebx
	shr	ecx,2
	rep	stosd
	mov	cl,bl
	and	cl,11b
	rep	stosb
	retn

update_value_link:
; in:
;  ebx - SymbolTree_Leaf
;  edx - ValueDefinition to link
; preserves: ebx, edx
; note: value must be from the current pass
	mov	eax,[ebx+SymbolTree_Leaf.definition]
	test	eax,eax
	jz	values_detached
	mov	ecx,[eax+ValueDefinition.pass]
	cmp	ecx,[current_pass]
	je	symbol_already_defined
	test	[ebx+SymbolTree_Leaf.flags],SYM_LINK
	jnz	update_established_link
    detach_and_retire_values:
	test	eax,eax
	jz	values_detached
	dec	[eax+ValueDefinition.reference_count]
	or	[eax+ValueDefinition.flags],VAL_DETACHED
	mov	ecx,eax
	xchg	eax,[retired_definition]
	xchg	eax,[ecx+ValueDefinition.previous]
	jmp	detach_and_retire_values
      values_detached:
	mov	[ebx+SymbolTree_Leaf.definition],eax
	mov	eax,[ebx+SymbolTree_Leaf.retired_definition]
    detach_retired_values:
	test	eax,eax
	jz	retired_values_detached
	or	[eax+ValueDefinition.flags],VAL_DETACHED
	mov	ecx,eax
	xchg	eax,[retired_definition]
	xchg	eax,[ecx+ValueDefinition.previous]
	jmp	detach_retired_values
      retired_values_detached:
	mov	[ebx+SymbolTree_Leaf.retired_definition],eax
	or	[ebx+SymbolTree_Leaf.flags],SYM_LINK
	jmp	link_new_value
    update_established_link:
	mov	ecx,[current_pass]
	cmp	ecx,[ebx+SymbolTree_Leaf.last_use_pass]
	jne	link_new_value
	or	[ebx+SymbolTree_Leaf.flags],SYM_LINK_PREDICTED
    link_new_value:
       ; cmp	 ecx,[edx+ValueDefinition.pass]
       ; jne	 internal_error
	mov	eax,edx
	inc	[eax+ValueDefinition.reference_count]
	xchg	eax,[ebx+SymbolTree_Leaf.definition]
	xchg	eax,[ebx+SymbolTree_Leaf.retired_definition]
	test	eax,eax
	jz	link_updated
	dec	[eax+ValueDefinition.reference_count]
	jnz	link_updated
	test	[eax+ValueDefinition.flags],VAL_DETACHED
	jz	link_updated
	mov	ecx,eax
	xchg	eax,[retired_definition]
	mov	[ecx+ValueDefinition.previous],eax
      link_updated:
	retn

detect_mispredictions:
; in: ebx - SymbolTree_Root
; note:
;  while it is looking for mispredicted definitions, it also prepares the tree for the next pass
	mov	edx,[tree_stack_base]
    browse_from_root:
	xor	eax,eax
	mov	[ebx+SymbolTree_Root.current_label],eax
	and	[ebx+SymbolTree_Root.flags],not NAMESPACE_CURRENT_LABEL_TO_CONFIRM
	test	[ebx+SymbolTree_Root.attributes],SYMTREE_LOCAL
	jnz	browse_local_tree
	add	ebx,sizeof.SymbolTree_Root
	mov	eax,[tree_stack_end]
	sub	eax,TREE_HEIGHT*8+16
	cmp	edx,eax
	jbe	tree_stack_prepared
	mov	eax,[tree_stack_base]
	sub	edx,eax
	mov	ecx,[tree_stack_end]
	sub	ecx,eax
	add	ecx,TREE_HEIGHT*8+16
	push	edx
	call	grow_stack
	pop	edx
	add	edx,eax
	mov	[tree_stack_base],eax
	add	eax,ecx
	mov	[tree_stack_end],eax
    tree_stack_prepared:
	mov	ecx,TREE_HEIGHT
    browse_node:
	mov	edi,ebx
	dec	cl
    browse_branch:
	cmp	dword [edi],0
	je	branch_browsed
	test	cl,cl
	jnz	deeper_node
	mov	esi,[edi]
    browse_foliage:
	mov	eax,[esi+SymbolTree_Foliage.child_namespace]
	test	eax,eax
	jz	subtree_browsed
	mov	[edx],ebx
	mov	[edx+4],ecx
	mov	[edx+8],esi
	mov	[edx+12],edi
	add	edx,16
	mov	ebx,eax
	jmp	browse_from_root
    browse_local_tree:
	add	ebx,sizeof.SymbolTree_Root
	mov	eax,[tree_stack_end]
	sub	eax,LOCAL_TREE_HEIGHT*8+16
	cmp	edx,eax
	jbe	local_tree_stack_prepared
	mov	eax,[tree_stack_base]
	sub	edx,eax
	mov	ecx,[tree_stack_end]
	sub	ecx,eax
	add	ecx,LOCAL_TREE_HEIGHT*8+16
	push	edx
	call	grow_stack
	pop	edx
	add	edx,eax
	mov	[tree_stack_base],eax
	add	eax,ecx
	mov	[tree_stack_end],eax
    local_tree_stack_prepared:
	mov	ecx,LOCAL_TREE_HEIGHT + 1 shl 8
	jmp	browse_node
    subtree_browsed:
	mov	eax,esi
	add	eax,sizeof.SymbolTree_Foliage
    inspect_leaf:
	test	[eax+SymbolTree_Leaf.flags],SYM_LINK_PREDICTED
	jz	link_prediction_ok
	push	eax ecx edx esi edi
	mov	edx,[eax+SymbolTree_Leaf.definition]
	mov	ecx,[eax+SymbolTree_Leaf.retired_definition]
	mov	al,[edx+ValueDefinition.type]
	cmp	al,[ecx+ValueDefinition.type]
	jne	link_value_mispredicted
	mov	esi,[ecx+ValueDefinition.value]
	mov	ecx,[ecx+ValueDefinition.value_length]
	mov	edi,[edx+ValueDefinition.value]
	mov	edx,[edx+ValueDefinition.value_length]
	cmp	ecx,edx
	jne	link_value_mispredicted
	jecxz	reset_link_prediction
	shr	ecx,2
	repe	cmpsd
	jne	link_value_mispredicted
	mov	cl,dl
	and	cl,11b
	repe	cmpsb
	je	reset_link_prediction
    link_value_mispredicted:
	or	[next_pass_needed],1
    reset_link_prediction:
	and	[eax+SymbolTree_Leaf.flags],not SYM_LINK_PREDICTED
	pop	edi esi edx ecx eax
    link_prediction_ok:
	test	[eax+SymbolTree_Leaf.flags],SYM_VARIABLE
	jnz	inspect_next_leaf
	and	[eax+SymbolTree_Leaf.flags],not SYM_CONSTANT
	test	[eax+SymbolTree_Leaf.flags],SYM_PREDICTED or SYM_USAGE_PREDICTED
	jz	inspect_next_leaf
	push	edx
	test	[eax+SymbolTree_Leaf.flags],SYM_PREDICTED
	jz	definition_prediction_ok
	mov	edx,[eax+SymbolTree_Leaf.definition]
	test	edx,edx
	jz	detect_misprediction_of_undefined
	test	[edx+ValueDefinition.flags],VAL_INTERNAL
	jnz	detect_misprediction_of_defined
	mov	edx,[edx+ValueDefinition.pass]
	cmp	edx,[current_pass]
	je	detect_misprediction_of_defined
    detect_misprediction_of_undefined:
	test	[eax+SymbolTree_Leaf.flags],SYM_PREDICTED_DEFINED
	jz	reset_prediction
	or	[next_pass_needed],1
	jmp	reset_prediction
    detect_misprediction_of_defined:
	test	[eax+SymbolTree_Leaf.flags],SYM_PREDICTED_DEFINED
	jnz	reset_prediction
	or	[next_pass_needed],1
    reset_prediction:
	and	[eax+SymbolTree_Leaf.flags],not SYM_PREDICTED
    definition_prediction_ok:
	test	[eax+SymbolTree_Leaf.flags],SYM_USAGE_PREDICTED
	jz	usage_prediction_ok
	mov	edx,[eax+SymbolTree_Leaf.last_use_pass]
	cmp	edx,[current_pass]
	je	detect_misprediction_of_used
    detect_misprediction_of_unused:
	test	[eax+SymbolTree_Leaf.flags],SYM_PREDICTED_USED
	jz	reset_usage_prediction
	or	[next_pass_needed],1
	jmp	reset_usage_prediction
    detect_misprediction_of_used:
	test	[eax+SymbolTree_Leaf.flags],SYM_PREDICTED_USED
	jnz	reset_usage_prediction
	or	[next_pass_needed],1
    reset_usage_prediction:
	and	[eax+SymbolTree_Leaf.flags],not SYM_USAGE_PREDICTED
    usage_prediction_ok:
	pop	edx
	jmp	inspect_next_leaf
    inspect_next_leaf:
	mov	eax,[eax+SymbolTree_Leaf.next]
	test	eax,eax
	jnz	inspect_leaf
	mov	esi,[esi+SymbolTree_Foliage.next]
	test	esi,esi
	jnz	browse_foliage
    branch_browsed:
	test	ch,ch
	jnz	local_branch_browsed
	add	edi,4
	lea	eax,[ebx+sizeof.SymbolTree_Node]
	cmp	edi,eax
	jne	browse_branch
	inc	cl
	cmp	cl,TREE_HEIGHT
	je	tree_browsed
	sub	edx,8
	mov	ebx,[edx]
	mov	edi,[edx+4]
	jmp	branch_browsed
    local_branch_browsed:
	add	edi,4
	lea	eax,[ebx+sizeof.SymbolTree_LocalNode]
	cmp	edi,eax
	jne	browse_branch
	inc	cl
	cmp	cl,LOCAL_TREE_HEIGHT
	je	tree_browsed
	sub	edx,8
	mov	ebx,[edx]
	mov	edi,[edx+4]
	jmp	local_branch_browsed
    deeper_node:
	mov	[edx],ebx
	mov	[edx+4],edi
	add	edx,8
	mov	ebx,[edi]
	jmp	browse_node
    tree_browsed:
	cmp	edx,[tree_stack_base]
	je	complete_tree_browsed
	sub	edx,16
	mov	ebx,[edx]
	mov	ecx,[edx+4]
	mov	esi,[edx+8]
	mov	edi,[edx+12]
	jmp	subtree_browsed
    complete_tree_browsed:
	retn
