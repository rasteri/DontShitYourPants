del autofloppy.img
del /Q floppy\*.* 
copy gfxtest.exe floppy
copy fun.dat floppy
bfi -t=4 -f=autofloppy.img .\floppy