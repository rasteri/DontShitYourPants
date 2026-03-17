#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include "gamelogic.h"

GameVerb *firstverb = NULL;
GameString *GameStrings = NULL;
unsigned char pillstaken = 0;
int Countdown = 40;
int PillCountdown = 0;
int FartCount = 0;

unsigned long Awards = 0;

char *FindString(int id)
{

    GameString *currstring = GameStrings;

    while (currstring != NULL)
    {

        if (currstring->ID == id)
        {
            return currstring->Text;
        }

        currstring = currstring->next;
    }

    return NULL;
}

int FindVerb(char *TextEntry)
{

    GameVerb *currverb = NULL;
    Synonym *currsyn = NULL;

    currverb = firstverb;
    while (currverb != NULL)
    {

        currsyn = currverb->Synonyms;
        while (currsyn != NULL)
        {

            if (strcmp(currsyn->Text, TextEntry) == 0)
                return currverb->ID;

            currsyn = currsyn->next;
        }

        currverb = currverb->next;
    }

    DisplayText("Can't do that.");
    return 0;
}

char filter[3] = {'\r', '\n', 0x00};

void LoadVerbs()
{

    char line[1024];

    int linenum = 1;

    GameString *currstring;
    GameString *prevstring = NULL;

    Synonym *prevsyn = NULL, *currsyn = NULL;
    GameVerb *prevverb = NULL, *currverb = NULL;
    char *token;

    FILE *file = fopen("strings.txt", "r");

    while (fgets(line, 1024, file))
    {

        // one string per line
        currstring = malloc(sizeof(GameString));
        currstring->ID = linenum;
        currstring->next = NULL;

        currstring->Text = malloc(strlen(line) + 1);
        strcpy(currstring->Text, line);
        currstring->Text[strcspn(currstring->Text, filter)] = 0; // strip newline

        if (prevstring != NULL)
            prevstring->next = currstring;

        if (GameStrings == NULL)
            GameStrings = currstring;

        prevstring = currstring;
        linenum++;
    }

    fclose(file);

    file = fopen("verbs.txt", "r");

    linenum = 1;
    while (fgets(line, 1024, file))
    {
        // one verb per line
        currverb = malloc(sizeof(GameVerb));
        currverb->ID = linenum;
        currverb->Synonyms = NULL;
        currverb->next = NULL;

        if (firstverb == NULL)
            firstverb = currverb;

        if (prevverb != NULL)
            prevverb->next = currverb;

        // Get the first token
        token = strtok(line, ",");

        prevsyn = NULL;

        // Walk through other tokens
        while (token != NULL)
        {

            // alloc a synonym
            currsyn = malloc(sizeof(Synonym));
            currsyn->next = NULL;

            // if first one, link it into verb
            if (currverb->Synonyms == NULL)
                currverb->Synonyms = currsyn;

            currsyn->Text = malloc(strlen(token) + 1);

            strcpy(currsyn->Text, token);
            currsyn->Text[strcspn(currsyn->Text, filter)] = 0; // strip newline

            /*pnt = currsyn->Text;
            while (*pnt != 0){
                printf("%02X", *pnt);
                pnt++;
            }

            printf(" -- %s\n", token);*/

            if (prevsyn != NULL)
                prevsyn->next = currsyn;

            prevsyn = currsyn;

            token = strtok(NULL, ","); // Subsequent calls
        }

        prevverb = currverb;

        linenum++;
    }

    fclose(file);

    /*currverb = firstverb;
    while (currverb != NULL) {

        printf("%d\n", currverb->ID);
        currsyn = currverb->Synonyms;
        while (currsyn != NULL){
            pnt = currsyn->Text;
            while (*pnt != 0){
                printf("%02X", *pnt);
                pnt++;
            }
            printf(" --- %s\n", currsyn->Text);
            currsyn = currsyn->next;
        }

        currverb = currverb->next;
    }*/
}

GameState *CurrState = &GameStates[STATE_MENU];

void EnterState()
{
    GameAction *StateEntryAction = CurrState->Actions;

    while (StateEntryAction->Verb != 0)
    {
        if (StateEntryAction->Verb == VERB_ONENTRY)
        {
            if (RunAction(StateEntryAction))
                break; // some actions abort future ones
        }
        StateEntryAction++;
    }
}

unsigned int OldAwards = 0;


void DrawAward(int x, int y, int Award, int NameString, int DescString) {

    DrawTextColor(x + 2, y, 0x0f, FindString(NameString));

    if (Awards & Award) {
        DrawTextColor(x, y, 0x0c, "\xfb");
        DrawTextColor(x + 3, y + 1, 0x0e, FindString(DescString));
    }
    else {
        DrawTextColor(x + 3, y + 1, 0x0e, "??????");
    }
}

void SaveAwards() {
    FILE *SaveFile;
    SaveFile = fopen("save.bum", "wb");

    fwrite(&Awards, sizeof(Awards), 1, SaveFile);
    fclose (SaveFile);
}

void LoadAwards() {
    FILE *SaveFile;
    SaveFile = fopen("save.bum", "rb");

    fread(&Awards, sizeof(Awards), 1, SaveFile);
    fclose (SaveFile);
    OldAwards = Awards;
}

// returns 1 if action should stop future actions from running all others
int RunAction(GameAction *curraction)
{

    switch (curraction->Type)
    {
    case ACTION_CLEARSCREEN:
        ClearScreen();
        CrownX = 0;
        CrownY = 0;
        break;
    case ACTION_TEXTOUTPUT:
        DisplayText(FindString(curraction->Action));
        break;
    case ACTION_CROWNX:
        CrownX = curraction->Action;
        break;
    case ACTION_CROWNY:
        CrownY = curraction->Action;
        if (Awards & AWARD_SHITKING)
            GFX_DrawSprite(GFX_CROWN, CrownX, CrownY);
        break;
    case ACTION_GFXLINES:
        SetGFXLines(curraction->Action);
        break;
    case ACTION_TEXTLINES:
        SetTextLines(curraction->Action);
        break;
    case ACTION_TEXTWINDOWLINE:
        SetTextLines(curraction->Action);
        break;
    case ACTION_DISPLAYGFX:
        DisplayGFX(curraction->Action);
        break;

    case ACTION_GIVEAWARD:
        Awards |= curraction->Action;

        // All awards, give tenth award too
        if ((Awards & 0x1FF) == 0x1FF)
        Awards |= AWARD_SHITKING;
        SaveAwards();
        break;

    case ACTION_DELETEAWARDS:
        Awards = 0;
        OldAwards = Awards;
        SaveAwards();
        break;

    case ACTION_PLAYSOUND:
        PlaySound(JukeBox[curraction->Action]);

    case ACTION_DISPLAYNEWAWARDS:
        if (Awards != OldAwards){
            CurrState = &GameStates[STATE_AWARDS];
            OldAwards = Awards;
            return 1;
        }

        break;

    case ACTION_TAKEPILLS:
        pillstaken = curraction->Action;
        if (pillstaken)
        {
            PillCountdown = 45;
        }
        else {
            PillCountdown = 0;
        }
        break;

    // If action is 0, set to 0
    // otherwise increment fart counter
    case ACTION_FART:
        if (curraction->Action)
            FartCount ++;
        else
            FartCount = 0;
        break;

    case ACTION_ADDTOTIMER:
        if (curraction->Action == 0)
        {
            Countdown = 40;
        }
        else
        {
            Countdown += curraction->Action;
        }
        break;

    case ACTION_GOTOSTATE:
        // only goto elvis state if we've taken pills
        if (curraction->Action == STATE_ELVIS && !pillstaken){
            break;
        }
        CurrState = &GameStates[curraction->Action];
        break;

    case ACTION_EXITGAME:
        Frontend_Exit();
        break;

    case ACTION_DRAWMENU:
        switch (curraction->Action)
        {
            case 0:
                DrawTextColor(55, 21, 0x09, "A survival horror game");
                DrawTextColor(2, 23, 0x0f, "Instructions :");
                DrawTextColor(2, 24, 0x0f, "- To start type \"play\"");
                DrawTextColor(2, 25, 0x0f, "- To view achievements type \"awards\"");
                DrawTextColor(2, 27, 0x0f, "Goal :");
                DrawTextColor(2, 28, 0x0f, "- Don't shit your pants");
                DrawTextColor(2, 30, 0x0f, "- Type \"delete\" to delete your file");
                break;

            case 1:

                DrawAward(0, 14, AWARD_SHITINTOILET, STRING_AWARD1NAME, STRING_AWARD1DESC);
                DrawAward(0, 17, AWARD_SHITONFLOOR, STRING_AWARD2NAME, STRING_AWARD2DESC);
                DrawAward(0, 21, AWARD_SHITINPANTSSTANDING, STRING_AWARD3NAME, STRING_AWARD3DESC);
                DrawAward(0, 25, AWARD_SHITINPANTSSITTING, STRING_AWARD4NAME, STRING_AWARD4DESC);
                DrawAward(0, 29, AWARD_DIE, STRING_AWARD5NAME, STRING_AWARD5DESC);
                DrawAward(40, 17, AWARD_PILLSKICKIN, STRING_AWARD6NAME, STRING_AWARD6DESC);
                DrawAward(40, 21, AWARD_PILLSFAIL, STRING_AWARD7NAME, STRING_AWARD7DESC);
                DrawAward(40, 25, AWARD_STARTINGGUN, STRING_AWARD8NAME, STRING_AWARD8DESC);
                DrawAward(40, 29, AWARD_TIMEOVER, STRING_AWARD9NAME, STRING_AWARD9DESC);
                DrawAward(0, 32, AWARD_SHITKING, STRING_AWARD10NAME, STRING_AWARD10DESC);

            break;

            case 2:
                DrawTextColor(34, 14, 0x09, "Bonus Awards");
                DrawAward(0, 17, AWARD_SHITPANTSWHILEOFF, STRING_AWARD11NAME, STRING_AWARD11DESC);
                DrawAward(0, 21, AWARD_DONTSHIT, STRING_AWARD12NAME, STRING_AWARD12DESC);
                DrawAward(0, 25, AWARD_BELTSUSPENDERS, STRING_AWARD13NAME, STRING_AWARD13DESC);
                DrawAward(40, 17, AWARD_FART, STRING_AWARD14NAME, STRING_AWARD14DESC);
                DrawAward(40, 21, AWARD_SHITONBATHROOMFLOOR, STRING_AWARD15NAME, STRING_AWARD15DESC);
                DrawAward(40, 25, AWARD_ELVIS, STRING_AWARD16NAME, STRING_AWARD16DESC);
                /*DrawAward(40, 21, AWARD_PILLSFAIL, STRING_AWARD17NAME, STRING_AWARD17DESC);
                DrawAward(40, 25, AWARD_STARTINGGUN, STRING_AWARD18NAME, STRING_AWARD18DESC);
                DrawAward(40, 29, AWARD_TIMEOVER, STRING_AWARD19NAME, STRING_AWARD19DESC);
                DrawAward(0, 32, AWARD_SHITKING, STRING_AWARD20NAME, STRING_AWARD20DESC);*/

            break;
        }

        break;
    }
    return 0;
}

void RunVerb(int Verb)
{
    GameAction *curraction = CurrState->Actions;
    char foundone = 0;

    int oldgamestate = CurrState->ID;

    while (curraction->Verb != 0)
    {

        // different string after you take pills
        if (pillstaken) {
            if (Verb == VERB_TAKEPILLS)
                Verb = VERB_TAKEPILLSTAKEN;
            if (Verb == VERB_LOOKPILLS)
                Verb = VERB_LOOKPILLSTAKEN;
            if (Verb == VERB_LOOKPOCKET)
                Verb = VERB_LOOKPOCKETEMPTY;

        }

        // fart lightly has a use count
        if (Verb == VERB_FARTLIGHTLY1) {
            Verb += FartCount;
        }

        if (curraction->Verb == VERB_WILDCARD || curraction->Verb == Verb)
        {
            foundone = 1;
            if (RunAction(curraction)){

                break; // some actions abort future ones
            }

        }

        curraction++;
    }

    // if we changed state, run the state's entry actions
    // keep doing so until state stabilizes
    while (CurrState->ID != oldgamestate){
        oldgamestate = CurrState->ID;
        EnterState();
    }

    if (foundone)
        return;

    // check global actions if we didn't find an override
    curraction = ACTIONS_GLOBAL;
    while (curraction->Verb != 0)
    {

        if (curraction->Verb == VERB_WILDCARD || curraction->Verb == Verb)
        {
            foundone = 1;
            RunAction(curraction);
        }

        curraction++;
    }

    if (foundone)
    return;

    DisplayText("Can't do that.");
}

void Gamelogic_Init()
{
    LoadVerbs();
    LoadAwards();
}

void GameLogic_TextInput(char *Text)
{
    RunVerb(FindVerb(Text));
}

void Gamelogic_SecondTick()
{
    // don't tick outside main 6 states
    if (CurrState->ID < STATE_STANDING || CurrState->ID > STATE_ONTOILETPANTSOFF)
        return;

    // Run countdown timer
    Countdown--;

    if (Countdown == 20){
        RunVerb(VERB_20SEC);
    }

    else if (Countdown == 5){
        RunVerb(VERB_5SEC);
    }

    else if (!Countdown)
    {
        RunVerb(VERB_TIMEOUT);
    }

    // Countdown for pills kicking in
    if (PillCountdown)
    {

        PillCountdown--;

        if (!PillCountdown)
        {
            RunVerb(VERB_PILLSACTIVE);
        }
    }
}