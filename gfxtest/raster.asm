.code

PUBLIC raster_split_nopoll_

EXTRN _SplitAtLine:WORD
EXTRN _BelowSplitMode:BYTE
EXTRN _AboveSplitMode:BYTE
EXTRN _GFXRegisterMode:BYTE
raster_split_nopoll_ PROC



frame_loop:

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

; switch to 100 line mode for start of screen no matter what
; this is to make sure CRTC calculates the correct scanline count
    mov dx,03D4h
    mov al,09h
    out dx,al
    inc dx
    mov al,_GFXRegisterMode
    out dx,al

; GA status
    mov dx,03DAh

; wait till start of screen before switching to above mode
; otherwise CRTC outputs way too many scanlines
; I guess because the CRTC's scanline calculation is done at the start of frame?
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