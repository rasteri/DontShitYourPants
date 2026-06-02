#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <graph.h>
#include <string.h>
#include <stdlib.h>

#include "gamelogic.h"

/* CGA text memory */
unsigned char far *text_mem = MK_FP(0xB000, 0x8000);

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

int main(void)
{
    char inkey;

    int bufpos = 0;

    FILE *bum;

    memset(InputBuff, 0x00, 100);

    PlaySound(JukeBox[SOUND_INTRO]);

    Gamelogic_Init();

    GFX_Init();

    EnterState();

    while (1)
       {
            GFX_DrawScreenSplit();

            if (CurrState->ID <= STATE_ONTOILETPANTSOFF && CurrState->ID >= STATE_STANDING)
            SecondCount++;
            if (SecondCount == 60)
            {
                SecondCount = 0;
                Gamelogic_SecondTick();
                sprintf(TimeBuf, "%02d:%02d", Countdown / 60, Countdown % 60);
                DrawTextColor(70, TextLine + 2, 0x07, TimeBuf);
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
                    if (CurrState->ID <= STATE_ONTOILETPANTSOFF)
                        DrawTextColor(3, TextLine + 2, 0x0F, "                                                                           ");
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
                            DrawChar(4 + bufpos, TextLine + 2, ' ');
                            InputBuff[bufpos] = 0;
                        }
                    }

                    else if (inkey != 0)
                    {
                        DrawChar(4 + bufpos, TextLine + 2, inkey);
                        InputBuff[bufpos] = inkey;
                        InputBuff[bufpos + 1] = 0;
                        bufpos++;
                    }
                }
                update_cursor(strlen(InputBuff) + 4, TextLine + 2);
            }

            keybuf_head = 0;
            Music_Task();
        }

    GFX_Exit();

    return 0;
}