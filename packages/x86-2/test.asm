include 'x86-2.inc'
include 'iev.alm'

    {i286}	loadall 	; 0F 05
    {i386}	loadall 	; 0F 07

use i386, bits32

    {rmdst}	xor eax,ebx	; 31 D8
    {rmsrc}	xor eax,ebx	; 33 C3

    {imm8}	add ecx,1	; 83 C1 01
    {imm32}	add ecx,1	; 81 C1 01 00 00 00

use AMD64, bits64

    {imm32}	mov rax,1000	; 48 C7 C0 E8 03 00 00
    {imm64}	mov rax,1000	; 48 B8 E8 03 00 00 00 00 00 00

    {AVX}	vaddpd ymm1,ymm2,[rbx+80h]	; C5 ED 58 8B 80 00 00 00
    {AVX512}	vaddpd ymm1,ymm2,[rbx+80h]	; 62 F1 ED 28 58 4B 04
