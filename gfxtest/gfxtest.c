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

unsigned int SplitAtLine = 40;
unsigned char Vsync = 45;
unsigned char Vtotal = 46;
unsigned char Vadj = 6;
unsigned char Vdisp = 40;



// 160x100 CGA CRTC setup
unsigned char cga160crtc[] = {
    113, /* R0  Horizontal total */
    80,  /* R1  Horizontal displayed */
    90,  /* R2  Horizontal sync position */
    10,  /* R3  Sync width */
    46, /* R4  Vertical total */
    6,   /* R5  Vertical total adjust */
    40, /* R6  Vertical displayed */
    43,  /* R7  Vertical sync position */
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

void raster_split_nopoll(void);

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

int main(void)
{
    union REGS r;
    FILE *infile;
    char inkey;
    char buf[80];
    
    set_160x100_mode_cga();

    infile = fopen("demo.bin", "rb");
    if (!infile)
    {
        printf("Failed to open input file\n");
        return 1;
    }

    fread(text_mem, 6400, 1, infile);
    fclose(infile);

    sprintf(buf, "  Q/A      W/S      E/D      R/F      T/G", SplitAtLine, Vsync, Vtotal, Vadj, Vdisp );
    DrawTextColor(0, 22, 0x0f, buf);

    while (1) {

        raster_split_nopoll();

        for (inkey = 0; inkey < 254; inkey++);
        
        if (kbhit()) {
            inkey = getch();
            if (inkey != 0) {
                switch (inkey) {
                    case 'q':SplitAtLine++;break;
                    case 'a':SplitAtLine--;break;
                    case 'w':Vsync++;break;
                    case 's':Vsync--;break;
                    case 'e':Vtotal++;break;
                    case 'd':Vtotal--;break;
                    case 'r':Vadj++;break;
                    case 'f':Vadj--;break;
                    case 't':Vdisp++;break;
                    case 'g':Vdisp--;break;
                    case 27:goto bum;break;
                }

                outp(CGA_CRTC_INDEX, 7);
                outp(CGA_CRTC_DATA, Vsync);

                outp(CGA_CRTC_INDEX, 4);
                outp(CGA_CRTC_DATA, Vtotal);

                outp(CGA_CRTC_INDEX, 5);
                outp(CGA_CRTC_DATA, Vadj);

                outp(CGA_CRTC_INDEX, 6);
                outp(CGA_CRTC_DATA, Vdisp);

                sprintf(buf, "split:%u vsync:%u vtotal:%u vadj:%u vdisp:%u \n", SplitAtLine, Vsync, Vtotal, Vadj, Vdisp );
                DrawTextColor(0, 23, 0x0f, buf);
            }
        }

    }
bum:
    // Restore normal text mode
    r.h.ah = 0x00;
    r.h.al = 0x03;
    int86(0x10, &r, &r);

    return 0;
}