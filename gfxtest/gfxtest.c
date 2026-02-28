#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <graph.h>
#include <string.h>
#include <stdlib.h>

/* CGA ports */
#define CGA_CRTC_INDEX  0x3D4
#define CGA_CRTC_DATA   0x3D5
#define CGA_STATUS      0x3DA

#define CGA_MODE_CTRL   0x3D8

#define KEYBUF_SIZE 32
extern volatile unsigned char keybuf[KEYBUF_SIZE];
extern volatile unsigned int  keybuf_head;
extern volatile unsigned char last_keybyte;

char InputBuff[100];
char TimeBuf[30];

#define rasterDisable() outp(CGA_MODE_CTRL, 0x01)
#define rasterEnable()  outp(CGA_MODE_CTRL, 0x09)

char kbd_US [128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', /* <-- Tab */
  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, /* <-- control key */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',  0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */

};

static void interrupt keyb_int()
{
    /*unsigned char sixone;
    unsigned char scancode;
    unsigned char next_head;

    scancode = inp(0x60);     // read scancode

    // ignore break codes
    if (!(scancode & 0x80)) {
        next_head = (kb_head + 1) % KB_BUF_SIZE;

        //only store if buffer not full
        if (next_head != kb_tail) {
            kb_buf[kb_head] = scancode;
            kb_head = next_head;
        }
    }

    // for XT, need to strobe bit7 of 0x61 to acknowledge
    sixone = inp(0x61);
    sixone |= 0x80;
    outp(0x61, sixone);
    sixone &= ~0x80;
    outp(0x61, sixone);*/

    outp(0x20, 0x20);         // EOI to PIC

}

void interrupt (*old_keyb_int)();

void hook_keyb_int(void)
{
    old_keyb_int = _dos_getvect(0x09);
    _dos_setvect(0x09, keyb_int);
}

void unhook_keyb_int(void)
{
    if (old_keyb_int != NULL)
    {
        _dos_setvect(0x09, old_keyb_int);
        old_keyb_int = NULL;
    }
}

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


    // only need to set 4 scanline mode
    outp(CGA_CRTC_INDEX, 9);
    outp(CGA_CRTC_DATA, 3);
}

void raster_loop_250_frames(void);

unsigned int GFXLinesPerChar = 2;
unsigned int SplitAtLine = 344;
unsigned int GFXVerticalHeight = 86;
unsigned int TextLine = 86;

unsigned char GfxMaxScanLine = 1;
unsigned char TextMaxScanLine = 9;

void SetGFXLines(int rows) {
    GFXVerticalHeight = rows;
    SplitAtLine = rows * GFXLinesPerChar;
    TextLine = GFXVerticalHeight;
}

void SetTextLine(int line){
    TextLine = line;
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
    "mov al,GfxMaxScanLine" \
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
    "mov al,TextMaxScanLine" \
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

    ClearLine(TextLine);
    ClearLine(TextLine+1);
    DrawTextColor(2, TextLine, 0x07, text);

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


void GFX_DrawScreenSplit() {
    // draw first x lines in 160x100 mode
    if (GFXVerticalHeight) {
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
    printf("1 CGA, 2 EGA200, 3 EGA350, 4 VGA\n");

    graphicsmode = getch();

    switch (graphicsmode){
        case 0x31:
            GFXLinesPerChar = 2;
            GfxMaxScanLine = 1;
            TextMaxScanLine = 7;
            set_160x100_mode_cga();
            break;

        case 0x32:
            GFXLinesPerChar = 2;
            GfxMaxScanLine = 1;
            TextMaxScanLine = 7;
            set_160x100_mode_ega200();
            break;

        case 0x33:
            GFXLinesPerChar = 3;
            GfxMaxScanLine = 2;
            TextMaxScanLine = 15;
            set_160x100_mode_ega350();
            break;

        case 0x34:
            GFXLinesPerChar = 4;
            GfxMaxScanLine = 3;
            TextMaxScanLine = 15;
            set_160x100_mode_vga();
            break;

        default:
            printf("invalid selection %x\n", graphicsmode);
            exit(0);
    }

}

int main(void)
{
    char inkey;
    FILE *infile;
    int bufpos = 0;

    unsigned int i = 0;
    unsigned int exitframe = 0;

    memset(InputBuff, 0x00, 100);
    
    GFX_Init();
    SetGFXLines(86);
    hook_keyb_int();


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

        /*SecondCount++;
        if (SecondCount == 60){
            SecondCount = 0;
            Gamelogic_SecondTick();
            sprintf(TimeBuf, "%02d:%02d", Countdown / 60, Countdown % 60);
            //sprintf(TimeBuf, "%d-%d",bufpos, keybuf_head);
            DrawTextColor(70, textline + 2, 0x07, TimeBuf);
        }*/

        /*sprintf(TimeBuf, "%d-%d- %02X,%02X  ",bufpos, keybuf_head, keybuf[0], keybuf[1]);
        DrawTextColor(60, textline + 2, 0x07, TimeBuf);*/

        for(i=0;i<keybuf_head;i++)
        {
            // no breaks
            if (keybuf[i] & 0x80)
                continue;

            if (keybuf[i] == 0)
                continue;                

            inkey = kbd_US[keybuf[i]];

            //delete
            if (inkey == '\b'){
                if (bufpos) {
                    bufpos--;
                    DrawChar(4 + bufpos, TextLine + 2, ' ');
                    InputBuff[bufpos] = 0; 
                }
            }
            //enter
            else if (inkey == '\n'){
                goto finn;
                bufpos = 0;
                InputBuff[bufpos] = 0;
                InputBuff[bufpos+1] = 0;
                ClearLine(TextLine + 2);
            } 
            else if (inkey != 0) {
                DrawChar(4 + bufpos, TextLine + 2, inkey);
                InputBuff[bufpos] = inkey;
                InputBuff[bufpos+1] = 0;
                bufpos++;
            }


        }

        DrawTextColor(2, TextLine + 2, 0x07, ">");
        update_cursor(strlen(InputBuff) + 4, TextLine + 2);

        keybuf_head = 0;
    }
finn:
    unhook_keyb_int();
    GFX_Exit();

    return 0;
}