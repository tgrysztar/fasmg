
The name of flat assembler g (abbreviated to fasmg) is intentionally stylized
with lowercase letters. This is a nod to the history of its precedessor.

The "source" directory contains the complete source code which
can be assembled with either fasm or fasmg except for MacOS version,
which can only be assembled with fasmg.

The executable file for Windows is "fasmg.exe", while "fasmg" and "fasmg.x64"
are for Linux in 32-bit and 64-bit format respectively. The files for MacOS
are at "source/macos/fasmg" and "source/macos/x64/fasmg".

The "source/libc/fasmg.asm" may be used to assemble fasmg as an ELF object
that can then be linked to a 32-bit C library with a third-party linker to
produce an executable native to a particular operating system. A similar
object file in Mach-O format can be assembled from "source/macos/fasmg.o.asm".

When the source code is assembled with fasmg, it depends on the files from
"examples/x86/include" directory that implement the instruction sets and
output formats compatible with flat assembler 1.