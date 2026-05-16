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

unsigned char lz4test[1000];

extern void far *inb, *outb;

int main(void)
{
    char inkey;

    int bufpos = 0;

    unsigned int i = 0;
    unsigned int exitframe = 200;
    unsigned int curpos = 0;
    int x = 0, y = 0;
    unsigned char substate = 0;
    unsigned char subsubstate = 0;
    char *pt;
    unsigned char cnt;
    unsigned char bmm = 0;


    FILE *bum;
    unsigned char deleteprogress = 0;


    memset(InputBuff, 0x00, 100);

    PlaySound(JukeBox[SOUND_INTRO]);

    Gamelogic_Init();

    GFX_Init();

    EnterState();



    while (1) {



        // do something altogether different
        if (CurrState->ID == STATE_UNK2) {
            //ClearScreen();
            //GFX_DrawSprite(GFX_UNK2, 25, 3);
            GFX_Exit();
            DisableBlink();
            GFXLine = 0;
            DisplayGFX(GFX_UNK1);
            DrawTextColor(0, 0, 0x07, "ERROR : Causality Violation");
            DrawTextColor(0, 1, 0x07, "                                                     ");
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
                        case 2:
                            DrawTextColor(0, y, 0x07, "Bad command or file name");
                            break;

                        case 1:
                            DrawTextColor(0, y, 0x07, "Bad command or filename");
                            break;

                        case 3:
                            DrawTextColor(32, 12, 0x40, "  ");
                            DrawTextColor(32, 13, 0x40, "  ");
                            DrawTextColor(32, 14, 0x40, "  ");
                            DrawTextColor(32, 15, 0x40, "  ");
                            DrawTextColor(36, 16, 0x40, "  ");
                            DrawTextColor(0, y, 0x04, "YOU CANNOT ENTER");
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
                    if (y >= 4){
                        DrawChar(x, y, inkey);
                        x++;
                    }

                    if (x >= 80) {
                        x = 0; 
                        y++;
                    }
                    update_cursor(x, y);
                    sprintf(OutputBuff, "%d,%d", x, y);
                    DrawTextColor(0, 24, 0x07, OutputBuff);

                    //always column 40, starting at line 4
                    if (x == 40) {
                        if (y == 4 && subsubstate == 0) {
                            DrawTextColor(43, 19, 0x07, "?????????");
                            subsubstate++;
                        } else if (y == 6 && subsubstate == 1) {
                            DrawTextColor(43, 19, 0x07, "What are you doing?");
                            subsubstate++;
                        } else if (y == 9 && subsubstate == 2) {
                            DrawTextColor(43, 19, 0x04, "WHAT ARE YOU DOING?!");
                            subsubstate++;
                        } else if (y == 12 && subsubstate == 3) {
                            DrawTextColor(43, 19, 0x04, "STOP!!!!!!!!!!!!!!!!!!!!");
                            subsubstate++;
                        } else if (y == 15 && subsubstate == 4) {
                            DrawTextColor(43, 19, 0x04, "YOU CANNOT DESTROY ME!!!!!");
                            subsubstate++;
                        } else if (y == 18 && subsubstate == 5) {
                            DrawTextColor(43, 19, 0x04, "IF I DIE, IT ALL ENDS     ");
                            subsubstate++;
                        } else if (y == 21 && subsubstate == 6) {
                            DrawTextColor(43, 22, 0x07, "..........................");
                            subsubstate++;
                        } else if (y == 22 && subsubstate == 7) {
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

                pt = text_mem + 68;
                cnt = 100;
                while (cnt--){
                    *pt = bmm++;
                    *(pt+1) = 0x07;
                    pt += 160;
                }
            } else if (CurrState->ID == STATE_AWARDS2 && (endingcount >= NUMENDINGS - 1)) {
                pt = text_mem + (32 * 160) + (30 * 2);
                cnt = 2;
                while (cnt--){
                    *pt = bmm += 29;
                    *(pt+1) = 0x0F;
                    pt += 2;
                }
                if (!(Awards & AWARD_UNK)) {

                    /*pt = text_mem + (32 * 160) + 24;
                    cnt = 19;
                    while (cnt--){
                        *pt = bmm++;
                        //*(pt+1) = 0x0F;
                        pt += 2;
                    }
                    pt = text_mem + (33 * 160) + 6;
                    cnt = 25;
                    while (cnt--){
                        *pt = bmm++;
                        //*(pt+1) = 0x0e;
                        pt += 2;
                    }
                    */
                }
            }

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
                    DrawTextColor(3, TextLine + 2, 0x0F, "                                                                           ");

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
    }

    GFX_Exit();

    return 0;
}