
; TetrOS
; version 1.01 (05-09-2004)
; coded by Tomasz Grysztar ("Privalov")

; For your playing pleasure, it's a boot-sector Tetris game.
; The quick help on keys:
;   Left - move left
;   Right - move right
;   Up - rotate
;   Down - drop
;   Esc - new game at any time
; Requires VGA and 386 or higher processor.

include 'cpu/80386.inc'
include 'macro/@@.inc'

format binary as 'img'
org 7C00h

ROWS = 23
DELAY = 4

virtual at 46Ch
  clock dw ?
end virtual

virtual at bp
  current dw ?
  current_column db ?
  current_row dw ?
  last_tick dw ?
  delay dw ?
  random dw ?
  score dw ?
end virtual

label well at 9000h
label pics at well-2*64

        cli
        xor        ax,ax
        mov        ss,ax
        mov        ds,ax
        mov        es,ax
        mov        sp,0FFFEh
        sti
        push        ax
        push        start
        retf

start:
        mov        al,13h
        int        10h
        mov        di,3*4
        mov        ax,int_3
        stosw
        xor        ax,ax
        stosw

        mov        bp,8000h

        mov        di,pics
        mov        cx,64
        inc        ax
        rep        stosb
        mov        ah,15
        mov        dx,7
      @@:
        mov        al,15
        stosb
        mov        al,ah
        mov        cl,6
        rep        stosb
        mov        al,8
        stosb
        mov        ah,7
        dec        dx
        jnz        @b
        mov        cl,8
        rep        stosb

        mov        ax,[clock]
        mov        [last_tick],ax
        mov        [random],ax
        mov        byte [current_row+1],well shr 8

        xor        ax,ax
        mov        [score],ax
        dec        ax
        stosw
        stosw
        stosw
        mov        cl,ROWS
    new_piece:
        mov        ax,1100000000000011b
        rep        stosw
      @@:
        mov        bx,[random]
        mov        ax,257
        mul        bx
        inc        ax
        mov        cx,43243
        div        cx
        mov        [random],dx
        and        bx,7
        cmp        bx,7
        je        @b
        shl        bx,1
        mov        ax,[pieces+bx]
        mov        [current],ax
        mov        word [current_column],6 + ((3+ROWS-4)*2) shl 8
        xor        ch,ch
        mov        ax,test_piece
        int3
        mov        al,draw_piece and 0FFh
        int3
        or        ch,ch
        jz        update_screen
        xor        bp,bp

process_key:
        xor        ah,ah
        int        16h
        mov        al,ah
        dec        al
        jz        start
        or        bp,bp
        jz        process_key
        mov        si,rotate
        cmp        al,48h-1
        je        action
        mov        si,left
        cmp        al,4Bh-1
        je        action
        mov        si,right
        cmp        al,4Dh-1
        je        action
        cmp        al,50h-1
        je        drop_down
        jmp        main_loop

action:
        call        do_move
        jmp        update_screen

drop_down:
        mov        si,down
        call        do_move
        jz        drop_down

update_screen:
        mov        bx,7
        mov        dx,12h
        mov        ah,2
        int        10h
        mov        cl,12
      @@:
        mov        ax,[score]
        shr        ax,cl
        and        al,0Fh
        cmp        al,10
        sbb        al,69h
        das
        mov        ah,0Eh
        int        10h
        sub        cl,4
        jnc        @b
        push        es
        push        0A000h
        pop        es
        mov        si,well+3*2
        mov        di,320*184+112
      draw_well:
        lodsw
        push        si
        xchg        bx,ax
        shr        bx,2
        mov        dl,12
      draw_row:
        shr        bx,1
        salc
        and        ax,64
        mov        si,pics
        add        si,ax
        mov        al,8
      copy_line:
        mov        cx,8
        rep        movsb
        add        di,320-8
        dec        ax
        jnz        copy_line
        sub        di,320*8-8
        dec        dx
        jnz        draw_row
        sub        di,320*8+12*8
        pop        si
        cmp        si,well+(3+ROWS)*2
        jb        draw_well
        pop        es

main_loop:
        mov        ah,1
        int        16h
        jnz        process_key
        mov        ax,[clock]
        sub        ax,[last_tick]
        cmp        al,DELAY
        jb        main_loop
        add        [last_tick],ax
        mov        si,down
        call        do_move
        jz        update_screen
    lay_piece:
        mov        dx,1
        mov        si,well+3*2
        mov        di,si
        mov        cx,ROWS
      check_row:
        lodsw
        cmp        ax,1111111111111111b
        je        remove_row
        stosw
        dec        cx
        jmp        check_next_row
      remove_row:
        shl        dx,1
      check_next_row:
        cmp        si,well+(3+ROWS)*2
        jb        check_row
        add        [score],dx
        jmp        new_piece

do_move:
        mov        ax,clear_piece
        int3
        push        dword [current]
        call        si
        xor        ch,ch
        mov        al,test_piece and 0FFh
        int3
        mov        al,draw_piece and 0FFh
        pop        edx
        or        ch,ch
        jz        @f
        mov        dword [current],edx
      @@:
        int3
      no_move:
        ret
down:
        sub        byte [current_row],2
        ret
left:
        dec        [current_column]
        ret
right:
        inc        [current_column]
        ret
rotate:
        mov        cx,3
     @@:
        bt        [current],cx
        rcl        dx,1
        add        cl,4
        cmp        cl,16
        jb        @b
        sub        cl,17
        jnc        @b
        mov        [current],dx
        ret

int_3:
        mov        di,[current_row]
        mov        bx,4
      on_piece_row:
        mov        dx,[current]
        mov        cl,bh
        shr        dx,cl
        and        dx,1111b
        mov        cl,[current_column]
        add        cl,4
        shl        edx,cl
        shr        edx,4
        call        ax
        add        bh,4
        scasw
        dec        bl
        jnz        on_piece_row
        iret

clear_piece:
        not        dx
        and        [di],dx
        ret
test_piece:
        test        [di],dx
        jz        @f
        or        ch,1
      @@:
        ret
draw_piece:
        or        [di],dx
        ret

pieces dw 0010001000100010b
       dw 0010011000100000b
       dw 0010001001100000b
       dw 0100010001100000b
       dw 0000011001100000b
       dw 0100011000100000b
       dw 0010011001000000b

rb 7C00h+510-$
dw 0AA55h
