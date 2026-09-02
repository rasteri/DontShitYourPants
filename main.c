#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <graph.h>
#include <string.h>
#include <stdlib.h>

#include "gamelogic.h"

int SecondCount = 0;

extern GameState *CurrState;

void Frontend_Exit()
{
    union REGS r;

    /* Restore normal text mode */
    r.h.ah = 0x00;
    r.h.al = 0x03;
    int86(0x10, &r, &r);

    exit(0);
}

char InputBuff[100];
char TimeBuf[30];
char OutputBuff[100];


static void (__interrupt __far *old_int23)(void);

void __interrupt __far my_int23(void) {

}

void reboot(void) {
    unsigned short far *bootflag;

    // try using the keyboard controller (AT onwards)
    outp(0x64, 0xFE);

    // failing that...

    // warm boot flag
    bootflag = MK_FP(0x40, 0x72);
    *bootflag = 0x1234;

    // jump to reset vector
    _asm {
        mov ax, 0FFFFh
        push ax

        xor ax, ax
        push ax

        retf
    }

}
unsigned char tmp[1107];

extern void far *inb, *outb;

unsigned char far *vram = (unsigned char far *)0xA0000000L;
unsigned char *lz4pnt;

int main(void)
{
    FILE *f;
    char inkey;

    int bufpos = 0;

    unsigned int i = 0;
    unsigned int exitframe = 200;
    unsigned int curpos = 0;
    unsigned int x = 0, y = 0;
    unsigned char substate = 0;
    unsigned char subsubstate = 0;
    char *pt;
    unsigned char cnt,cnt2;
    unsigned char bmm = 0;
    FILE *bum;
    unsigned char deleteprogress = 0;
    union REGS r;

    memset(InputBuff, 0x00, 100);

    PlaySound(JukeBox[SOUND_INTRO]);

    Gamelogic_Init();

    GFX_Init();

    EnterState();

    while (1) {

        // do something altogether different
        if (CurrState->ID == STATE_UNK2) {

            GFX_Exit();

            old_int23 = _dos_getvect(0x23);
            _dos_setvect(0x23, my_int23);



            rasterDisable();
            DisableBlink();
            rasterEnable();
            GFXLine = 0;

            if (graphicsmode = GFX_MODE_EGA) {
                graphicsmode = GFX_MODE_CGA;
                DisplayGFX(GFX_UNK2);

            } else {
                DisplayGFX(GFX_UNK2);
                SetGFXLines(0);
            }
            
            DrawTextInWindow(0, 0, 0x07, "ERROR : Causality Violation");
            DrawTextInWindow(0, 1, 0x07, "                                                     ");
            y = 2;
            CurrState->ID = STATE_UNK3;
        }
        else if (CurrState->ID == STATE_UNK3) {

            getcwd(InputBuff, 100);
            sprintf(OutputBuff, "%s>", InputBuff);
            DrawTextInWindow(0, y, 0x07, OutputBuff);

            x = strlen(OutputBuff);
            update_cursor(x, y);


            while(1) {
                inkey = getch();
                // enter
                if (inkey == '\r' || inkey == '\n')
                {
                    y++;
                    switch (substate) {
                        case 0:
                        case 2:
                            DrawTextInWindow(0, y, 0x07, "Bad command or file name");
                            break;

                        case 1:
                            DrawTextInWindow(0, y, 0x07, "Bad command or filename");
                            break;

                        case 3:
                            DrawTextInWindow(32, 12, 0x40, "  ");
                            DrawTextInWindow(32, 13, 0x40, "  ");
                            DrawTextInWindow(32, 14, 0x40, "  ");
                            DrawTextInWindow(32, 15, 0x40, "  ");
                            DrawTextInWindow(36, 16, 0x40, "  ");
                            DrawTextInWindow(0, y, 0x04, "YOU CANNOT ENTER");
                            break;

                        case 4:
                            reboot();
                            break;
                    }
                    y += 2;
                    substate++;
                    break;
                }
                else if (inkey != 0)
                {
                    DrawChar(x, y, inkey);
                    x++;

                    //speed up in future
                    if (subsubstate && (y >= 4)){
                        DrawChar(x, y, inkey);
                        x++;
                    }

                    if (x >= 80) {
                        x = 0; 
                        y++;
                    }
                    update_cursor(x, y);

                    //always column 40, starting at line 4
                    if (x == 40) {
                        if (y == 4 && subsubstate == 0) {
                            DrawTextInWindow(43, 19, 0x07, "?????????");
                            subsubstate++;
                        } else if (y == 6 && subsubstate == 1) {
                            DrawTextInWindow(43, 19, 0x07, "What are you doing?");
                            subsubstate++;
                        } else if (y == 15 && subsubstate == 2) {
                            DrawTextInWindow(43, 19, 0x04, "YOU CANT DESTROY ME!!!!!");
                            subsubstate++;
                        } else if (y == 22 && subsubstate == 3) {
                            Awards |= AWARD_UNK;
                            EndingLog |= ENDING_UNK;
                            SaveAwards();
                            reboot();
                        }
                    }

                }
            }
        }
        else {

            GFX_DrawScreenSplit();
            if (CurrState->ID == STATE_UNK1){
                if (graphicsmode == GFX_MODE_CGA) {
                    pt = text_mem + 68;
                    cnt = 100;
                    while (cnt--){
                        *pt = RandomTable[bmm++];
                        *(pt+1) = 0x07;
                        pt += 160;
                    }
                } else {
                    pt = graphics_mem + 34;
                    cnt = 200;
                    while (cnt--){
                        *pt = RandomTable[bmm++];
                        pt += 80;
                    }
                }
            } else if (CurrState->ID == STATE_AWARDS2 && (endingcount >= NUMENDINGS - 1)) {
                pt = text_mem + (32 * 160) + (30 * 2);
                cnt = 2;
                if (graphicsmode == GFX_MODE_CGA) {
                    while (cnt--){
                        *pt = RandomTable[bmm++];
                        *(pt+1) = 0x0F;
                        pt += 2;
                    }
                } else {
                    // Enable writing to all planes
                    outp(0x3C4, 0x02);
                    outp(0x3C5, 0xFF);

                    //Bit Mask = FFh
                    outp(0x3CE, 0x08);
                    outp(0x3CF, 0xFF);

                    pt = graphics_mem + (20 * 8 * 80) + 30;
                    cnt2 = 8;
                    while (cnt2--) {
                        cnt = 2;
                        while (cnt--) {
                            *pt = RandomTable[bmm++];
                            pt += 1;
                        }
                        pt += 78;
                    }
                }
            }

            if (CurrState->ID <= STATE_ONTOILETPANTSOFF && CurrState->ID >= STATE_STANDING)
            SecondCount++;
            if (SecondCount == FramesPerSecond)
            {
                SecondCount = 0;
                Gamelogic_SecondTick();
                if (Countdown > 0) {
                    sprintf(TimeBuf, "%02d:%02d", Countdown / 60, Countdown % 60);
                    DrawText(70, InputLine + 2, 0x07, TimeBuf);
                }
            }

            while (kbhit())
            {
                inkey = getch();

                // enter
                if (inkey == '\r' || inkey == '\n')
                {
                    CGA_Unsplit();
                    GameLogic_TextInput(InputBuff);
                    bufpos = 0;
                    InputBuff[bufpos] = 0;
                    InputBuff[bufpos + 1] = 0;
                    if (CurrState->ID <= STATE_ONTOILETPANTSOFF){
                        ClearLine(2);
                        DrawTextInInput(2, 2, 0x07, ">");
                    }
                    CGA_Resplit();

                }
                else if (CurrState->ID <= STATE_ONTOILETPANTSOFF) // only display text line on some states
                {
                    // delete
                    if (inkey == '\b')
                    {
                        if (bufpos)
                        {
                            bufpos--;
                            DrawChar(4 + bufpos, InputLine + 2, ' ');
                            InputBuff[bufpos] = 0;
                        }
                    }

                    else if (inkey != 0)
                    {
                        DrawChar(4 + bufpos, InputLine + 2, inkey);
                        InputBuff[bufpos] = inkey;
                        InputBuff[bufpos + 1] = 0;
                        bufpos++;
                    }
                }
                update_cursor(strlen(InputBuff) + 4, InputLine + 2);
            }

            Music_Task();
        }
    }

    GFX_Exit();

    return 0;
}