
include 'x86-2.inc'

use i386, 32

{rmdst} xor eax,ebx	; 31 D8
{rmsrc} xor eax,ebx	; 33 C3

{imm8}	add ecx,1	; 83 C1 01
{imm32} add ecx,1	; 81 C1 01 00 00 00

use AMD64, 64

{imm32} mov rax,1000	; 48 C7 C0 E8 03 00 00
{imm64} mov rax,1000	; 48 B8 E8 03 00 00 00 00 00 00
