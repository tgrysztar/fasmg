
	format binary as 'bin'
	include 'cpu/x64.inc'

	include 'listing.inc'


	ORG	1600h

	USE16

	cli				; disable the interrupts, just in
					; case they are not disabled yet

	lgdt	[cs:GDTR]		; load GDT register

	mov	eax,cr0 		; switch to protected mode
	or	al,1
	mov	cr0,eax

	jmp	CODE_SELECTOR:pm_start


NULL_SELECTOR = 0
DATA_SELECTOR = 1 shl 3 		; flat data selector (ring 0)
CODE_SELECTOR = 2 shl 3 		; 32-bit code selector (ring 0)
LONG_SELECTOR = 3 shl 3 		; 64-bit code selector (ring 0)

GDTR:					; Global Descriptors Table Register
  dw 4*8-1				; limit of GDT (size minus one)
  dq GDT				; linear address of GDT

GDT rw 4				; null desciptor
    dw 0FFFFh,0,9200h,08Fh		; flat data desciptor
    dw 0FFFFh,0,9A00h,0CFh		; 32-bit code desciptor
    dw 0FFFFh,0,9A00h,0AFh		; 64-bit code desciptor

	USE32

pm_start:

	mov	eax,DATA_SELECTOR	; load 4 GB data descriptor
	mov	ds,ax			; to all data segment registers
	mov	es,ax
	mov	fs,ax
	mov	gs,ax
	mov	ss,ax

	mov	eax,cr4
	or	eax,1 shl 5
	mov	cr4,eax 		; enable physical-address extensions

	mov	edi,70000h
	mov	ecx,4000h shr 2
	xor	eax,eax
	rep	stosd			; clear the page tables

	mov	dword [70000h],71000h + 111b ; first PDP table
	mov	dword [71000h],72000h + 111b ; first page directory
	mov	dword [72000h],73000h + 111b ; first page table

	mov	edi,73000h		; address of first page table
	mov	eax,0 + 111b
	mov	ecx,256 		; number of pages to map (1 MB)
  make_page_entries:
	stosd
	add	edi,4
	add	eax,1000h
	loop	make_page_entries

	mov	eax,70000h
	mov	cr3,eax 		; load page-map level-4 base

	mov	ecx,0C0000080h		; EFER MSR
	rdmsr
	or	eax,1 shl 8		; enable long mode
	wrmsr

	mov	eax,cr0
	or	eax,1 shl 31
	mov	cr0,eax 		; enable paging

	jmp	LONG_SELECTOR:long_start

	USE64

long_start:

	mov	rax,'L O N G '
	mov	[0B8000h],rax

	jmp	long_start