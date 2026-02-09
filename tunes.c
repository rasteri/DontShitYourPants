#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <graph.h>
#include <string.h>
#include <stdlib.h>

#include "gamelogic.h"

unsigned long NoteTable[] = {
    145940,
    137749,
    130018,
    122721,
    115833,
    109332,
    103196,
    97404,
    91937,
    86777,
    81906,
    77309,
    72970,
    68875,
    65009,
    61360,
    57917,
    54666,
    51598,
    48702,
    45968,
    43388,
    40953,
    38655,
    36485,
    34437,
    32505,
    30680,
    28958,
    27333,
    25799,
    24351,
    22984,
    21694,
    20477,
    19327,
    18243,
    17219,
    16252,
    15340,
    14479,
    13666,
    12899,
    12175,
    11492,
    10847,
    10238,
    9664,
    9121,
    8609,
    8126,
    7670,
    7240,
    6833,
    6450,
    6088,
    5746,
    5424,
    5119,
    4832,
    4561,
    4305,
    4063,
    3835,
    3620,
    3417,
    3225,
    3044,
    2873,
    2712,
    2560,
    2416,
    2280,
    2152,
    2032,
    1918,
    1810,
    1708,
    1612,
    1522,
    1437,
    1356,
    1280,
    1208,
    1140,
    1076,
    1016,
    959,
    905,
    854,
    806,
    761,
    718,
    678,
    640,
    604,
    570,
    538,
    508,
    479,
    452,
    427,
    403,
    380,
    359,
    339,
    320,
    302,
    285,
    269,
    254,
    240,
    226,
    214,
    202,
    190,
    180,
    169,
    160,
    151,
    143,
    135,
    127,
    120,
    113,
    107,
    101,
    95
};



Note SONG_INTRO[] = {

    {0, 44},
    {600, 255},
    {750, 38},
    {1200, 41},
    {1500, 40},
    {1800, 255},
    {2100, 36},
    {2500, 39},
    {2850, 38},
    {3150, 255},
    {3450, 33},
    {3900, 36},
    {4200, 35},
    {4500, 255},
    {4800, 31},
    {5250, 255},
    {5400, 33},
    {6000, 255},
    {0, 0}
};

Note SONG_SUCCESS[] = {

    {0, 67},
    {600, 255},
    {900, 69},
    {1200, 70},
    {1450, 255},
    {1800, 67},
    {2100, 255},
    {2400, 78},
    {2700, 76},
    {3000, 74},
    {3300, 255},
    {3600, 79},
    {3900, 255},
    {4200, 43},
    {4500, 255},
    {0, 0}
};

Note SONG_FAILURE[] = {

    {0, 44},
    {250, 255},
    {1000, 36},
    {1350, 255},
    {2000, 37},
    {2350, 255},
    {0, 0}
};

Note SONG_AWARD[] = {

    {0, 95},
    {50, 96},
    {200, 93},
    {350, 97},
    {500, 255},
    {0, 0}
};

Note *JukeBox[] = {
    SONG_INTRO,
    SONG_SUCCESS,
    SONG_FAILURE,
    SONG_AWARD
};


Note *CurrNote;
unsigned int MSPerFrame = 14;
unsigned long MSCounter = 0;

void note(unsigned char note) {

    // Set timer to frequency
    outp(0x43, 0xB6);
    outp(0x42, NoteTable[note] & 0xFF);
    outp(0x42, NoteTable[note] >> 8);
    // Turn speaker on
    outp(0x61, inp(0x61) | 3);
}

void nosound() {
    // Turn speaker off
    outp(0x61, inp(0x61) & 0xFC);
}

void Music_Task()
{
    if (CurrNote->NoteNum != 0) {

        MSCounter += MSPerFrame;

        if (MSCounter >= CurrNote->Time){
            if (CurrNote->NoteNum == 255){
                nosound();
            }
            else {
                note(CurrNote->NoteNum);
            }
            CurrNote++;
        }
    }
}

void PlaySound(Note *Song){
    CurrNote = Song;
    MSCounter = 0;
}