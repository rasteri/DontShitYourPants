#include <stdio.h>
#include <dos.h>
#include <conio.h>

#include "gamelogic.h"

char *
strsep (char **stringp, const char *delim)
{
  char *start = *stringp;
  char *ptr;

  if (start == NULL)
    return NULL;

  /* Optimize the case of no delimiters.  */
  if (delim[0] == '\0')
    {
      *stringp = NULL;
      return start;
    }

  /* Optimize the case of one delimiter.  */
  if (delim[1] == '\0')
    ptr = strchr (start, delim[0]);
  else
    /* The general case.  */
    ptr = strpbrk (start, delim);
  if (ptr == NULL)
    {
      *stringp = NULL;
      return start;
    }

  *ptr = '\0';
  *stringp = ptr + 1;

  return start;
}

const char* tokline(char* line)
{
    #define KEYNAME 0
    #define FRIENDLYNAME 1
    #define SHORTNAME 2
    #define LEGEND 3
    #define XPOS 4
    #define YPOS 5
    #define XSIZE 6
    #define YSIZE 7
    #define USAGEPAGE 8
    #define USAGEID 9
    #define SET1MAKE 10
    #define SET1BREAK 11
    #define SET2MAKE 12
    #define SET2BREAK 13
    #define ENDOFLINE 14
    
    uint8_t field;
    
    char* tok;
    
    while ((tok = strsep(&line, ",")) != NULL)
    {
        switch(field) {
            case KEYNAME:
                strncpy(key->Name, tok, 100);
                break;
            case FRIENDLYNAME:
                strncpy(key->FriendlyName, tok, 100);
                break;
            case SHORTNAME:
                strncpy(key->ShortName, tok, 100);
                break;
            case LEGEND:
                parsehexbytesequence(key->Legend, tok);
                break;
            case XPOS:
                key->xpos = atoi(tok);
                break;
            case YPOS:
                key->ypos = atoi(tok);
                break;
            case XSIZE:
                key->xsize = atoi(tok);
                break;
            case YSIZE:
                key->ysize = atoi(tok);
                break;
            case USAGEPAGE:
                key->UsagePage = strtoul(tok, NULL, 16);
                break;
            case USAGEID:
                key->UsageID = strtoul(tok, NULL, 16);
                break;
            case SET1MAKE:
                parsehexbytesequence(key->Set1Make, tok);
                break;
            case SET1BREAK:
                parsehexbytesequence(key->Set1Break, tok);
                break;
            case SET2MAKE:
                parsehexbytesequence(key->Set2Make, tok);
                break;
            case SET2BREAK:
                parsehexbytesequence(key->Set2Break, tok);
                break;
        }
        
        if (field++ == ENDOFLINE)
            break;
    }
    return NULL;
}

int main(void)
{

}