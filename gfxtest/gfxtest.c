#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <graph.h>
#include <string.h>
#include <stdlib.h>

#define rasterDisable() outp(CGA_MODE_CTRL, 0x01)
#define rasterEnable()  outp(CGA_MODE_CTRL, 0x09)

// CGA ports
#define CGA_CRTC_INDEX  0x3D4
#define CGA_CRTC_DATA   0x3D5
#define CGA_STATUS      0x3DA

#define CGA_MODE_CTRL   0x3D8

// CGA text memory
unsigned char far *text_mem = MK_FP(0xB000, 0x8000);

int graphicsmode = 0;

char TextAtTop = 0;

// 160x100 CGA CRTC setup
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

void set_160x100_mode_cga(void)
{
    union REGS r;
    int i;

    rasterDisable();

    // BIOS mode 3 (80x25 text)
    r.h.ah = 0x00;
    r.h.al = 0x03;
    int86(0x10, &r, &r);

    // disable blink 
    r.h.ah = 0x10;
    r.h.al = 0x03;
    r.h.bl = 0x00;
    int86(0x10, &r, &r);

    for (i = 0; i < sizeof(cga160crtc); i++)
    {
        outp(CGA_CRTC_INDEX, i);
        outp(CGA_CRTC_DATA, cga160crtc[i]);
    }
    
   rasterEnable();
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

}

void set_160x100_mode_ega350(void)
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
}

void raster_split_nopoll(void);

// How many lines one char (i.e. one vertical "pixel" in 160x100 mode) take up
// 2 on CGA/EGA200, 3 on EGA350, 4 on VGA
unsigned int GFXLinesPerChar = 2;

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

// Guess what this does
char HideTextInput = 0;

void RecalcScreenGeometry() {

    GFXVerticalLines = GFXVerticalHeight * GFXLinesPerChar;
    TextVerticalLines = TextVerticalHeight * TextLinesPerChar;

     if (TextAtTop) {
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

void SetTextLines(int lines) {
    TextVerticalHeight = lines;
    RecalcScreenGeometry();
}

void SetTextWindowLine(int line) {
    TextLine = line;
}

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
            printf("invalid textat selection %x\n", tat);
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
            printf("invalid gfx mode selection %x\n", graphicsmode);
            exit(0);
    }

    GFXRegisterMode = GFXLinesPerChar - 1;

}


int main(void)
{
    char inkey;
    FILE *infile;
    int bufpos = 0;
    char inputbuff[256];

    unsigned int i = 0;
    unsigned int exitframe = 0;
    
    GFX_Init();
    SetGFXLines(86);

    memset (inputbuff, 0x00, sizeof(inputbuff));

    infile = fopen("fun.dat", "rb");
    if (!infile)
    {
        printf("Failed to open input file\n");
        return 1;
    }

    fread(text_mem, 0x3E80, 1, infile);
    fclose(infile);

    while (1) {

        GFX_DrawScreenSplit();

        if (kbhit()) {
            inputbuff[bufpos] = getch();

            if (inputbuff[bufpos] == '\n' || inputbuff[bufpos] == '\r')
                break;

            bufpos++;
            DrawTextColor(0, TextLine, 0x0F, inputbuff);
        }

    }
finn:

    GFX_Exit();

    return 0;
}