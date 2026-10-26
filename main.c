#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <graph.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

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


void * xmalloc(size_t size)
{
    void *p = malloc(size);


    if (p == NULL) {
        fprintf(stderr, "Malloc shit its pants\n");
        exit(1);
    }
    return p;
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

int main(int argc, char *argv[])
{
    char inkey;
    int bufpos = 0;

    memset(InputBuff, 0x00, 100);

    PlaySound(JukeBox[SOUND_INTRO]);

    if (argc == 2) 
        graphicsmode = argv[1][0];

    GFX_Init();

    Gamelogic_Init();

    EnterState();

    while (1)
       {
            GFX_DrawScreenSplit();

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
