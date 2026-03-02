#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <graph.h>
#include <string.h>
#include <stdlib.h>

#include "gamelogic.h"


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

int SecondCount = 0;

extern GameState *CurrState;

void Frontend_Exit(){
    union REGS r;

    /* Restore normal text mode */
    r.h.ah = 0x00;
    r.h.al = 0x03;
    int86(0x10, &r, &r);

    exit(0);
}

char InputBuff[100];
char TimeBuf[30];

int main(void)
{
    char inkey;

    int bufpos = 0;

    unsigned int i = 0;
    unsigned int exitframe = 200;

    memset(InputBuff, 0x00, 100);

    PlaySound(JukeBox[SOUND_INTRO]);

    Gamelogic_Init();
    
    GFX_Init();
    EnterState();

    while (1) {
        GFX_DrawScreenSplit();

        SecondCount++;
        if (SecondCount == 60) {
            SecondCount = 0;
            Gamelogic_SecondTick();
            sprintf(TimeBuf, "%02d:%02d", Countdown / 60, Countdown % 60);
            DrawTextColor(70, TextLine + 2, 0x07, TimeBuf);
        }

        while (kbhit())
        {
            inkey = getch();

            //delete
            if (inkey == '\b'){
                if (bufpos) {
                    bufpos--;
                    DrawChar(4 + bufpos, TextLine + 2, ' ');
                    InputBuff[bufpos] = 0; 
                }
            }
            //enter
            else if (inkey == '\r' || inkey == '\n'){
                GameLogic_TextInput(InputBuff);
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
        Music_Task();
    }

    GFX_Exit();

    return 0;
}