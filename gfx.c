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

int graphicsmode = 0;

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
    6,  /* R10 Cursor start */
    7,   /* R11 Cursor end */
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


void raster_loop_frames(void);

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

unsigned int row_multiplier = 2;
unsigned int split_rows = 344;
unsigned int gfxlines = 86;
unsigned int textline = 86;

void SetRows(int rows) {
    gfxlines = rows;
    split_rows = rows * row_multiplier;
    textline = gfxlines;
}

void SetTextLine(int line){
    textline = line;
}



volatile unsigned char keybuf[KEYBUF_SIZE];
volatile unsigned int  keybuf_head = 0;
volatile unsigned char last_keybyte = 0;

void raster_loop_frames(void);

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
    /* ---- XT keyboard sample + acknowledge ---- */ \
    "in  al,60h" \
    "cmp al,last_keybyte" \
    "je  kb_skip" \
    "mov last_keybyte,al" \
    "mov si,keybuf_head" \
    "mov keybuf[si],al" \
    "inc si" \
    "and si,0FFh" \
    "mov keybuf_head,si" \
"h3:" \
    "in  al,dx" \
    "test al,01h" \
    "jnz h3" \
"h4:" \
    "in  al,dx" \
    "test al,01h" \
    "jz  h4" \
    /* strobe bit 7 of port 0x61 */ \
    "in  al,61h" \
    "or  al,80h" \
    "out 61h,al" \
    "and al,7Fh" \
    "out 61h,al" \
    "dec cx" \
    "kb_skip:" \
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
    "sti" \
    "cli" \
"h2:" \
    "in  al,dx" \
    "test al,01h" \
    "sti" \
    "cli" \
    "jz  h2" \
    /* ---- XT keyboard sample + acknowledge ---- */ \
    "in  al,60h" \
    "cmp al,last_keybyte" \
    "je  kb_skip" \
    "mov last_keybyte,al" \
    "mov si,keybuf_head" \
    "mov keybuf[si],al" \
    "inc si" \
    "and si,0FFh" \
    "mov keybuf_head,si" \
    /* strobe bit 7 of port 0x61 */ \
    "in  al,61h" \
    "or  al,80h" \
    "out 61h,al" \
    "and al,7Fh" \
    "out 61h,al" \
"kb_skip:" \
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

void DrawChar(unsigned int x, unsigned int y, unsigned char data) {
    unsigned char far *screenpt;
    screenpt = text_mem + (y * 160) + (2 * x);
    *screenpt++ = data;
    *screenpt = 0x07;
}

void DrawTextColor(unsigned int x, unsigned int y, unsigned char color, unsigned char *data) {
    unsigned char far *screenpt;

    screenpt = text_mem + (y * 160) + (2 * x);

    while (*data){
        // newline
        if (*data == '\\' && *(data+1) == 'n') {
            data += 2;
            y++;
            screenpt = text_mem + (y * 160) + (2 * x);
        }
        *screenpt++ = *data++;
        *screenpt++ = color;
    }
}

void * DrawPoint;

void ClearLine(int line){
    DrawTextColor(0, line, 0x0F, "                                                                                ");
}

void ClearScreen(){
    memset(text_mem, 0x00, 16384);
}

void DisplayText(char *text){

    ClearLine(textline);
    ClearLine(textline+1);
    DrawTextColor(2, textline, 0x07, text);

}

// Simple RLE decoder for gfx
void Decode(char *gfx, int length) {
    char *writepnt = text_mem;
    unsigned int Numbytes;

    //gfx[0] is value
    //gfx[1] is number bytes
    length += 2;
    while (length-=2) {
        Numbytes = gfx[1]+1;
        while (Numbytes--) {
            writepnt[0] = 0xDD; // TODO this doesn't have to be done for every screen change
            writepnt[1] = gfx[0];
            writepnt += 2;
        }
        gfx += 2;
    }
}

void DisplayGFX(int id){
    memset(text_mem, 0x00, gfxlines * 160);
    if (Graphics[id].Length != 0) {
        Decode(Graphics[id].Data, Graphics[id].Length);
    }
}

void enable_cursor(unsigned char cursor_start, unsigned char cursor_end)
{
        outp(0x3D4, 0x0A);
        outp(0x3D5, (inp(0x3D5) & 0xC0) | cursor_start);

        outp(0x3D4, 0x0B);
        outp(0x3D5, (inp(0x3D5) & 0xE0) | cursor_end);
}

void update_cursor(int x, int y)
{
        unsigned int pos = y * 80 + x;

        outp(0x3D4, 0x0F);
        outp(0x3D5, (unsigned char) (pos & 0xFF));
        outp(0x3D4, 0x0E);
        outp(0x3D5, (unsigned char) ((pos >> 8) & 0xFF));
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

void GFX_DrawScreenSplit() {
    // draw first x lines in 160x100 mode
    if (gfxlines) {
        switch (graphicsmode) {
            case 0x31:
                    raster_loop_250_frames_cga();
                break;

            case 0x34:
                    raster_loop_250_frames_vga();
                break;
        }
    }
}

void GFX_Init() {
    printf("1 CGA, 2 EGA200, 3 EGA350, 4 VGA, 5 VGA 200\n");

    graphicsmode = getch();


    switch (graphicsmode){
        case 0x31:
            row_multiplier = 2;
            set_160x100_mode_cga();
            break;

        case 0x32:
            set_160x100_mode_ega200();
            break;

        case 0x33:
            set_160x100_mode_ega();
            break;

        case 0x34:
            row_multiplier = 4;
            set_160x100_mode_vga();
            break;

        case 0x35:
            set_160x100_mode_vga_200();
            break;

        default:
            printf("invalid selection %x\n", graphicsmode);
            exit(0);
    }

    rasterEnable();

    memset(Graphics, 0x00, sizeof(Graphics));

    LoadGFX(GFX_MENU, "1.bin");
    LoadGFX(GFX_STANDING, "2.bin");
    LoadGFX(GFX_STANDINGPANTSOFF, "3.bin");
    LoadGFX(GFX_DOOROPEN, "4.bin");
    LoadGFX(GFX_DOOROPENPANTSOFF, "5.bin");
    LoadGFX(GFX_ONTOILET, "6.bin");
    LoadGFX(GFX_ONTOILETPANTSOFF, "7.bin");
    LoadGFX(GFX_AWARDS, "8.bin");
    LoadGFX(GFX_SHITONFLOOR, "10.bin");
    LoadGFX(GFX_SHITINTOILET, "11.bin");
    LoadGFX(GFX_SHITPANTSSTANDING, "12.bin");
    LoadGFX(GFX_SHITINPANTSSITTING, "13.bin");
    LoadGFX(GFX_DIEPANTSON, "18.bin");
    LoadGFX(GFX_DIEPANTSOFF, "19.bin");
    LoadGFX(GFX_PILLSSTANDINGPANTSON1, "22.bin");
    LoadGFX(GFX_PILLSSTANDINGPANTSON2, "23.bin");
    LoadGFX(GFX_PILLSSTANDINGPANTSOFF1, "26.bin");
    LoadGFX(GFX_PILLSSTANDINGPANTSOFF2, "27.bin");
    LoadGFX(GFX_PILLSSITTINGPANTSON1, "29.bin");
    LoadGFX(GFX_PILLSSITTINGPANTSON2, "30.bin");
    LoadGFX(GFX_PILLSSITTINGPANTSOFF2, "33.bin");
    LoadGFX(GFX_SHITINPANTSWHILEOFF, "39.bin");
    LoadGFX(GFX_DIEPANTSONSITTING, "42.bin");
    LoadGFX(GFX_DIEPANTSOFFSITTING, "43.bin");
    LoadGFX(GFX_SHITONBATHROOMFLOOR, "45.bin");
    LoadGFX(GFX_ELVIS, "46.bin");

}
