#include<stdio.h>

int main(){

    char inbuf[4];
    char outbuf[2];

    FILE* fileout;
    FILE* file;
    file = fopen("standingdooropen.raw", "rb");
    fileout = fopen("standingdooropen.bin", "wb");

    while (fread(inbuf, 4, 1, file)) // 2 pixels at a time
    {
        outbuf[0] = 0xDD;
        outbuf[1] = inbuf[0] | (inbuf[2] << 4);
        fwrite(outbuf, 2, 1, fileout);
    }

    fclose(file);
    fclose(fileout);
}