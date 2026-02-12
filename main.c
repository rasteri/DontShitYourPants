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

    /* Restore normal text mode */
    r.h.ah = 0x00;
    r.h.al = 0x03;
    int86(0x10, &r, &r);

    exit(0);
}

int main(void)
{
    union REGS r;

    char inkey;

    char InputBuff[100];
    int bufpos = 0;

    char TimeBuf[10];
    unsigned int i = 0;
    unsigned int exitframe = 0;

    memset(InputBuff, 0x00, 100);

    PlaySound(JukeBox[SOUND_INTRO]);

    Gamelogic_Init();
    GFX_Init();

    EnterState();

    while (1) {

        GFX_DrawScreenSplit();

        SecondCount++;
        if (SecondCount == 60){
            SecondCount = 0;
            Gamelogic_SecondTick();
            sprintf(TimeBuf, "%02d:%02d", Countdown / 60, Countdown % 60);
            //sprintf(TimeBuf, "%d", keybuf_head);
            DrawTextColor(70, textline + 2, 0x07, TimeBuf);
        }

        if (kbhit())
        {
            inkey = getch();
            //delete
            if (inkey == '\b'){
                if (bufpos) {
                    bufpos--;
                    InputBuff[bufpos] = 0;
                }
                ClearLine(textline + 2);
            }
            //enter
            else if (inkey == '\r'){
                GameLogic_TextInput(InputBuff);
                bufpos = 0;
                InputBuff[bufpos] = 0;
                InputBuff[bufpos+1] = 0;
                ClearLine(textline + 2);
            }
            else {
                InputBuff[bufpos] = inkey;
                InputBuff[bufpos+1] = 0;
                bufpos++;
            }

            DrawTextColor(10,  textline + 2, 0x07, InputBuff);
            DrawTextColor(8, textline + 2, 0x07, ">");
            update_cursor(strlen(InputBuff) + 10, textline + 2);
        }

        Music_Task();
    }

    /* Restore normal text mode */
    r.h.ah = 0x00;
    r.h.al = 0x03;
    int86(0x10, &r, &r);

    return 0;
}