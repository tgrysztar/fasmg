
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

	mov	al,10001b		; begin PIC 1 initialization
	out	20h,al
	mov	al,10001b		; begin PIC 2 initialization
	out	0A0h,al
	mov	al,80h			; IRQ 0-7: interrupts 80h-87h
	out	21h,al
	mov	al,88h			; IRQ 8-15: interrupts 88h-8Fh
	out	0A1h,al
	mov	al,100b 		; slave connected to IRQ2
	out	21h,al
	mov	al,2
	out	0A1h,al
	mov	al,1			; Intel environment, manual EOI
	out	21h,al
	out	0A1h,al
	in	al,21h
	mov	al,11111100b		; enable only clock and keyboard IRQ
	out	21h,al
	in	al,0A1h
	mov	al,11111111b
	out	0A1h,al

	xor	edi,edi 		; create IDT (at linear address 0)
	mov	ecx,21
  make_exception_gates: 		; make gates for exception handlers
	mov	esi,exception_gate
	movsq
	movsq
	loop	make_exception_gates
	mov	ecx,256-21
  make_interrupt_gates: 		; make gates for the other interrupts
	mov	esi,interrupt_gate
	movsq
	movsq
	loop	make_interrupt_gates

	mov	word [80h*16],clock	; set IRQ 0 handler
	mov	word [81h*16],keyboard	; set IRQ 1 handler

	lidt	[IDTR]			; load IDT register

	sti				; now we may enable the interrupts

  main_loop:

	mov	rax,'L O N G '
	mov	[0B8000h],rax

	jmp	main_loop


IDTR:					; Interrupt Descriptor Table Register
  dw 256*16-1				; limit of IDT (size minus one)
  dq 0					; linear address of IDT

exception_gate:
  dw exception and 0FFFFh,LONG_SELECTOR
  dw 8E00h,exception shr 16
  dd 0,0

interrupt_gate:
  dw interrupt and 0FFFFh,LONG_SELECTOR
  dw 8F00h,interrupt shr 16
  dd 0,0

exception:				; exception handler
	in	al,61h			; turn on the speaker
	or	al,3
	out	61h,al
	jmp	exception		; repeat it until reboot

interrupt:				; handler for all other interrupts
	iretq

clock:
	inc	byte [0B8000h+2*80]	; make the ticks appear
	push	rax
	mov	al,20h
	out	20h,al
	pop	rax
	iretq

keyboard:
	push	rax
	in	al,60h
	cmp	al,1			; check for Esc key
	je	reboot
	mov	[0B8000h+2*(80+1)],al	; show the scan key
	in	al,61h			; give finishing information
	out	61h,al			; to keyboard...
	mov	al,20h
	out	20h,al			; ...and interrupt controller
	pop	rax
	iretq

reboot:
	mov	al,0FEh
	out	64h,al			; reboot computer
	jmp	reboot
