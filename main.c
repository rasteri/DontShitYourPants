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

int SecondCount = 0;

extern GameState *CurrState;

void Frontend_Exit(){
    union REGS r;

    unhook_keyb_int();
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
    union REGS r;
    char inkey;

    int bufpos = 0;

    unsigned int i = 0;
    unsigned int exitframe = 0;

    memset(InputBuff, 0x00, 100);

    PlaySound(JukeBox[SOUND_INTRO]);

    Gamelogic_Init();
    
    GFX_Init();
    rasterDisable();
    EnterState();
    hook_keyb_int();

    rasterEnable();

    while (1) {

        GFX_DrawScreenSplit();

        SecondCount++;
        if (SecondCount == 60){
            SecondCount = 0;
            Gamelogic_SecondTick();
            sprintf(TimeBuf, "%02d:%02d", Countdown / 60, Countdown % 60);
            //sprintf(TimeBuf, "%d-%d",bufpos, keybuf_head);
            DrawTextColor(70, textline + 2, 0x07, TimeBuf);
        }

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
                    DrawChar(4 + bufpos, textline + 2, ' ');
                    InputBuff[bufpos] = 0; 
                }
            }
            //enter
            else if (inkey == '\n'){
                GameLogic_TextInput(InputBuff);
                bufpos = 0;
                InputBuff[bufpos] = 0;
                InputBuff[bufpos+1] = 0;
                ClearLine(textline + 2);
            }
            else if (inkey != 0) {
                DrawChar(4 + bufpos, textline + 2, inkey);
                InputBuff[bufpos] = inkey;
                InputBuff[bufpos+1] = 0;
                bufpos++;
            }


        }

        //DrawTextColor(4,  textline + 2, 0x07, InputBuff);
        DrawTextColor(2, textline + 2, 0x07, ">");
        update_cursor(strlen(InputBuff) + 4, textline + 2);

        keybuf_head = 0;
        Music_Task();
        //if (exitframe++ > 400)
        //    break;
    }

    unhook_keyb_int();
    /* Restore normal text mode */
    r.h.ah = 0x00;
    r.h.al = 0x03;
    int86(0x10, &r, &r);

    return 0;
}