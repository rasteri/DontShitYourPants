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
    {VERB_SHITPANTS, ACTION_GOTOSTATE, STATE_SHITPANTSSTANDING},
    {VERB_FART, ACTION_GOTOSTATE, STATE_SHITPANTSSTANDING},
    {VERB_SHITTOILET, ACTION_TEXTOUTPUT, STRING_WHATTOILET},
    {0,0,0}
};

GameState GameStates[] = {
    {0, 0, 0},
    {STATE_MENU, STATE_MENU, MenuActions},
    {STATE_STANDING, STATE_STANDING, StandingActions}
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

            //printf("Comparing %s -- %s\n", currsyn->Text, TextEntry);
            if (strcmp(currsyn->Text, TextEntry) == 0)
                return currverb->ID;

            currsyn = currsyn->next;
        }

        currverb = currverb->next;
    }

    return 0;
}


void LoadVerbs() {
    char line[1024];

    int linenum = 1;

    GameString *currstring;
    GameString *prevstring = NULL;

    FILE* file = fopen("strings.txt", "r");

    while (fgets(line, 1024, file)) {

        // one string per line
        currstring = malloc(sizeof(GameVerb));
        currstring->ID = linenum;
        currstring->next = NULL;

        currstring->Text = malloc(strlen(line) + 1);
        strcpy(currstring->Text, line);
        currstring->Text[strcspn(currstring->Text, "\r\n")] = 0; // strip newline

        if (prevstring != NULL)
            prevstring->next = currstring;

        if (GameStrings == NULL)
            GameStrings = currstring;

        prevstring = currstring;
        linenum++;
    }

    fclose(file);

    file = fopen("verbs.txt", "r");

    Synonym *prevsyn = NULL, *currsyn = NULL;
    GameVerb *prevverb = NULL, *currverb = NULL;

    linenum = 1;
    while (fgets(line, 1024, file))
    {
        char *token;

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

            currsyn->Text = malloc(strlen(token));

            strcpy(currsyn->Text, token);
            currsyn->Text[strcspn(currsyn->Text, "\r\n")] = 0; // strip newline

            if (prevsyn != NULL)
                prevsyn->next = currsyn;

            prevsyn = currsyn;

            token = strtok(NULL, ","); // Subsequent calls
            
        }

        prevverb = currverb;

        linenum++;
    }

    fclose(file);

    /*char *pnt;
    currverb = firstverb;
    while (currverb != NULL) {

        printf("%d\n", currverb->ID);
        currsyn = currverb->Synonyms;
        while (currsyn != NULL){
            pnt = currsyn->Text;
            while (*pnt != 0){
                //printf("%02X", *pnt);
                pnt++;
            }
            printf(" --- %s\n", currsyn->Text);
            currsyn = currsyn->next;
        }

        currverb = currverb->next;
    }*/
}

GameState *CurrState = &GameStates[1];

void RunVerb(int Verb){
    GameAction *curraction = CurrState->Actions;
    while (curraction->Verb != 0) {
        if (curraction->Verb == Verb) {
            switch (curraction->Type) {
                case ACTION_TEXTOUTPUT:
                    //printf("printf string %d\n", curraction->Action);
                    printf("%s\n", FindString(curraction->Action));
                    break;
                case ACTION_GOTOSTATE:
                    printf("goto state %d\n", curraction->Action);
                    CurrState = &GameStates[curraction->Action];
                    break;

            }

        }

        curraction++;
    }
}



int main(void)
{   
    char inbuf[100];
    LoadVerbs();





    while(1) {
        fgets(inbuf, 100, stdin);
        inbuf[strcspn(inbuf, "\r\n")] = 0; // strip newline
        //printf("verb ID %d\n",FindVerb(inbuf));
        RunVerb(FindVerb(inbuf));
    }

    return 0;
}