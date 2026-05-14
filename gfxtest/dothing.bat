del autofloppy.img
del /Q floppy\*.* 
copy gfxtest.exe floppy
copy demo.bin floppy
bfi -t=4 -f=autofloppy.img .\floppy
copy autofloppy.img C:\martypc\media\floppies