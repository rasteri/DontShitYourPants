#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <graph.h>
#include <string.h>
#include <stdlib.h>

#include "gamelogic.h"


/* CGA text memory */
unsigned char far *text_mem = MK_FP(0xB000, 0x8000);

int CrownX = 0, CrownY = 0;

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

    for (i = 0; i < sizeof(cga160crtc); i++)
    {
        outp(CGA_CRTC_INDEX, i);
        outp(CGA_CRTC_DATA, cga160crtc[i]);
    }
}

unsigned char OldEGASwitches;
unsigned char far *ega_switches;

void set_160x100_mode_ega200(void)
{
    union REGS r;

    ega_switches = (unsigned char far *)MK_FP(0x40, 0x88);

    OldEGASwitches = *ega_switches;

    /* Clear EGA 350-line flag (bit 3) -> force 200-line mode */
    *ega_switches &= ~(1 << 3);

    /* BIOS mode 3 (80x25 text) */
    r.h.ah = 0x00;
    r.h.al = 0x03;
    int86(0x10, &r, &r);

    /* disable blink */
    r.h.ah = 0x10;
    r.h.al = 0x03;
    r.h.bl = 0x00;
    int86(0x10, &r, &r);

    // only need to set 2 scanline mode
    outp(CGA_CRTC_INDEX, 9);
    outp(CGA_CRTC_DATA, 1);
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


    // only need to set 4 scanline mode
    outp(CGA_CRTC_INDEX, 9);
    outp(CGA_CRTC_DATA, 3);
}

void raster_loop_250_frames(void);

unsigned int row_multiplier = 2;
unsigned int split_rows = 344;
unsigned int gfxlines = 86;
unsigned int textline = 86;

unsigned char gfx_rows = 1;
unsigned char text_rows = 9;

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

#pragma aux raster_split = \
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
    /* ---- XT keyboard sample + acknowledge */ \
    "in  al,60h" \
    "cmp al,last_keybyte" \
    "je  kb_skip1" \
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
    "kb_skip1:" \
    \
    "in  al,dx" \
    "test al,08h" \
    "jz vb2" \
    /* restore 2/4-scanline rows (CRTC reg) */ \
    "mov dx,03D4h" \
    "mov al,09h" \
    "out dx,al" \
    "inc dx" \
    "mov al,gfx_rows" \
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
    /* Don't check keyboard if we only have 1 scanline left because otherwise we'll underflow cx */ \
    "cmp cx,01h" \
    "je kb_skip" \
    /* ---- XT keyboard sample + acknowledge  */ \
    "in  al,60h" \
    "cmp al,last_keybyte" \
    "je  kb_skip" \
    "mov last_keybyte,al" \
    "mov si,keybuf_head" \
    "mov keybuf[si],al" \
    "inc si" \
    "and si,0FFh" \
    "mov keybuf_head,si" \
    /* wait for next scanline because we don't have enough time left in this scanline*/ \
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
    /* switch to 8/16 scanline rows */ \
    "mov dx,03D4h" \
    "mov al,09h" \
    "out dx,al" \
    "inc dx" \
    "mov al,text_rows" \
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
// Sprites can be transparent and aren't RLE'd
// first byte is just colour
//second byte is
// bit0 - transparent first pixel
// bit1 - transparent second pixel
// bit2 - newline
void DecodeSprite(char *gfx, int length, int x, int y) {
    char *writepnt = text_mem + (y * 160) + (2 * x);
    unsigned int Numbytes;
    unsigned char writebyte = 0;

    length += 2;

    while (length -= 2) {
        writebyte = 0;

        if ((gfx[1] & 0x01)){
            writebyte |= writepnt[1] & 0x0F;
        } else {
            writebyte |= gfx[0] & 0x0F;
        }
        if ((gfx[1] & 0x02)){
            writebyte |= writepnt[1] & 0xF0;
        } else {
            writebyte |= gfx[0] & 0xF0;
        }

        if (y >= 0){
            writepnt[0] = 0xDD;
            writepnt[1] = writebyte;
        }

        //newline   
        if ((gfx[1] & 0x04)){
            y++;
            writepnt = text_mem + (y * 160) + (2 * x);
        }
        else writepnt += 2;
        gfx += 2;
    }
}

void DisplayGFX(int id){
    rasterDisable();
    memset(text_mem, 0x00, gfxlines * 160);
    if (Graphics[id].Length != 0) {
        Decode(Graphics[id].Data, Graphics[id].Length);
    }
    rasterEnable();
}

void GFX_DrawSprite(int id, int x, int y){
    rasterDisable();
    DecodeSprite(Graphics[id].Data, Graphics[id].Length, x, y);
    rasterEnable();
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
        raster_split();
    }
}

void GFX_Exit() {
    union REGS r;
    if (graphicsmode == 0x32){
        *ega_switches = OldEGASwitches;
    }

    /* Restore normal text mode */
    r.h.ah = 0x00;
    r.h.al = 0x03;
    int86(0x10, &r, &r);
    
}

void GFX_Init() {
    printf("1 CGA, 2 EGA, 3 VGA\n");

    graphicsmode = getch();

    switch (graphicsmode){
        case 0x31:
            row_multiplier = 2;
            gfx_rows = 1;
            text_rows = 7;
            set_160x100_mode_cga();
            break;

        case 0x32:
            row_multiplier = 2;
            gfx_rows = 1;
            text_rows = 7;
            set_160x100_mode_ega200();
            break;

        case 0x33:
            row_multiplier = 4;
            gfx_rows = 3;
            text_rows = 15;
            set_160x100_mode_vga();
            break;

        default:
            printf("invalid selection %x\n", graphicsmode);
            exit(0);
    }

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
    LoadGFX(GFX_CROWN, "crown.bin");
}
