#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <graph.h>
#include <string.h>
#include <stdlib.h>

#include "gamelogic.h"

// CGA text memory
unsigned char far *text_mem = MK_FP(0xB000, 0x8000); 

// EGA graphics memory
unsigned char far *graphics_mem = (unsigned char far *)0xA0000000L; 

int CrownX = 0, CrownY = 0;

int graphicsmode = 0;

char TextAtTop = 0;

void far *inb, *outb;

Graphic Graphics[GFXCOUNT];

void DisableBlink(void) {
    union REGS r;
    r.h.ah = 0x10;
    r.h.al = 0x03;
    r.h.bl = 0x00;
    int86(0x10, &r, &r);
}



void Set_CGA_Register(unsigned char reg, unsigned char val) {
    if (graphicsmode == GFX_MODE_CGA) {
        outp(CGA_CRTC_INDEX, reg);
        outp(CGA_CRTC_DATA, val);
    }
}

void set_mode_cga(void)
{
    union REGS r;
    int i;

    rasterDisable();

    // BIOS mode 3 (80x25 text)
    r.h.ah = 0x00;
    r.h.al = 0x03;
    int86(0x10, &r, &r);

    DisableBlink();

    rasterEnable();
    MSPerFrame = 17;
    FramesPerSecond = 60;
}

void set_mode_ega(void)
{
    union REGS r;

    // set mode 0E (640x200x16)
    r.h.ah = 0x00;
    r.h.al = 0x0E;
    int86(0x10, &r, &r);

    // Disable Set/Reset on all planes
    outp(0x3CE, 0x00);
    outp(0x3CF, 0x00);

    // Disable Set/Reset on all planes
    outp(0x3CE, 0x01);
    outp(0x3CF, 0x00);

    // All planes writable
    outp(0x3CE, 0x02);
    outp(0x3CF, 0x0F);

    // Data Rotate = replace
    outp(0x3CE, 0x03);
    outp(0x3CF, 0x00);

    // Graphics Controller: write mode 0
    outp(0x3CE, 0x05);
    outp(0x3CF, 0x00);

    //Bit Mask = FFh
    outp(0x3CE, 0x08);
    outp(0x3CF, 0xFF);

    MSPerFrame = 17;
    TextAtTop = 0;
    FramesPerSecond = 60;
}

void set_mode_vga(void)
{
    set_mode_ega();
    MSPerFrame = 14;
    FramesPerSecond = 70;
}


// How many lines one char (i.e. one vertical "pixel" in 160x100 mode) take up
// 2 on CGA/EGA200, 3 on EGA350, 4 on VGA
unsigned int GFXLinesPerChar = 2;

// What CRTC R9 (Max scan line) is set to for GFX window
// 1 less than GFXLinesPerChar
unsigned int GFXRegisterMode = 1;

// How many scanlines a line of text takes up
// again varies depending on gfx card
unsigned int TextLinesPerChar = 8;

// The value that gets put in the CRTCs MaximumScanLineAddress reg above the split point
unsigned char AboveSplitMode;

// Line to split at, this is in raw screen output scanlines
unsigned int SplitAtLine;

// Value that gets put in the CRTCs MaximumScanLineAddress reg below the split point
unsigned char BelowSplitMode;

// The vertical gfx height in chars
// note this is not the number of scanlines, which will be 2x/3x/4x this depending on screen mode
unsigned int GFXVerticalHeight = 86;

// The vertical gfx height in scanlines
unsigned int GFXVerticalLines;

// The vertical height of the text window in chars
// note this is not the number of scanlines, which will be 2x/3x/4x this depending on screen modes
unsigned int TextVerticalHeight;

// The vertical text height in scanlines
unsigned int TextVerticalLines;

// What character line the text "window" begins at
unsigned int TextLine;

// What character line the text input window is
unsigned int InputLine;

// Char line to draw GFX at, always 0 when text at bottom
unsigned char GFXLine = 0;

// need to change when screen geometry changes : R4, R5, R6, R7. Also R9 but that's done in a loop
// For Text-at-bottom mode, 
// retracelen = 62 (i.e. 262-200)
// retracechars = int(62/8) = 7
// R5_vadj = retracelen - (retracechars * 8) = 6

// R6_vdisp = GFXVerticalHeight + TextVerticalHeight;
// R4_vtotal = vdisp + (retracechars - 1) = vdisp + 6
// R7_vsync = vdisp + 3 (24 lines)

// For Text-at-top mode, 
// retracelen = 62 (i.e. 262-200)
// retracechars = int(62/2) = 31
// R5_vadj = retracelen - (retracechars * 2) = 0

// R6_vdisp = GFXVerticalHeight + TextVerticalHeight;
// R4_vtotal = vdisp + (retracechars - 1) = vdisp + 30
// R7_vsync = vdisp + 12 (24 lines)


void RecalcScreenGeometry() {
    unsigned char vdisp;

    GFXVerticalLines = GFXVerticalHeight * 2;
    vdisp = GFXVerticalHeight + TextVerticalHeight;

    if (CurrState->ID == STATE_MENU || CurrState->ID == STATE_AWARDS || CurrState->ID == STATE_AWARDS2) {
        if (graphicsmode == GFX_MODE_CGA){
            GFXLine = 0;
            AboveSplitMode = 1;
            SplitAtLine = GFXVerticalHeight * 2;
            BelowSplitMode = 7;
            TextLine = GFXVerticalHeight;
            InputLine = 32;
            
            Set_CGA_Register(4, vdisp + 6);
            Set_CGA_Register(5, 6);
            Set_CGA_Register(6, vdisp);
            Set_CGA_Register(7, vdisp + 3);
        } else { 
            TextLine = GFXVerticalHeight >> 2;
            InputLine = 17;
        }

    } else if (graphicsmode == GFX_MODE_CGA) {
        TextVerticalLines = TextVerticalHeight * 8;
        vdisp = GFXVerticalHeight + TextVerticalHeight;
        GFXLine = TextVerticalHeight;

        AboveSplitMode = 7;
        SplitAtLine = TextVerticalLines;
        BelowSplitMode = 1;
        TextLine = 0;
        InputLine = 0;

        Set_CGA_Register(4, vdisp + 30);
        Set_CGA_Register(5, 0);
        Set_CGA_Register(6, vdisp);
        Set_CGA_Register(7, vdisp + 12);

    } else {
        InputLine = TextLine = GFXVerticalHeight >> 2;
    }
    
}

void SetGFXLines(int lines) {
    GFXVerticalHeight = lines;
    RecalcScreenGeometry();
}

void SetTextLines(int lines, char HideTextInput) {
    TextVerticalHeight = lines;
    RecalcScreenGeometry();
    if (!HideTextInput){
        ClearLine(2);
        DrawTextInInput(2, 2, 0x07, ">");
    }
}

void CGA_Unsplit(void) {
    if (graphicsmode == GFX_MODE_CGA) {
        raster_waitvsync();
        rasterDisable();
        Set_CGA_Register(4, 31);
        Set_CGA_Register(5,  6);
        Set_CGA_Register(6, 25);
        Set_CGA_Register(7, 28);
        Set_CGA_Register(9,  7);
    }
}

void CGA_Resplit(void) {
    if (graphicsmode == GFX_MODE_CGA) {
        raster_waitvsync();
        RecalcScreenGeometry();
        rasterEnable();
    }
}

volatile unsigned char keybuf[KEYBUF_SIZE];
volatile unsigned int  keybuf_head = 0;
volatile unsigned char last_keybyte = 0;

void raster_loop_frames(void);

void DrawChar(unsigned int x, unsigned int y, unsigned char data) {
    unsigned char far *screenpt;
    union REGS r;
    if (graphicsmode == GFX_MODE_CGA) {
        screenpt = text_mem + (y * 160) + (2 * x);
        *screenpt++ = data;
        *screenpt = 0x07;
    }
    else {
        r.h.ah = 0x02;
        r.h.bh = 0;
        r.h.dh = y;
        r.h.dl = x;
        int86(0x10, &r, &r);
        putch(data);
    }
}

void DrawText(unsigned int x, unsigned int y, unsigned char color, unsigned char *data) {

    unsigned char far *screenpt;
    union REGS r;
    unsigned int xn = x;

    // Just write screen buffer directly for CGA
    if (graphicsmode == GFX_MODE_CGA) {
        screenpt = text_mem + (y * 160) + (2 * x);

        while (*data) {
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

    // Use BIOS for EGA because I CBA writing a text renderer
    else {

        while (*data){
            // newline
            if (*data == '\\' && *(data+1) == 'n') {
                data += 2;
                y++;
                xn = x;
            }

            // set cursor pos
            r.h.ah = 0x02;
            r.h.bh = 0;
            r.h.dh = y;
            r.h.dl = xn++;
            int86(0x10, &r, &r);
            
            // write char
            r.h.ah = 0x09;
            r.h.al = *data++;
            r.h.bh = 0;
            r.h.bl = color;
            r.h.cl = 1;
            r.h.ch = 0;
            int86(0x10, &r, &r);
        }
    }
}

void DrawTextInWindow(unsigned int x, unsigned int y, unsigned char color, unsigned char *data) {
    DrawText(x, y + TextLine, color, data);
}

void DrawTextInInput(unsigned int x, unsigned int y, unsigned char color, unsigned char *data) {
    DrawText(x, y + InputLine, color, data);
}

void ClearLine(int line) {
    if (graphicsmode == GFX_MODE_CGA) {
        memset(text_mem + ((line + InputLine) * 160), 0x00, 160);
    } else {
        // Enable writing to all planes
        outp(0x3C4, 0x02);
        outp(0x3C5, 0xFF);

        outpw(0x3CE, 0xFF08); // bit mask
        memset(graphics_mem + ((line + InputLine) * 8 * 80), 0x00, 8 * 80);
    }
}

void ClearGFX() {
    if (graphicsmode == GFX_MODE_CGA)
        memset(text_mem + (GFXLine * 160), 0x00, GFXVerticalHeight * 160);
    else {
        // Enable writing to all planes
        outp(0x3C4, 0x02);
        outp(0x3C5, 0xFF);

        outpw(0x3CE, 0xFF08); // bit mask

        memset(graphics_mem, 0x00, GFXVerticalHeight * 2 * 80);

    }
}

void ClearScreen() {

    if (graphicsmode == GFX_MODE_CGA)
        memset(text_mem, 0x00, 16384);
    else {
        // Enable writing to all planes
        outp(0x3C4, 0x02);
        outp(0x3C5, 0xFF);

        outpw(0x3CE, 0xFF08); // bit mask

        memset(graphics_mem, 0x00, 16000);
    }
}

void DisplayText(char *text) { 
    ClearLine(0);
    ClearLine(1);
    ClearLine(2);
    DrawTextInInput(2, 0, 0x07, text);
}




// Sprites can be transparent and aren't RLE'd
// first byte is just colour
//second byte is
// bit0 - transparent first pixel
// bit1 - transparent second pixel
// bit2 - newline
void DecodeSprite(char *gfx, int length, int x, int y) {
    char *writepnt;
    unsigned int Numbytes;
    unsigned char writebyte = 0;
    char *tmp;

    if (graphicsmode == GFX_MODE_CGA) {

        writepnt = text_mem + (GFXLine * 160) + (y * 160) + (2 * x);

        length += 2;

        while (length -= 2) {
            writebyte = 0;

            if ((gfx[1] & 0x01)) {
                writebyte |= writepnt[1] & 0x0F;
            } else {
                writebyte |= gfx[0] & 0x0F;
            }
            if ((gfx[1] & 0x02)) {
                writebyte |= writepnt[1] & 0xF0;
            } else {
                writebyte |= gfx[0] & 0xF0;
            }

            if (y >= 0) {
                writepnt[0] = 0xDD;
                writepnt[1] = writebyte;
            }

            //newline
            if ((gfx[1] & 0x04)){
                y++;
                writepnt = text_mem + (GFXLine * 160) + (y * 160) + (2 * x);
            }
            else writepnt += 2;
            gfx += 2;
        }
    } else {

        writepnt = graphics_mem + (GFXLine * 160) + (y * 160) + x;

        length += 2;

        while (length -= 2) {
            writebyte = 0;

            // transparent 1st pixel
            if ((gfx[1] & 0x01)) {
                // do nothing
            } else {
                // blank it out first
                outpw(0x3CE, 0xF008); // bit mask

                outp(0x3C4, 0x02);  // enable planes
                outp(0x3C5, 0xFF);  // all planes

                *writepnt = 0x00;
                *(writepnt + 80) = 0x00;

                // set colours
                outp(0x3C4, 0x02);  // enable planes
                outp(0x3C5, gfx[0] & 0x0F);  // colors

                outpw(0x3CE, 0xF008); // bit mask
                *writepnt = 0xFF;
                *(writepnt + 80) = 0xFF;
            }

            // dummy read to fill latches
            tmp = *writepnt;

            // transparent 2nd pixel
            if ((gfx[1] & 0x02)) {
                // do nothing
            } else {
                // blank it out first
                outpw(0x3CE, 0x0F08); // bit mask

                outp(0x3C4, 0x02);  // enable planes
                outp(0x3C5, 0xFF);  // all planes

                *writepnt = 0x00;
                *(writepnt + 80) = 0x00;

                outp(0x3C4, 0x02);  // enable planes
                outp(0x3C5, (gfx[0] & 0xF0) >> 4);  // colors

                outpw(0x3CE, 0x0F08); // bit mask
                *writepnt = 0xFF;
                *(writepnt + 80) = 0xFF;
            }

            //newline
            if ((gfx[1] & 0x04)){
                y++;
                writepnt = graphics_mem + (GFXLine * 160) + (y * 160) + x;
            }
            else writepnt ++;
            gfx += 2;
        }
    }
}

/*void DrawEGA(unsigned char far *from, unsigned int lines) {

    unsigned char cnt;

    unsigned char far *vram = graphics_mem;
    unsigned char *lz4pnt;
    unsigned int x = 0, y = 0;

    unsigned char bmm;

    lz4pnt = from + 1;

    for (y = 0; y < lines; y++){
        for (x = 0; x < 80; x++) {

            cnt = 0;
            bmm = *lz4pnt;

            outp(0x3C4, 0x02);  // enable planes
            outp(0x3C5, bmm & 0x0F);  // colors

            outpw(0x3CE, 0xF008); // bit mask
            *vram = 0xFF;
            *(vram + 80) = 0xFF;

            // dummy read to fill latches
            cnt = *vram;

            outp(0x3C4, 0x02);  // enable planes
            outp(0x3C5, (bmm & 0xF0) >> 4);  // colors

            outpw(0x3CE, 0x0F08); // bit mask
            *vram = 0xFF;
            *(vram + 80) = 0xFF;

            vram += 1;
            lz4pnt += 2;
        }
        vram += 80;
    }

    //reset registers
    //Bit Mask = FFh
    outp(0x3CE, 0x08);
    outp(0x3CF, 0xFF);

    // All planes writable
    outp(0x3CE, 0x02);
    outp(0x3CF, 0x0F);
}*/

unsigned char *LZ4Buffer;

unsigned int DecodeSize;

unsigned char LZ4Magic[] = {0x02, 0x21, 0x4C, 0x18, 0x00};

char far *FindMagic(char far *s1, unsigned int length) {

    while (length--){
        if (
            s1[0] == LZ4Magic[0] && 
            s1[1] == LZ4Magic[1] && 
            s1[2] == LZ4Magic[2] && 
            s1[3] == LZ4Magic[3]
        ) return s1;
        
        else s1++;
    }

    return NULL;
    
}

unsigned char bum[30];

unsigned char lz4bum[16000];

void Decode(char far *gfx, unsigned int length) {

    int x;

    if (graphicsmode == GFX_MODE_CGA) {
        inb = gfx;
        outb = text_mem + (GFXLine * 160);
        lz4_decompress();
    } else {
        //Bit Mask = FFh
        outp(0x3CE, 0x08);
        outp(0x3CF, 0xFF);


        inb = gfx;
        outb = graphics_mem;

        // enable plane 0
        outp(0x3C4, 0x02);
        outp(0x3C5, 0x01);

        // read from plane 0
        outp(0x3CE, 0x04);
        outp(0x3CF, 0x00);

        lz4_decompress();

        
        gfx = FindMagic(gfx + 4, length);
        if (gfx == NULL) exit(1);

        inb = gfx;
        outb = graphics_mem;

        // enable plane 1
        outp(0x3C4, 0x02);
        outp(0x3C5, 0x02);

        // read from plane 1
        outp(0x3CE, 0x04);
        outp(0x3CF, 0x01);

        lz4_decompress();

                
        gfx = FindMagic(gfx + 4, length);
        if (gfx == NULL) exit(1);

        inb = gfx;
        outb = graphics_mem;

        // enable plane 2
        outp(0x3C4, 0x02);
        outp(0x3C5, 0x04);

        // read from plane 2
        outp(0x3CE, 0x04);
        outp(0x3CF, 0x02);

        lz4_decompress();


        gfx = FindMagic(gfx + 4, length);
        if (gfx == NULL) exit(1);

        inb = gfx;
        outb = graphics_mem;

        // enable plane 3
        outp(0x3C4, 0x02);
        outp(0x3C5, 0x08);

        // read from plane 3
        outp(0x3CE, 0x04);
        outp(0x3CF, 0x03);

        lz4_decompress();
    }
}

void DisplayGFX(int id) {
    ClearGFX();
    if (Graphics[id].Length != 0) {
        Decode(Graphics[id].Data, Graphics[id].Length);
    }
}

void GFX_DrawSprite(int id, int x, int y){
    DecodeSprite(Graphics[id].Data, Graphics[id].Length, x, y);
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
        unsigned int pos = (y) * 80 + x;

        outp(0x3D4, 0x0F);
        outp(0x3D5, (unsigned char) (pos & 0xFF));
        outp(0x3D4, 0x0E);
        outp(0x3D5, (unsigned char) ((pos >> 8) & 0xFF));
}


void get_cursor_pos(int *row, int *col)
{
    union REGS regs;

    regs.h.ah = 0x03;   // BIOS: Read cursor position
    regs.h.bh = 0x00;   // Page number (0 for active page)

    int86(0x10, &regs, &regs);

    *row = regs.h.dh;   // Row (0-based)
    *col = regs.h.dl;   // Column (0-based)
}

unsigned char *GFXData;

void LoadGFX() {
    FILE *infile;
    long filelen;
    unsigned char i;
    long offset;

    // load the big blob of graphics

    if (graphicsmode == GFX_MODE_CGA)
        infile = fopen("cga.poo", "rb");
    else    
        infile = fopen("ega.poo", "rb");
    
    if (!infile) {
        printf("Can't open .poo file\n");
        exit (1);
    }

    fseek(infile, 0, SEEK_END);
    filelen = ftell(infile);
    fseek(infile, 0, SEEK_SET);

    GFXData = malloc(filelen);
    fread(GFXData, filelen, 1, infile);
    fclose(infile);

    // now link up the pointers

    for (i=0; i < GFXCOUNT; i++) {
        // get the index to the file from the index table at the start of the data block
        offset = *((long *)(GFXData + (i * sizeof(long) * 2)));

        // skip the index table itself
        offset += sizeof(long) * GFXCOUNT;

        // set the pointer
        Graphics[i].Data = GFXData + offset;

        // check ok
        if (FindMagic(Graphics[i].Data, 1) == NULL && i != GFX_CROWN)
            printf("Invalid GFX %d", i);

        // length is the second byte in the index table
        offset += sizeof(long);
        Graphics[i].Length = GFXData + offset;
    }
}


void GFX_DrawScreenSplit() {
    if (graphicsmode == GFX_MODE_CGA){
        raster_split_nopoll();
    }
    else
    {
        raster_waitvsync();
    }
}

void GFX_Exit() {
    union REGS r;

    /* Restore normal text mode */
    r.h.ah = 0x00;
    r.h.al = 0x03;
    int86(0x10, &r, &r);
    
}

void GFX_Init() {
    char tat = 0;

    system("cls");
    printf("1. CGA\n2. EGA\n3. VGA\n");

    graphicsmode = getch();

    memset(Graphics, 0x00, sizeof(Graphics));

    LoadGFX();

    switch (graphicsmode) {
        case GFX_MODE_CGA:
            GFXLinesPerChar = 2;
            TextLinesPerChar = 8;
            TextAtTop = 1;
            set_mode_cga();
            break;

        case GFX_MODE_EGA:
            set_mode_ega();
            break;

        case GFX_MODE_VGA:
            set_mode_vga();
            break;

        default:
            printf("invalid selection %x\n", graphicsmode);
            exit(0);
    }

    GFXRegisterMode = GFXLinesPerChar - 1;

}
