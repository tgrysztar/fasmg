set include=..\..\..\include
fasmg hello_x32.asm -iInclude('format/format.inc')
fasmg obj1.asm -iInclude('format/format.inc')
fasmg obj2.asm -iInclude('format/format.inc')
