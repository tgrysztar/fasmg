
T_BOOLEAN = 4
T_CHAR	  = 5
T_FLOAT   = 6
T_DOUBLE  = 7
T_BYTE	  = 8
T_SHORT   = 9
T_INT	  = 10
T_LONG	  = 11

macro aaload
  db 0x32
end macro

macro aastore
  db 0x53
end macro

macro aconst_null
  db 0x01
end macro

macro aload index
  if index<0
    err "invalid index"
  else if index<=3
    db 0x2a+index
  else if index<100h
    db 0x19,index
  else
    db 0xc4,0x19,(index) shr 8,(index) and 0FFh
  end if
end macro

macro anewarray class
  db 0xbd,(class) shr 8,(class) and 0FFh
end macro

macro areturn
  db 0xb0
end macro

macro arraylength
  db 0xbe
end macro

macro astore index
  if index<0
    err "invalid index"
  else if index<=3
    db 0x4b+index
  else if index<100h
    db 0x3a,index
  else
    db 0xc4,0x3a,(index) shr 8,(index) and 0FFh
  end if
end macro

macro athrow
  db 0xbf
end macro

macro baload
  db 0x33
end macro

macro bastore
  db 0x54
end macro

macro bipush byte
  if byte>-1 & byte<=5
   db 0x03+byte
  else
   db 0x10,byte
  end if
end macro

macro caload
  db 0x34
end macro

macro castore
  db 0x55
end macro

macro checkcast class
  db 0xc0,(class) shr 8,(class) and 0FFh
end macro

macro d2f
  db 0x90
end macro

macro d2i
  db 0x8e
end macro

macro d2l
  db 0x8f
end macro

macro dadd
  db 0x63
end macro

macro daload
  db 0x31
end macro

macro dastore
  db 0x52
end macro

macro dcmpg
  db 0x98
end macro

macro dcmpl
  db 0x97
end macro

macro dconst_0
  db 0x0e
end macro

macro dconst_1
  db 0x0f
end macro

macro ddiv
  db 0x6f
end macro

macro dload index
  if index<0
    err "invalid index"
  else if index<=3
    db 0x26+index
  else if index<100h
    db 0x18,index
  else
    db 0xc4,0x18,(index) shr 8,(index) and 0FFh
  end if
end macro

macro dmul
  db 0x6b
end macro

macro dneg
  db 0x77
end macro

macro drem
  db 0x73
end macro

macro dreturn
  db 0xaf
end macro

macro dstore index
  if index<0
    err "invalid index"
  else if index<=3
    db 0x47+index
  else if index<100h
    db 0x39,index
  else
    db 0xc4,0x39,(index) shr 8,(index) and 0FFh
  end if
end macro

macro dsub
  db 0x67
end macro

macro dup
  db 0x59
end macro

macro dup_x1
  db 0x5a
end macro

macro dup_x2
  db 0x5b
end macro

macro dup2
  db 0x5c
end macro

macro dup2_x1
  db 0x5d
end macro

macro dup2_x2
  db 0x5e
end macro

macro f2d
  db 0x8d
end macro

macro f2i
  db 0x8b
end macro

macro f2l
  db 0x8c
end macro

macro fadd
  db 0x62
end macro

macro faload
  db 0x30
end macro

macro fastore
  db 0x51
end macro

macro fcmpg
  db 0x96
end macro

macro fcmpl
  db 0x95
end macro

macro fconst_0
  db 0x0b
end macro

macro fconst_1
  db 0x0c
end macro

macro fconst_2
  db 0x0d
end macro

macro fdiv
  db 0x6e
end macro

macro fload index
  if index<0
    err "invalid index"
  else if index<=3
    db 0x22+index
  else if index<100h
    db 0x17,index
  else
    db 0xc4,0x17,(index) shr 8,(index) and 0FFh
  end if
end macro

macro fmul
  db 0x6a
end macro

macro fneg
  db 0x76
end macro

macro frem
  db 0x72
end macro

macro freturn
  db 0xae
end macro

macro fstore index
  if index<0
    err "invalid index"
  else if index<=3
    db 0x43+index
  else if index<100h
    db 0x38,index
  else
    db 0xc4,0x38,(index) shr 8,(index) and 0FFh
  end if
end macro

macro fsub
  db 0x66
end macro

macro getfield index
  db 0xb4,(index) shr 8,(index) and 0FFh
end macro

macro getstatic index
  db 0xb2,(index) shr 8,(index) and 0FFh
end macro

macro goto branch
  offset = branch-$
  if offset>=-8000h & offset<8000h
    db 0xa7,offset shr 8,offset and 0FFh
  else
    db 0xc8,offset shr 24,(offset shr 16) and 0FFh,(offset shr 8) and 0FFh,offset and 0FFh
  end if
end macro

macro goto_w branch
  offset = branch-$
  db 0xc8,offset shr 24,(offset shr 16) and 0FFh,(offset shr 8) and 0FFh,offset and 0FFh
end macro

macro i2b
  db 0x91
end macro

macro i2c
  db 0x92
end macro

macro i2d
  db 0x87
end macro

macro i2f
  db 0x86
end macro

macro i2l
  db 0x85
end macro

macro i2s
  db 0x93
end macro

macro iadd
  db 0x60
end macro

macro iaload
  db 0x2e
end macro

macro iand
  db 0x7e
end macro

macro iastore
  db 0x4f
end macro

macro iconst_m1
  db 0x02
end macro

macro iconst_0
  db 0x03
end macro

macro iconst_1
  db 0x04
end macro

macro iconst_2
  db 0x05
end macro

macro iconst_3
  db 0x06
end macro

macro iconst_4
  db 0x07
end macro

macro iconst_5
  db 0x08
end macro

macro idiv
  db 0x6c
end macro

macro if_acmpeq branch
  offset = branch-$
  db 0xa5,offset shr 8,offset and 0FFh
end macro

macro if_acmpne branch
  offset = branch-$
  db 0xa6,offset shr 8,offset and 0FFh
end macro

macro if_icmpeq branch
  offset = branch-$
  db 0x9f,offset shr 8,offset and 0FFh
end macro

macro if_icmpne branch
  offset = branch-$
  db 0xa0,offset shr 8,offset and 0FFh
end macro

macro if_icmplt branch
  offset = branch-$
  db 0xa1,offset shr 8,offset and 0FFh
end macro

macro if_icmpge branch
  offset = branch-$
  db 0xa2,offset shr 8,offset and 0FFh
end macro

macro if_icmpgt branch
  offset = branch-$
  db 0xa3,offset shr 8,offset and 0FFh
end macro

macro if_icmple branch
  offset = branch-$
  db 0xa4,offset shr 8,offset and 0FFh
end macro

macro ifeq branch
  offset = branch-$
  db 0x99,offset shr 8,offset and 0FFh
end macro

macro ifne branch
  offset = branch-$
  db 0x9a,offset shr 8,offset and 0FFh
end macro

macro iflt branch
  offset = branch-$
  db 0x9b,offset shr 8,offset and 0FFh
end macro

macro ifge branch
  offset = branch-$
  db 0x9c,offset shr 8,offset and 0FFh
end macro

macro ifgt branch
  offset = branch-$
  db 0x9d,offset shr 8,offset and 0FFh
end macro

macro ifle branch
  offset = branch-$
  db 0x9e,offset shr 8,offset and 0FFh
end macro

macro ifnonnull branch
  offset = branch-$
  db 0xc7,offset shr 8,offset and 0FFh
end macro

macro ifnull branch
  offset = branch-$
  db 0xc6,offset shr 8,offset and 0FFh
end macro

macro iinc index,const
  if index<0
    err "invalid index"
  else if index<100h & const<80h & const>=-80h
    db 0x84,index,const
  else
    db 0xc4,0x84,(index) shr 8,(index) and 0FFh,(const) shr 8,(const) and 0FFh
  end if
end macro

macro iload index
  if index<0
    err "invalid index"
  else if index<=3
    db 0x1a+index
  else if index<100h
    db 0x15,index
  else
    db 0xc4,0x15,(index) shr 8,(index) and 0FFh
  end if
end macro

macro imul
  db 0x68
end macro

macro ineg
  db 0x74
end macro

macro instanceof index
  db 0xc1,(index) shr 8,(index) and 0FFh
end macro

macro invokedynamic index
  db 0xba,(index) shr 8,(index) and 0FFh,0,0
end macro

macro invokeinterface index,count
  db 0xb9,(index) shr 8,(index) and 0FFh,count
end macro

macro invokespecial index
  db 0xb7,(index) shr 8,(index) and 0FFh
end macro

macro invokestatic index
  db 0xb8,(index) shr 8,(index) and 0FFh
end macro

macro invokevirtual index
  db 0xb6,(index) shr 8,(index) and 0FFh
end macro

macro ior
  db 0x80
end macro

macro irem
  db 0x70
end macro

macro ireturn
  db 0xac
end macro

macro ishl
  db 0x78
end macro

macro ishr
  db 0x7a
end macro

macro istore index
  if index<0
    err "invalid index"
  else if index<=3
    db 0x3b+index
  else if index<100h
    db 0x36,index
  else
    db 0xc4,0x36,(index) shr 8,(index) and 0FFh
  end if
end macro

macro isub
  db 0x64
end macro

macro iushr
  db 0x7c
end macro

macro ixor
  db 0x82
end macro

macro jsr branch
  offset = branch-$
  if offset>=-8000h & offset<8000h
    db 0xa8,offset shr 8,offset and 0FFh
  else
    db 0xc9,offset shr 24,(offset shr 16) and 0FFh,(offset shr 8) and 0FFh,offset and 0FFh
  end if
end macro

macro jsr_w branch
  offset = branch-$
  db 0xc9,offset shr 24,(offset shr 16) and 0FFh,(offset shr 8) and 0FFh,offset and 0FFh
end macro

macro l2d
  db 0x8a
end macro

macro l2f
  db 0x89
end macro

macro l2i
  db 0x88
end macro

macro ladd
  db 0x61
end macro

macro laload
  db 0x2f
end macro

macro land
  db 0x7f
end macro

macro lastore
  db 0x50
end macro

macro lcmp
  db 0x94
end macro

macro lconst_0
  db 0x09
end macro

macro lconst_1
  db 0x0a
end macro

macro ldc index
  if index<0
    err "invalid index"
  else if index<100h
   db 0x12,index
  else
   db 0x13,(index) shr 8,(index) and 0FFh
  end if
end macro

macro ldc_w index
  db 0x13,(index) shr 8,(index) and 0FFh
end macro

macro ldc2_w index
  db 0x14,(index) shr 8,(index) and 0FFh
end macro

macro ldiv
  db 0x6d
end macro

macro lload index
  if index<0
    err "invalid index"
  else if index<=3
    db 0x1e+index
  else if index<100h
    db 0x16,index
  else
    db 0xc4,0x16,(index) shr 8,(index) and 0FFh
  end if
end macro

macro lmul
  db 0x69
end macro

macro lneg
  db 0x75
end macro

macro lookupswitch
;  db 0xab,...
  err "not implemented yet"
end macro

macro lor
  db 0x81
end macro

macro lrem
  db 0x71
end macro

macro lreturn
  db 0xad
end macro

macro lshl
  db 0x79
end macro

macro lshr
  db 0x7b
end macro

macro lstore index
  if index<0
    err "invalid index"
  else if index<=3
    db 0x3f+index
  else if index<100h
    db 0x37,index
  else
    db 0xc4,0x37,(index) shr 8,(index) and 0FFh
  end if
end macro

macro lsub
  db 0x65
end macro

macro lushr
  db 0x7d
end macro

macro lxor
  db 0x83
end macro

macro monitorenter
  db 0xc2
end macro

macro monitorexit
  db 0xc3
end macro

macro multianewarray index,dimensions
  db 0xc5,(index) shr 8,(index) and 0FFh,dimensions
end macro

macro new index
  db 0xbb,(index) shr 8,(index) and 0FFh
end macro

macro newarray atype
  db 0xbc,atype
end macro

macro nop
  db 0x00
end macro

macro pop
  db 0x57
end macro

macro pop2
  db 0x58
end macro

macro putfield index
  db 0xb5,(index) shr 8,(index) and 0FFh
end macro

macro putstatic index
  db 0xb3,(index) shr 8,(index) and 0FFh
end macro

macro ret index
  if index<0
    err "invalid index"
  else if index<100h
    db 0xa9,index
  else
    db 0xc4,0xa9,(index) shr 8,(index) and 0FFh
  end if
end macro

macro return
  db 0xb1
end macro

macro saload
  db 0x35
end macro

macro sastore
  db 0x56
end macro

macro sipush short
  db 0x11,(short) shr 8,(short) and 0FFh
end macro

macro swap
  db 0x5f
end macro

macro tableswitch
;  db 0xaa,...
  err "not implemented yet"
end macro

macro breakpoint
  db 0xca
end macro

macro impdep1
  db 0xfe
end macro

macro impdep2
  db 0xff
end macro
