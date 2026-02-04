#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <graph.h>
#include <string.h>
#include <stdlib.h>

#include "gamelogic.h"

/* CGA ports */
#define CGA_CRTC_INDEX  0x3D4
#define CGA_CRTC_DATA   0x3D5
#define CGA_STATUS      0x3DA

#define CGA_MODE_CTRL   0x3D8

/* CGA text memory */
unsigned char far *text_mem = MK_FP(0xB000, 0x8000);

/* Disable/enable video output (prevents snow while programming CRTC) */
#define rasterDisable() outp(CGA_MODE_CTRL, 0x01)
#define rasterEnable()  outp(CGA_MODE_CTRL, 0x09)

/* 160x100 CGA CRTC setup */
unsigned char cga160crtc[] = {
    113, /* R0  Horizontal total */
    80,  /* R1  Horizontal displayed */
    89,  /* R2  Horizontal sync position */
    15,  /* R3  Sync width */
    127, /* R4  Vertical total */
    6,   /* R5  Vertical total adjust */
    100, /* R6  Vertical displayed */
    112, /* R7  Vertical sync position */
    2,   /* R8  Interlace mode */
    1,   /* R9  Max scan line */
    32,  /* R10 Cursor start */
    0,   /* R11 Cursor end */
    0,   /* R12 Start address high */
    0    /* R13 Start address low */
};


/* for co40
[R0] HorizontalTotal 56
[R1] HorizontalDisplayed 40
[R2] HorizontalSyncPosition 45
[R3] SyncWidth 10
[R4] VerticalTotal 31
[R5] VerticalTotalAdjust 6
[R6] VerticalDisplayed 25
[R7] VerticalSync 28
[R8] InterlaceMode 2
[R9] MaximumScanLineAddress 7
[R10] CursorStartLine 6
[R11] CursorEndLine 7
[R12] StartAddressH 0
[R13] StartAddressL 0
Start Address 0000
[R14] CursorAddressH 0
[R15] CursorAddressL 160
[R16] LightPenPositionH 0
[R17] LightPenPositionL 0
*/

Graphic Graphics[GFXCOUNT];

/* ---------- Video mode setup ---------- */

void set_160x100_mode_cga(void)
{
    union REGS r;
    int i;

    /* BIOS mode 3 (80x25 text) */
    r.h.ah = 0x00;
    r.h.al = 0x03;
    int86(0x10, &r, &r);

    /* disable blink */
    r.h.ah = 0x10;
    r.h.al = 0x03;
    r.h.bl = 0x00;
    int86(0x10, &r, &r);

    rasterDisable();

    for (i = 0; i < sizeof(cga160crtc); i++)
    {
        outp(CGA_CRTC_INDEX, i);
        outp(CGA_CRTC_DATA, cga160crtc[i]);
    }
}

void set_160x100_mode_ega200(void)
{
    union REGS r;
    int i;

    /* BIOS mode 3 (80x25 text) */
    r.h.ah = 0x00;
    r.h.al = 0x03;
    int86(0x10, &r, &r);

    /* disable blink */
    r.h.ah = 0x10;
    r.h.al = 0x03;
    r.h.bl = 0x00;
    int86(0x10, &r, &r);

    rasterDisable();

    // only need to set 2 scanline mode
    outp(CGA_CRTC_INDEX, 9);
    outp(CGA_CRTC_DATA, 1);
}

void set_160x100_mode_ega(void)
{
    union REGS r;
    int i;

    /* BIOS mode 3 (80x25 text) */
    r.h.ah = 0x00;
    r.h.al = 0x03;
    int86(0x10, &r, &r);

    /* disable blink */
    r.h.ah = 0x10;
    r.h.al = 0x03;
    r.h.bl = 0x00;
    int86(0x10, &r, &r);

    rasterDisable();

    // only need to set 3 scanline mode
    outp(CGA_CRTC_INDEX, 9);
    outp(CGA_CRTC_DATA, 2);
}

void set_160x100_mode_vga(void)
{
    union REGS r;
    int i;

    /* BIOS mode 3 (80x25 text) */
    r.h.ah = 0x00;
    r.h.al = 0x03;
    int86(0x10, &r, &r);

    /* disable blink */
    r.h.ah = 0x10;
    r.h.al = 0x03;
    r.h.bl = 0x00;
    int86(0x10, &r, &r);

    rasterDisable();

    // only need to set 4 scanline mode
    outp(CGA_CRTC_INDEX, 9);
    outp(CGA_CRTC_DATA, 3);
}

void set_160x100_mode_vga_200(void)
{
    union REGS r;
    int i;

    /* BIOS mode 3 (80x25 text) */
    //r.h.ah = 0x00;
    //r.h.al = 0x03;
    //int86(0x10, &r, &r);

    /* disable blink */
    r.h.ah = 0x10;
    r.h.al = 0x03;
    r.h.bl = 0x00;
    int86(0x10, &r, &r);

    /* 200 line mode */
    r.h.ah = 0x12;
    r.h.al = 0x00;
    r.h.bl = 0x30;
    int86(0x10, &r, &r);

    //rasterDisable();

    // only need to set 2 scanline mode
    // TODO, this seems to reset the VGA back to 400-line mode, so this mode doesn't work
    outp(CGA_CRTC_INDEX, 9);
    outp(CGA_CRTC_DATA, 1);
}

void raster_loop_250_frames(void);

unsigned int split_rows = 344;

#pragma aux raster_loop_250_frames_cga = \
    "cli" \
    "mov bx,250"            /* frame counter */ \
"frame_loop:" \
    "mov dx,03DAh"          /* CGA status port */ \
    /* wait for VBLANK start */ \
"vb1:" \
    "in  al,dx" \
    "test al,08h" \
    "jnz  vb1" \
"vb2:" \
    "in  al,dx" \
    "test al,08h" \
    "jz vb2" \
    /* restore 2-scanline rows (CRTC reg 9 = 1) */ \
    "mov dx,03D4h" \
    "mov al,09h" \
    "out dx,al" \
    "inc dx" \
    "mov al,01h" \
    "out dx,al" \
    /* back to CGA status */ \
    "mov dx,03DAh" \
    "mov cx,split_rows"           /* split at scanline 100 */ \
"scan_loop:" \
"h1:" \
    "in  al,dx" \
    "test al,01h" \
    "jnz h1" \
"h2:" \
    "in  al,dx" \
    "test al,01h" \
    "jz  h2" \
    "loop scan_loop" \
    /* switch to 8 scanline rows */ \
    "mov dx,03D4h" \
    "mov al,09h" \
    "out dx,al" \
    "inc dx" \
    "mov al,07h" \
    "out dx,al" \
    /* next frame */ \
    "dec bx" \
    "sti" \
    modify [ax bx cx dx];

#pragma aux raster_loop_250_frames_ega = \
    "cli" \
    "mov bx,250"            /* frame counter */ \
"frame_loop:" \
    "mov dx,03DAh"          /* CGA status port */ \
    /* wait for VBLANK start */ \
"vb1:" \
    "in  al,dx" \
    "test al,08h" \
    "jnz  vb1" \
"vb2:" \
    "in  al,dx" \
    "test al,08h" \
    "jz vb2" \
    /* restore 3-scanline rows (CRTC reg 9 = 2) */ \
    "mov dx,03D4h" \
    "mov al,09h" \
    "out dx,al" \
    "inc dx" \
    "mov al,02h" \
    "out dx,al" \
    /* back to CGA status */ \
    "mov dx,03DAh" \
    "mov cx,258"           /* split at scanline 100 */ \
"scan_loop:" \
"h1:" \
    "in  al,dx" \
    "test al,01h" \
    "jnz h1" \
"h2:" \
    "in  al,dx" \
    "test al,01h" \
    "jz  h2" \
    "loop scan_loop" \
    /* switch to 14 scanline rows */ \
    "mov dx,03D4h" \
    "mov al,09h" \
    "out dx,al" \
    "inc dx" \
    "mov al,13" \
    "out dx,al" \
    /* next frame */ \
    "dec bx" \
    "jnz frame_loop" \
    "sti" \
    modify [ax bx cx dx];

#pragma aux raster_loop_250_frames_vga = \
    "cli" \
    "mov bx,250"            /* frame counter */ \
"frame_loop:" \
    "mov dx,03DAh"          /* CGA status port */ \
    /* wait for VBLANK start */ \
"vb1:" \
    "in  al,dx" \
    "test al,08h" \
    "jnz  vb1" \
"vb2:" \
    "in  al,dx" \
    "test al,08h" \
    "jz vb2" \
    /* restore 4-scanline rows (CRTC reg 9 = 3) */ \
    "mov dx,03D4h" \
    "mov al,09h" \
    "out dx,al" \
    "inc dx" \
    "mov al,03h" \
    "out dx,al" \
    /* back to CGA status */ \
    "mov dx,03DAh" \
    "mov cx,split_rows"           /* split at scanline 100 */ \
"scan_loop:" \
"h1:" \
    "in  al,dx" \
    "test al,01h" \
    "jnz h1" \
"h2:" \
    "in  al,dx" \
    "test al,01h" \
    "jz  h2" \
    "loop scan_loop" \
    /* switch to 14 scanline rows */ \
    "mov dx,03D4h" \
    "mov al,09h" \
    "out dx,al" \
    "inc dx" \
    "mov al,15" \
    "out dx,al" \
    /* next frame */ \
    "dec bx" \
    "sti" \
    modify [ax bx cx dx];

void DrawText(unsigned int x, unsigned int y, unsigned char *data) {
    unsigned char far *screenpt;

    screenpt = text_mem + (y * 160) + (2 * x);

    while (*data){
        *screenpt++ = *data++;
        *screenpt++ = 0x07;
    }
}

void DrawTextColor(unsigned int x, unsigned int y, unsigned char color, unsigned char *data) {
    unsigned char far *screenpt;

    screenpt = text_mem + (y * 160) + (2 * x);

    while (*data){
        *screenpt++ = *data++;
        *screenpt++ = color;
    }
}

void * DrawPoint;

void ClearLine(int line){
    DrawText(0, line, "                                                                                ");
}

void DisplayText(char *text){
    ClearLine(86);
    DrawText(10, 86, text);
}

void Decode(char *gfx, int length) {
    char *writepnt = text_mem;
    unsigned int Numbytes;

    //gfx[0] is value
    //gfx[1] is number bytes
    while (length-=2) {
        Numbytes = gfx[1]+1;
        while (Numbytes--) {
            writepnt[0] = 0xDD;
            writepnt[1] = gfx[0];
            writepnt += 2;
        }
        gfx += 2;
    }
}

void DisplayGFX(int id){
    memset(text_mem, 0x00, 16000);
    Decode(Graphics[id].Data, Graphics[id].Length);
}

void LoadGFX(int num, char * filename) {
    FILE *infile;
    infile = fopen(filename, "rb");
    if (!infile){
        printf("Can't open %s\n", filename);
        exit (1);
    }


    fseek(infile, 0, SEEK_END);
    Graphics[num].Length = ftell(infile);
    fseek(infile, 0, SEEK_SET);  /* same as rewind(f); */

    Graphics[num].Data = malloc(Graphics[num].Length + 1);
    fread(Graphics[num].Data, Graphics[num].Length, 1, infile);
    fclose(infile);
}


int main(void)
{
    union REGS r;

    int i;
    char buf[10];
    char inkey;

    char InputBuff[100];
    int bufpos = 0;
    long fsize;
    char *ReadGfx;

    memset(InputBuff, 0x00, 100);    

    Gamelogic_Init();

    printf("1 CGA, 2 EGA200, 3 EGA350, 4 VGA, 5 VGA 200\n");

    i = getch();    
    

    switch (i){
        case 0x31:
            set_160x100_mode_cga();
            break;

        case 0x32:
            set_160x100_mode_ega200();
            break;

        case 0x33:
            set_160x100_mode_ega();
            break;

        case 0x34:
            set_160x100_mode_vga();
            break;
        
        case 0x35:
            set_160x100_mode_vga_200();
            break;

        default:
            printf("invalid selection %x\n", i);
            exit(0);
    }



    rasterEnable();

    LoadGFX(GFX_MENU, "1.bin");
    LoadGFX(GFX_STANDING, "2.bin");
    LoadGFX(GFX_STANDINGPANTSOFF, "3.bin");
    LoadGFX(GFX_DOOROPEN, "4.bin");
    LoadGFX(GFX_DOOROPENPANTSOFF, "5.bin");
    LoadGFX(GFX_ONTOILET, "6.bin");
    LoadGFX(GFX_ONTOILETPANTSOFF, "7.bin");
    LoadGFX(GFX_SHITONFLOOR, "8.bin");
    
    /*DrawText(10, 86, "The quick brown fox jumped over the lazy god.");
    DrawText(10, 87, "The quick brown fox jumped over the lazy god.");
    DrawText(10, 88, "The quick brown fox jumped over the lazy god.");*/

        switch (i){
        case 0x31:
            for (i = 0; i < 100; i++){
                raster_loop_250_frames_cga();
                    if (kbhit()){
                        sprintf(buf, "hit%02X\n", getch());
                        DrawText(10, 86, buf);
                    }
                        
            }
            break;

        case 0x34:
            while (strcmp(InputBuff, "exit") != 0) {
                raster_loop_250_frames_vga();
                    if (kbhit()){
                        inkey = getch();
                        //delete
                        if (inkey == '\b'){
                            if (bufpos) {
                                bufpos--;
                                InputBuff[bufpos] = 0;
                                ClearLine(88);
                                DrawText(0, 88, InputBuff);
                            }
                        }
                        //enter
                        else if (inkey == '\r'){
                            GameLogic_TextInput(InputBuff);
                            bufpos = 0;
                            InputBuff[bufpos] = 0;
                            InputBuff[bufpos+1] = 0;
                            ClearLine(88);
                            DrawText(0, 88, InputBuff);
                        }
                        else {
                            InputBuff[bufpos] = inkey;
                            InputBuff[bufpos+1] = 0;
                            ClearLine(88);
                            DrawText(0, 88, InputBuff);
                            bufpos++;
                        }
                    }
                        
            }
            break;

        default:
            exit(0);
    }


    /* Restore normal text mode */
    r.h.ah = 0x00;
    r.h.al = 0x03;
    int86(0x10, &r, &r);

    return 0;
}