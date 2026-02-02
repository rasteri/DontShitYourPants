
del /Q floppy\*.* 
copy dontshit.exe floppy
copy u4.dat floppy
copy strings.txt floppy
copy verbs.txt floppy
bfi -t=4 -f=autofloppy.img .\floppy