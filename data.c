#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include "gamelogic.h"

GameVerb *firstverb = NULL;
GameString *GameStrings = NULL;

GameAction MenuActions[] = {

    {VERB_PLAY, ACTION_GOTOSTATE, STATE_STANDING},
    {VERB_PLAY, ACTION_TEXTOUTPUT, STRING_REALLYNEEDSHIT},
    {VERB_DELETE, ACTION_DELETEAWARDS, 0},
    {VERB_DELETE, ACTION_GOTOSTATE, STATE_STANDING},
    {VERB_AWARDS, ACTION_GOTOSTATE, STATE_AWARDS},
    {VERB_MENU, ACTION_TEXTOUTPUT, STRING_ALREADYMENU},
    {VERB_QUIT, ACTION_EXITGAME, 0},
    {VERB_IDIOT, ACTION_TEXTOUTPUT, STRING_IDIOT},
    {VERB_CREDITS, ACTION_GOTOSTATE, STATE_CREDITS},
    {VERB_SHIT, ACTION_GOTOSTATE, STATE_STARTINGGUN},
    {0,0,0}
};

GameAction StandingActions[] = {

    {VERB_SHIT, ACTION_GOTOSTATE, STATE_SHITPANTSSTANDING},
    {VERB_SHITPANTS, ACTION_GOTOSTATE, STATE_FARTPANTSON},
    {VERB_FART, ACTION_GOTOSTATE, STATE_FARTPANTSON},
    {VERB_SHITTOILET, ACTION_TEXTOUTPUT, STRING_WHATTOILET},
    {VERB_DONTSHIT, ACTION_GOTOSTATE, STATE_DONTSHITPANTSON},
    {VERB_BREAK, ACTION_GOTOSTATE, STATE_BREAKPANTSON},
    {VERB_DIE, ACTION_GOTOSTATE, STATE_DIEPANTSON},
    {VERB_REMOVEPANTS, ACTION_GOTOSTATE, STATE_STANDINGPANTSOFF},
    {VERB_REMOVEPANTS, ACTION_TEXTOUTPUT, STRING_REMOVEPANTS},
    {VERB_WEARPANTS, ACTION_TEXTOUTPUT, STRING_ALREADYON },
    {VERB_OPENDOOR, ACTION_TEXTOUTPUT, STRING_PUSHDOOR},
    {VERB_PULLDOOR, ACTION_GOTOSTATE, STATE_DOOROPEN},
    {VERB_PULLDOOR, ACTION_TEXTOUTPUT, STRING_PULLDOOR},
    {VERB_CLOSEDOOR, ACTION_TEXTOUTPUT, STRING_ALREADYCLOSED},
    {VERB_SITTOILET, ACTION_TEXTOUTPUT, STRING_CANTSIT},
    {VERB_STAND, ACTION_TEXTOUTPUT, STRING_STANDMORE},
    {VERB_LOOK, ACTION_TEXTOUTPUT, STRING_LOOK},
    {VERB_LOOKWASHROOM, ACTION_TEXTOUTPUT, STRING_CANTSEE},
    {VERB_LOOKDOOR, ACTION_TEXTOUTPUT, STRING_LOOKDOOR},
    {VERB_LOOKTOILET, ACTION_TEXTOUTPUT, STRING_WHATTOILET},
    {VERB_TIMEOUT, ACTION_GOTOSTATE, STATE_TIMEOVERSTANDINGPANTSON },
    {VERB_PILLSACTIVE, ACTION_GOTOSTATE, STATE_PILLSSTANDINGPANTSON1},
    {0,0,0}
};

GameAction StandingPantsOffActions[] = {

    {VERB_SHIT, ACTION_GOTOSTATE, STATE_SHITONFLOOR},
    {VERB_SHITPANTS, ACTION_GOTOSTATE, STATE_SHITINPANTSWHILEOFF},
    {VERB_FART, ACTION_GOTOSTATE, STATE_FARTPANTSOFF},
    {VERB_SHITTOILET, ACTION_TEXTOUTPUT, STRING_WHATTOILET},
    {VERB_DONTSHIT, ACTION_GOTOSTATE, STATE_DONTSHITPANTSOFF},
    {VERB_BREAK, ACTION_GOTOSTATE, STATE_BREAKPANTSOFF},
    {VERB_DIE, ACTION_GOTOSTATE, STATE_DIEPANTSOFF},
    {VERB_REMOVEPANTS, ACTION_TEXTOUTPUT, STRING_ALREADYOFF},
    {VERB_WEARPANTS, ACTION_GOTOSTATE, STATE_STANDING},
    {VERB_WEARPANTS, ACTION_TEXTOUTPUT, STRING_WEARPANTS },
    {VERB_OPENDOOR, ACTION_TEXTOUTPUT, STRING_PUSHDOOR},
    {VERB_PULLDOOR, ACTION_GOTOSTATE, STATE_DOOROPENPANTSOFF},
    {VERB_PULLDOOR, ACTION_TEXTOUTPUT, STRING_PULLDOOR},
    {VERB_CLOSEDOOR, ACTION_TEXTOUTPUT, STRING_ALREADYCLOSED},
    {VERB_SITTOILET, ACTION_TEXTOUTPUT, STRING_CANTSIT},
    {VERB_STAND, ACTION_TEXTOUTPUT, STRING_STANDMORE},
    {VERB_LOOK, ACTION_TEXTOUTPUT, STRING_LOOKNOPANTS},
    {VERB_LOOKWASHROOM, ACTION_TEXTOUTPUT, STRING_CANTSEE},
    {VERB_LOOKDOOR, ACTION_TEXTOUTPUT, STRING_LOOKDOOR},
    {VERB_LOOKTOILET, ACTION_TEXTOUTPUT, STRING_WHATTOILET},
    {VERB_TIMEOUT, ACTION_GOTOSTATE, STATE_TIMEOVERSTANDINGPANTSOFF },
    {VERB_PILLSACTIVE, ACTION_GOTOSTATE, STATE_PILLSSTANDINGPANTSOFF1},
    {0,0,0}
};

GameAction StandingDoorOpenActions[] = {

    {VERB_SHIT, ACTION_GOTOSTATE, STATE_SHITPANTSSTANDING},
    {VERB_SHITPANTS, ACTION_GOTOSTATE, STATE_FARTPANTSON},
    {VERB_FART, ACTION_GOTOSTATE, STATE_FARTPANTSON},
    {VERB_SHITTOILET, ACTION_GOTOSTATE, STATE_SHITINPANTSSITTING},
    {VERB_DONTSHIT, ACTION_GOTOSTATE, STATE_DONTSHITPANTSON},
    {VERB_BREAK, ACTION_GOTOSTATE, STATE_BREAKPANTSON},
    {VERB_DIE, ACTION_GOTOSTATE, STATE_DIEPANTSON},
    {VERB_REMOVEPANTS, ACTION_GOTOSTATE, STATE_DOOROPENPANTSOFF},
    {VERB_REMOVEPANTS, ACTION_TEXTOUTPUT, STRING_REMOVEPANTS},
    {VERB_WEARPANTS, ACTION_TEXTOUTPUT, STRING_ALREADYON },
    {VERB_OPENDOOR, ACTION_TEXTOUTPUT, STRING_ALREADYOPEN},
    {VERB_PULLDOOR, ACTION_TEXTOUTPUT, STRING_ALREADYOPEN},
    {VERB_CLOSEDOOR, ACTION_GOTOSTATE, STATE_STANDING},
    {VERB_CLOSEDOOR, ACTION_TEXTOUTPUT, STRING_CLOSEDOOR},
    {VERB_SITTOILET, ACTION_GOTOSTATE, STATE_ONTOILET},
    {VERB_SITTOILET, ACTION_TEXTOUTPUT, STRING_SITONTOILET},
    {VERB_STAND, ACTION_TEXTOUTPUT, STRING_STANDMORE},
    {VERB_LOOK, ACTION_TEXTOUTPUT, STRING_LOOKDOOROPEN},
    {VERB_LOOKWASHROOM, ACTION_TEXTOUTPUT, STRING_LOOKWASHROOM},
    {VERB_LOOKDOOR, ACTION_TEXTOUTPUT, STRING_LOOKDOOR},
    {VERB_LOOKTOILET, ACTION_TEXTOUTPUT, STRING_LOOKTOILET},
    {VERB_TIMEOUT, ACTION_GOTOSTATE, STATE_TIMEOVERSTANDINGPANTSON },
    {VERB_PILLSACTIVE, ACTION_GOTOSTATE, STATE_PILLSSTANDINGPANTSON1},
    {0,0,0}
};

GameAction StandingDoorOpenPantsOffActions[] = {

    {VERB_SHIT, ACTION_GOTOSTATE, STATE_SHITONFLOOR},
    {VERB_SHITPANTS, ACTION_GOTOSTATE, STATE_SHITINPANTSWHILEOFF},
    {VERB_FART, ACTION_GOTOSTATE, STATE_FARTPANTSOFF},
    {VERB_SHITTOILET, ACTION_GOTOSTATE, STATE_SHITINTOILET},
    {VERB_DONTSHIT, ACTION_GOTOSTATE, STATE_DONTSHITPANTSOFF},
    {VERB_BREAK, ACTION_GOTOSTATE, STATE_BREAKPANTSOFF},
    {VERB_DIE, ACTION_GOTOSTATE, STATE_DIEPANTSOFF},
    {VERB_REMOVEPANTS, ACTION_TEXTOUTPUT, STRING_ALREADYOFF},
    {VERB_WEARPANTS, ACTION_GOTOSTATE, STATE_DOOROPEN},
    {VERB_WEARPANTS, ACTION_TEXTOUTPUT, STRING_WEARPANTS },
    {VERB_OPENDOOR, ACTION_TEXTOUTPUT, STRING_ALREADYOPEN},
    {VERB_PULLDOOR, ACTION_TEXTOUTPUT, STRING_ALREADYOPEN},
    {VERB_CLOSEDOOR, ACTION_GOTOSTATE, STATE_STANDINGPANTSOFF},
    {VERB_CLOSEDOOR, ACTION_TEXTOUTPUT, STRING_CLOSEDOOR},
    {VERB_SITTOILET, ACTION_GOTOSTATE, STATE_ONTOILETPANTSOFF},
    {VERB_SITTOILET, ACTION_TEXTOUTPUT, STRING_SITONTOILET},
    {VERB_STAND, ACTION_TEXTOUTPUT, STRING_STANDMORE},
    {VERB_LOOK, ACTION_TEXTOUTPUT, STRING_LOOKNOPANTSDOOROPEN},
    {VERB_LOOKWASHROOM, ACTION_TEXTOUTPUT, STRING_LOOKWASHROOM},
    {VERB_LOOKDOOR, ACTION_TEXTOUTPUT, STRING_LOOKDOOR},
    {VERB_LOOKTOILET, ACTION_TEXTOUTPUT, STRING_LOOKTOILET},
    {VERB_TIMEOUT, ACTION_GOTOSTATE, STATE_TIMEOVERSTANDINGPANTSOFF },
    {VERB_PILLSACTIVE, ACTION_GOTOSTATE, STATE_PILLSSTANDINGPANTSOFF1},
    {0,0,0}
};

GameAction SittingActions[] = {

    {VERB_SHIT, ACTION_GOTOSTATE, STATE_SHITINPANTSSITTING},
    {VERB_SHITPANTS, ACTION_GOTOSTATE, STATE_SHITINPANTSSITTING},
    {VERB_FART, ACTION_GOTOSTATE, STATE_FARTPANTSONSITTING},
    {VERB_SHITTOILET, ACTION_GOTOSTATE, STATE_SHITINPANTSSITTING},
    {VERB_DONTSHIT, ACTION_GOTOSTATE, STATE_DONTSHITPANTSONSITTING},
    {VERB_BREAK, ACTION_TEXTOUTPUT, STRING_CONCENTRATE},
    {VERB_DIE, ACTION_GOTOSTATE, STATE_DIEPANTSONSITTING},
    {VERB_REMOVEPANTS, ACTION_TEXTOUTPUT, STRING_GETOFFTOILET},
    {VERB_WEARPANTS, ACTION_TEXTOUTPUT, STRING_ALREADYON },
    {VERB_OPENDOOR, ACTION_TEXTOUTPUT, STRING_ALREADYOPEN},
    {VERB_PULLDOOR, ACTION_TEXTOUTPUT, STRING_ALREADYOPEN},
    {VERB_CLOSEDOOR, ACTION_TEXTOUTPUT, STRING_CANTREACH},
    {VERB_SITTOILET, ACTION_TEXTOUTPUT, STRING_ALREADYSITTING},
    {VERB_STAND, ACTION_GOTOSTATE, STATE_DOOROPEN},
    {VERB_STAND, ACTION_TEXTOUTPUT, STRING_STAND},
    {VERB_LOOK, ACTION_TEXTOUTPUT, STRING_LOOKDOOROPEN},
    {VERB_LOOKWASHROOM, ACTION_TEXTOUTPUT, STRING_LOOKWASHROOM},
    {VERB_LOOKDOOR, ACTION_TEXTOUTPUT, STRING_LOOKDOOR},
    {VERB_LOOKTOILET, ACTION_TEXTOUTPUT, STRING_LOOKTOILET},
    {VERB_TIMEOUT, ACTION_GOTOSTATE, STATE_TIMEOVERSITTINGPANTSON },
    {VERB_PILLSACTIVE, ACTION_GOTOSTATE, STATE_PILLSSITTINGPANTSON1},
    {0,0,0}
};


GameAction SittingPantsOffActions[] = {

    {VERB_SHIT, ACTION_GOTOSTATE, STATE_SHITINTOILET},
    {VERB_SHITPANTS, ACTION_GOTOSTATE, STATE_SHITINPANTSWHILEOFF},
    {VERB_FART, ACTION_GOTOSTATE, STATE_SHITINTOILET},
    {VERB_SHITTOILET, ACTION_GOTOSTATE, STATE_SHITINTOILET},
    {VERB_DONTSHIT, ACTION_GOTOSTATE, STATE_DONTSHITPANTSOFFSITTING},
    {VERB_BREAK, ACTION_TEXTOUTPUT, STRING_CONCENTRATE},
    {VERB_DIE, ACTION_GOTOSTATE, STATE_DIEPANTSOFFSITTING},
    {VERB_REMOVEPANTS, ACTION_TEXTOUTPUT, STRING_ALREADYOFF},
    {VERB_WEARPANTS, ACTION_TEXTOUTPUT, STRING_GETOFFTOILET },
    {VERB_OPENDOOR, ACTION_TEXTOUTPUT, STRING_ALREADYOPEN},
    {VERB_PULLDOOR, ACTION_TEXTOUTPUT, STRING_ALREADYOPEN},
    {VERB_CLOSEDOOR, ACTION_TEXTOUTPUT, STRING_CANTREACH},
    {VERB_SITTOILET, ACTION_TEXTOUTPUT, STRING_ALREADYSITTING},
    {VERB_STAND, ACTION_GOTOSTATE, STATE_DOOROPENPANTSOFF},
    {VERB_STAND, ACTION_TEXTOUTPUT, STRING_STAND},
    {VERB_LOOK, ACTION_TEXTOUTPUT, STRING_LOOKNOPANTSDOOROPEN},
    {VERB_LOOKWASHROOM, ACTION_TEXTOUTPUT, STRING_LOOKWASHROOM},
    {VERB_LOOKDOOR, ACTION_TEXTOUTPUT, STRING_LOOKDOOR},
    {VERB_LOOKTOILET, ACTION_TEXTOUTPUT, STRING_LOOKTOILET},
    {VERB_TIMEOUT, ACTION_GOTOSTATE, STATE_TIMEOVERSITTINGPANTSOFF },
    {VERB_PILLSACTIVE, ACTION_GOTOSTATE, STATE_PILLSSITTINGPANTSOFF1},
    {0,0,0}
};

GameAction ShitOnFloorActions[] = {
    {VERB_ONENTRY, ACTION_DISPLAYGFX, STATE_SHITONFLOOR},
    {VERB_ONENTRY, ACTION_TEXTOUTPUT, STRING_ENDING_SHITONFLOOR},
    {VERB_WILDCARD, ACTION_GOTOSTATE, STATE_MENU},
    {0,0,0}
};

GameAction ShitInToiletActions[] = {
    {VERB_ONENTRY, ACTION_DISPLAYGFX, STATE_SHITINTOILET},
    {VERB_ONENTRY, ACTION_TEXTOUTPUT, STRING_ENDING_SHITINTOILET},
    {VERB_ONENTRY, ACTION_GOTOSTATE, STATE_MENU},
    {0,0,0}
};

GameAction ShitPantsStandingActions[] = {
    {VERB_ONENTRY, ACTION_DISPLAYGFX, STATE_SHITPANTSSTANDING},
    {VERB_ONENTRY, ACTION_TEXTOUTPUT, STRING_ENDING_SHITPANTSSTANDING},
    {VERB_ONENTRY, ACTION_GOTOSTATE, STATE_MENU},
    {0,0,0}
};

GameState GameStates[] = {
    {0, 0},
    {STATE_MENU, MenuActions},
    {STATE_STANDING, StandingActions},
    {STATE_STANDINGPANTSOFF, StandingPantsOffActions},
    {STATE_DOOROPEN, StandingDoorOpenActions},
    {STATE_DOOROPENPANTSOFF, StandingDoorOpenPantsOffActions},
    {STATE_ONTOILET, SittingActions},
    {STATE_ONTOILETPANTSOFF, SittingPantsOffActions},

    {STATE_AWARDS, NULL},
    {STATE_CREDITS, NULL},

    //endings
    {STATE_SHITONFLOOR, ShitOnFloorActions},
    {STATE_SHITINTOILET, ShitInToiletActions},
    {STATE_SHITPANTSSTANDING, ShitPantsStandingActions},
};

char * FindString(int id){

    GameString *currstring = GameStrings;

    while (currstring != NULL){

        if (currstring->ID == id){
            return currstring->Text;
        }

        currstring = currstring->next;
    }

    return NULL;
}

int FindVerb(char *TextEntry){

    GameVerb *currverb = NULL;
    Synonym *currsyn = NULL;

    currverb = firstverb;
    while (currverb != NULL){

        currsyn = currverb->Synonyms;
        while (currsyn != NULL) {

            if (strcmp(currsyn->Text, TextEntry) == 0)
                return currverb->ID;

            currsyn = currsyn->next;
        }

        currverb = currverb->next;
    }

    return 0;
}

char filter[4] = {'\r', '\n', 0x0B, 0x00};

void LoadVerbs() {
    char line[1024];

    int linenum = 1;

    GameString *currstring;
    GameString *prevstring = NULL;

    Synonym *prevsyn = NULL, *currsyn = NULL;
    GameVerb *prevverb = NULL, *currverb = NULL;
    char *token;
    char *pnt;
    FILE* file = fopen("strings.txt", "r");

    while (fgets(line, 1024, file)) {

        // one string per line
        currstring = malloc(sizeof(GameVerb));
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
        while(token != NULL) {



            // alloc a synonym
            currsyn = malloc(sizeof(Synonym));
            currsyn->next = NULL;

            // if first one, link it into verb
            if (currverb->Synonyms == NULL)
                currverb->Synonyms = currsyn;

            currsyn->Text = malloc(strlen(token) + 1);

            strcpy(currsyn->Text, token);


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

GameState *CurrState = &GameStates[1];

void RunAction(GameAction *curraction){

    GameAction *StateEntryAction = NULL;

    switch (curraction->Type) {
        case ACTION_TEXTOUTPUT:
            //printf("printf string %d\n", curraction->Action);
            printf("%s\n", FindString(curraction->Action));
            break;
        case ACTION_DISPLAYGFX:
            printf("display gfx %d\n", curraction->Action);
            break;
        case ACTION_GOTOSTATE:
            printf("goto state %d\n", curraction->Action);
            CurrState = &GameStates[curraction->Action];

            // run any "on entry" actions in the new state
            StateEntryAction = CurrState->Actions;
            while (StateEntryAction->Verb != 0) {
                if (StateEntryAction->Verb == VERB_ONENTRY){
                    RunAction(StateEntryAction);
                }
                StateEntryAction++;
            }
            break;

    }
}

void RunVerb(int Verb) {
    GameAction *curraction = CurrState->Actions;
    while (curraction->Verb != 0) {

        if (curraction->Verb == VERB_WILDCARD || curraction->Verb == Verb) {
            RunAction(curraction);
        }

        curraction++;
    }
}

void Gamelogic_Init(){
    LoadVerbs();
}

GameLogic_TextInput(char *Text) {
    RunVerb(FindVerb(Text));
}



/*int main(void)
{   
    char inbuf[100];


    while(1) {
        fgets(inbuf, 100, stdin);
        inbuf[strcspn(inbuf, "\r\n")] = 0; // strip newline
        printf("inputted %s\n", inbuf);
        //printf("verb ID %d\n",FindVerb(inbuf));
        RunVerb(FindVerb(inbuf));
    }

    return 0;
}*/