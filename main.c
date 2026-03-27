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



int main(void)
{
    char inkey;

    int bufpos = 0;

    unsigned int i = 0;
    unsigned int exitframe = 200;
    unsigned int curpos = 0;
    int x = 0, y = 0;
    unsigned char substate = 0;

    memset(InputBuff, 0x00, 100);

    PlaySound(JukeBox[SOUND_INTRO]);

    Gamelogic_Init();

    GFX_Init();
    EnterState();

    while (1)
    {

        // do something altogether different
        if (CurrState->ID == STATE_UNK2) {
            GFX_Exit();
            GFX_DrawSprite(GFX_UNK2, 25, 3);

            DrawTextColor(0, 0, 0x07, "ERROR : Causality Violation");
            y = 2;
            CurrState->ID = STATE_UNK3;

        }
        else if (CurrState->ID == STATE_UNK3) {

            getcwd(InputBuff, 100);
            sprintf(OutputBuff, "%s>", InputBuff);
            DrawTextColor(0, y, 0x07, OutputBuff);

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
                            DrawTextColor(0, y, 0x07, "Bad command or file name");
                            break;

                        case 1:
                            DrawTextColor(0, y, 0x07, "Bad command or filename");
                            break;

                        case 2:
                            DrawTextColor(0, y, 0x07, "What have you done to me?");
                            break;

                        case 3:
                            DrawTextColor(29, 11, 0x40, "  ");
                            DrawTextColor(29, 12, 0x40, "  ");
                            DrawTextColor(29, 13, 0x40, "  ");
                            DrawTextColor(29, 14, 0x40, "  ");
                            DrawTextColor(33, 15, 0x40, "  ");                            
                            DrawTextColor(0, y, 0x04, "WHAT HAVE YOU DONE TO ME?!");
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
                    if (x >= 80) {
                        x = 0; 
                        y++;
                    }
                    update_cursor(x, y);
                }
            }
        }
        else {

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
                    GameLogic_TextInput(InputBuff);
                    bufpos = 0;
                    InputBuff[bufpos] = 0;
                    InputBuff[bufpos + 1] = 0;
                    ClearLine(TextLine + 2);
                }
                else if (CurrState->ID <= STATE_CREDITS) // only display text line on some states
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
            }

            DrawTextColor(2, TextLine + 2, 0x07, ">");
            update_cursor(strlen(InputBuff) + 4, TextLine + 2);

            keybuf_head = 0;
            Music_Task();
        }
    }

    GFX_Exit();

    return 0;
}