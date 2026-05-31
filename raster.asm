.8086
.model large
.code

PUBLIC raster_split_nopoll_
PUBLIC raster_waitvsync_

EXTRN _SplitAtLine:WORD
EXTRN _BelowSplitMode:BYTE
EXTRN _AboveSplitMode:BYTE
EXTRN _GFXRegisterMode:BYTE

raster_waitvsync_ PROC

    mov dx,03DAh          ; CGA status port

; wait for VBLANK start
v1:
    in  al,dx
    test al,08h
    jnz  v1


v2:
    in  al,dx
    test al,08h
    jz   v2

; wait till start of screen

h5:
    in  al,dx
    test al,01h
    jnz h5

h6:
    in  al,dx
    test al,01h
    jz  h6

raster_waitvsync_ ENDP


raster_split_nopoll_ PROC

    mov dx,03DAh          ; CGA status port

; wait for VBLANK start
vb1:
    in  al,dx
    test al,08h
    jnz  vb1


vb2:
    in  al,dx
    test al,08h
    jz   vb2

    cli


; wait till start of screen before switching to above mode
; otherwise vsync will have a wonky number of lines

h3:
    in  al,dx
    test al,01h
    jnz h3

h4:
    in  al,dx
    test al,01h
    jz  h4

; switch to above mode
    mov dx,03D4h
    mov al,09h
    out dx,al
    inc dx
    mov al,_AboveSplitMode
    out dx,al

; Count scanlines by polling hblank register in tight loop
    mov dx,03DAh
    mov cx,_SplitAtLine

scan_loop:
h1:
    in  al,dx
    test al,01h
    jnz h1

h2:
    in  al,dx
    test al,01h
    jz  h2
    loop scan_loop

    ; switch to below mode
    mov dx,03D4h
    mov al,09h
    out dx,al
    inc dx
    mov al,_BelowSplitMode
    out dx,al

    sti
    ret

raster_split_nopoll_ ENDP

END