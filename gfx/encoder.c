/*

Simple RLE format
Value then number of bytes

*/


#include<stdio.h>
#include<stdlib.h>


int main(int argc, char *argv[]) {

    unsigned char inbuf[4];
    unsigned char outbuf[2];
    unsigned char thischar, lastchar;

    FILE* fileout;
    FILE* file;

    unsigned char runningcount = 0;

    unsigned char firsttime = 1;

    if (argc != 3) {
        printf("No file specified\n");
        exit(1);
    }

    printf("reading %s, writing %s\n", argv[1], argv[2]);


    file = fopen(argv[1], "rb");

    if (!file){
        printf ("can't open %s\n", argv[1]);
    }
    fileout = fopen(argv[2], "wb");

    if (!fileout){
        printf ("can't open %s\n", argv[2]);
    }

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

    fclose(file);
    fclose(fileout);
}