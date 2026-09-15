
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define far 
#include"..\gamelogic.h"

Graphic Graphics[GFXCOUNT];

long zero = 0;

int main(int argc, char *argv[]) {

    unsigned char inbuf[4];
    unsigned char outbuf[80];
    unsigned char thischar, lastchar;
    char mode = 0;
    int width = 0;

    unsigned char runningcount = 0;

    int index = 0;

    printf("writing %d entries to file %s\n", argc - 2, argv[1]);

    for (int x = 2; x < argc; x++) {

        // skip entries called NULL
        if (strcmp(argv[x], "NULL") == 0){
            Graphics[index].Length = 0;
            index ++;
            continue;
        }

        FILE *infile = fopen(argv[x], "rb");

        if (!infile) {
            printf("Can't open %s\n", argv[x]);
            exit(1);
        }

        fseek(infile, 0, SEEK_END);
        Graphics[index].Length = ftell(infile);
        fseek(infile, 0, SEEK_SET);
        
        Graphics[index].Data = malloc(Graphics[index].Length + 1);
        fread(Graphics[index].Data, Graphics[index].Length, 1, infile);
        fclose(infile);
        index ++;

    }

    FILE *fileout = fopen(argv[1], "wb");

    if (!fileout) {
        printf("Can't open %s\n", argv[1]);
        exit(1);
    }

    // output index and length to file

    long runningtotal = sizeof(long) * GFXCOUNT;

    for (int x = 0; x < index; x++) {
        if (Graphics[x].Length > 0)
            fwrite(&runningtotal, sizeof(long), 1, fileout);
        else
            fwrite(&zero, sizeof(long), 1, fileout);

        fwrite(&Graphics[x].Length, sizeof(long), 1, fileout);

        runningtotal += Graphics[x].Length;
    }

    // now actual data

    for (int x = 0; x < index; x++) {
        if (Graphics[x].Length > 0)
            fwrite(Graphics[x].Data, Graphics[x].Length, 1, fileout);
    }

    fclose (fileout);
    exit(0);
}
