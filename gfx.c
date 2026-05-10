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

char TextAtTop = 0;

void far *inb, *outb;

/* 160x100 CGA CRTC setup */
unsigned char cga160crtc[] = {
    113, /* R0  Horizontal total */
    80,  /* R1  Horizontal displayed */
    89,  /* R2  Horizontal sync position */
    15,  /* R3  Sync width */
    127, /* R4  Vertical total */
    6,   /* R5  Vertical total adjust */
    100, /* R6  Vertical displayed */
    112,  /* R7  Vertical sync position */
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


void DisableBlink(void) {
    union REGS r;
    r.h.ah = 0x10;
    r.h.al = 0x03;
    r.h.bl = 0x00;
    int86(0x10, &r, &r);
}

void raster_loop_frames(void);

void set_160x100_mode_cga(void)
{
    union REGS r;
    int i;

    rasterDisable();

    // BIOS mode 3 (80x25 text)
    r.h.ah = 0x00;
    r.h.al = 0x03;
    int86(0x10, &r, &r);

    DisableBlink();

    for (i = 0; i < sizeof(cga160crtc); i++)
    {
        outp(CGA_CRTC_INDEX, i);
        outp(CGA_CRTC_DATA, cga160crtc[i]);
    }
    
   rasterEnable();
   MSPerFrame = 17;
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

    DisableBlink();
    MSPerFrame = 17;
}

void set_160x100_mode_ega350(void)
{
    union REGS r;
    int i;

    /* BIOS mode 3 (80x25 text) */
    r.h.ah = 0x00;
    r.h.al = 0x03;
    int86(0x10, &r, &r);

    DisableBlink();
    MSPerFrame = 17;
}

void set_160x100_mode_vga(void)
{
    union REGS r;

    /* BIOS mode 3 (80x25 text) */
    r.h.ah = 0x00;
    r.h.al = 0x03;
    int86(0x10, &r, &r);

    DisableBlink();
    MSPerFrame = 14;
}

void raster_loop_250_frames(void);

void raster_split_nopoll(void);

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

// Char line to draw GFX at, always 0 when text at bottom
unsigned char GFXLine;

void RecalcScreenGeometry() {

    GFXVerticalLines = GFXVerticalHeight * GFXLinesPerChar;
    TextVerticalLines = TextVerticalHeight * TextLinesPerChar;

    if (CurrState->ID == STATE_MENU || CurrState->ID == STATE_AWARDS || CurrState->ID == STATE_AWARDS2) {
        GFXLine = 0;
        AboveSplitMode = GFXRegisterMode;
        SplitAtLine = GFXVerticalHeight * GFXLinesPerChar;
        BelowSplitMode = TextLinesPerChar - 1;
        TextLine = 32;
    } else if (TextAtTop) {
        GFXLine = TextVerticalHeight;
        AboveSplitMode = TextLinesPerChar - 1;
        SplitAtLine = TextVerticalLines;
        BelowSplitMode = GFXRegisterMode;
        TextLine = 0;
    } else {
        GFXLine = 0;
        AboveSplitMode = GFXRegisterMode;
        SplitAtLine = GFXVerticalHeight * GFXLinesPerChar;
        BelowSplitMode = TextLinesPerChar - 1;
        TextLine = GFXVerticalHeight;
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
        ClearLine(TextLine + 2);
        DrawTextColor(2, TextLine + 2, 0x07, ">");
    }
}

void SetTextWindowLine(int line) {
    TextLine = line;
}

volatile unsigned char keybuf[KEYBUF_SIZE];
volatile unsigned int  keybuf_head = 0;
volatile unsigned char last_keybyte = 0;

void raster_loop_frames(void);

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

void ClearLine(int line) {
    DrawTextColor(0, line, 0x0F, "                                                                                ");
}

void ClearScreen() {
    memset(text_mem, 0x00, 16384);
}

void DisplayText(char *text) {
    ClearLine(TextLine);
    ClearLine(TextLine+1);
    DrawTextColor(2, TextLine, 0x07, text);
}


void Decode(char far *gfx) {
    inb = gfx;
    outb = text_mem + (GFXLine * 160);
    lz4_decompress();
}
// Sprites can be transparent and aren't RLE'd
// first byte is just colour
//second byte is
// bit0 - transparent first pixel
// bit1 - transparent second pixel
// bit2 - newline
void DecodeSprite(char *gfx, int length, int x, int y) {
    char *writepnt = text_mem + (GFXLine * 160) + (y * 160) + (2 * x);
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

void ClearGFX() {
    memset(text_mem + (GFXLine * 160), 0x00, GFXVerticalHeight * 160);
}

void DisplayGFX(int id){ 
    ClearGFX();
    if (Graphics[id].Length != 0) {
        Decode(Graphics[id].Data);
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
        unsigned int pos = y * 80 + x;

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

void LoadGFX(int num, char * filename) {
    FILE *infile;
    infile = fopen(filename, "rb");
    if (!infile){
        printf("Can't open %s\n", filename);
        exit (1);
    }


    fseek(infile, 0, SEEK_END);
    Graphics[num].Length = ftell(infile);
    fseek(infile, 0, SEEK_SET);

    Graphics[num].Data = malloc(Graphics[num].Length + 1);
    fread(Graphics[num].Data, Graphics[num].Length, 1, infile);
    fclose(infile);
}


void GFX_DrawScreenSplit() {
    if (GFXVerticalHeight) {
        raster_split_nopoll();
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
    char tat = 0;

    printf("1 Text Above (slow machines), 2 Text Below (fast machines)\n");

    tat = getch();

    switch (tat){
        case 0x31:
            TextAtTop = 1;
            break;

        case 0x32:
            TextAtTop = 0;
            break;

        default:
            printf("invalid selection %x\n", tat);
            exit(0);
    }

    printf("1 CGA, 2 EGA200, 3 EGA350, 4 VGA\n");

    graphicsmode = getch();

    switch (graphicsmode){
        case 0x31:
            GFXLinesPerChar = 2;
            TextLinesPerChar = 8;
            set_160x100_mode_cga();
            break;

        case 0x32:
            GFXLinesPerChar = 2;
            TextLinesPerChar = 8;
            set_160x100_mode_ega200();
            break;

        case 0x33:
            GFXLinesPerChar = 3;
            TextLinesPerChar = 14;
            set_160x100_mode_ega350();
            break;

        case 0x34:
            GFXLinesPerChar = 4;
            TextLinesPerChar = 16;
            set_160x100_mode_vga();
            break;

        default:
            printf("invalid selection %x\n", graphicsmode);
            exit(0);
    }

    GFXRegisterMode = GFXLinesPerChar - 1;

    memset(Graphics, 0x00, sizeof(Graphics));

    LoadGFX(GFX_MENU, "1.lz4");
    LoadGFX(GFX_STANDING, "2.lz4");
    LoadGFX(GFX_STANDINGPANTSOFF, "3.lz4");
    LoadGFX(GFX_DOOROPEN, "4.lz4");
    LoadGFX(GFX_DOOROPENPANTSOFF, "5.lz4");
    LoadGFX(GFX_ONTOILET, "6.lz4");
    LoadGFX(GFX_ONTOILETPANTSOFF, "7.lz4");
    LoadGFX(GFX_AWARDS, "8.lz4");
    LoadGFX(GFX_SHITONFLOOR, "10.lz4");
    LoadGFX(GFX_SHITINTOILET, "11.lz4");
    LoadGFX(GFX_SHITPANTSSTANDING, "12.lz4");
    LoadGFX(GFX_SHITINPANTSSITTING, "13.lz4");
    LoadGFX(GFX_DIEPANTSON, "18.lz4");
    LoadGFX(GFX_DIEPANTSOFF, "19.lz4");
    LoadGFX(GFX_PILLSSTANDINGPANTSON1, "22.lz4");
    LoadGFX(GFX_PILLSSTANDINGPANTSON2, "23.lz4");
    LoadGFX(GFX_PILLSSTANDINGPANTSOFF1, "26.lz4");
    LoadGFX(GFX_PILLSSTANDINGPANTSOFF2, "27.lz4");
    LoadGFX(GFX_PILLSSITTINGPANTSON1, "29.lz4");
    LoadGFX(GFX_PILLSSITTINGPANTSON2, "30.lz4");
    LoadGFX(GFX_PILLSSITTINGPANTSOFF2, "33.lz4");
    LoadGFX(GFX_SHITINPANTSWHILEOFF, "39.lz4");
    LoadGFX(GFX_DIEPANTSONSITTING, "42.lz4");
    LoadGFX(GFX_DIEPANTSOFFSITTING, "43.lz4");
    LoadGFX(GFX_SHITONBATHROOMFLOOR, "45.lz4");
    LoadGFX(GFX_ELVIS, "46.lz4");
    LoadGFX(GFX_UNK1, "48.lz4");
    LoadGFX(GFX_UNK2, "unk.lz4");
    LoadGFX(GFX_END, "50.lz4");
    LoadGFX(GFX_CROWN, "crown.bin");
}
