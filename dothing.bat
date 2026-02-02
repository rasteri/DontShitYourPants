call c:\watcom\owsetenv.bat
wmake -f noname.mk
del /Q floppy\*.* 
copy noname.exe floppy
copy u4.dat floppy
bfi -t=4 -f=autofloppy.img .\floppy