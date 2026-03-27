/*

Simple RLE format
Value then number of bytes
Also there's a sprite format with transparency and no RLE

Invocation - 
encoder.exe in out mode [width]
*/


#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char *argv[]) {

    unsigned char inbuf[4];
    unsigned char outbuf[2];
    unsigned char thischar, lastchar;
    char mode = 0;
    int width = 0;

    FILE* fileout;
    FILE* file;

    unsigned char runningcount = 0;

    unsigned char firsttime = 1;

    if (argc < 4) {
        printf("No file specified\n");
        exit(1);
    }

    printf("reading %s, writing %s, mode %s, width %s\n", argv[1], argv[2], argv[3], argv[4]);

    if (strcmp(argv[3], "s") == 0){
        if (argc < 5){
            printf("no width specced\n");
            exit(1);
        }
        mode = 1;
        width = atoi(argv[4]);
    }
    else if (strcmp(argv[3], "b") == 0){
        mode = 0;
    }
    else {
        printf("Invalid mode\n");
        exit(1);
    }

    file = fopen(argv[1], "rb");

    if (!file){
        printf ("can't open %s\n", argv[1]);
    }
    fileout = fopen(argv[2], "wb");

    if (!fileout){
        printf ("can't open %s\n", argv[2]);
    }

    // background mode, RLE
    if (mode == 0){
        while (fread(inbuf, 4, 1, file)) // 2 pixels at a time  
        {
            thischar = inbuf[0] | (inbuf[2] << 4);
            if (firsttime) {
                // do nothing
                firsttime = 0;
            }
            else if ((thischar != lastchar) || runningcount > 250) { // max of 250 in a row
                outbuf[0] = lastchar;
                outbuf[1] = runningcount;
                fwrite(outbuf, 2, 1, fileout);

                runningcount = 0;
            }        
            else if ((thischar == lastchar)) {
                runningcount++;
                firsttime = 0;
            }
            lastchar = thischar;
        }
        
        //write last entry
        outbuf[0] = lastchar;
        outbuf[1] = runningcount;
        fwrite(outbuf, 2, 1, fileout);
        printf("last entry %X %X\n", outbuf[0], outbuf[1]);
    }

    // sprite mode, deal with transparency, no RLE
    // first byte is just colour
    //second byte is
    // bit0 - transparent first pixel
    // bit1 - transparent second pixel
    // bit2 - newline
    else if (mode == 1) {
        int widthcounter = 0;
        while (fread(inbuf, 4, 1, file)) // 2 pixels at a time
        {
            unsigned char mask = 0;

            // 0x10 is transparent colour
            if (inbuf[0] == 0x10)
                mask |= 0x01;
            if (inbuf[2] == 0x10)
                mask |= 0x02;

            widthcounter += 2;
            if (widthcounter == width){
                mask |= 0x04;
                widthcounter = 0;
            }

            outbuf[0] = (inbuf[0] & 0x0F) | ((inbuf[2] & 0x0F) << 4);
            outbuf[1] = mask;
            fwrite(outbuf, 2, 1, fileout);
        }
    }

    fclose(file);
    fclose(fileout);
}