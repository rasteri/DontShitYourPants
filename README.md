# Don't Shit Your Pants XT
The classic Flash survival horror game, now available for DOS!

<img src=screenshot.png>

## Gameplay Instructions
This is an all typing game. To begin, type "play" and press ENTER. Type instructions such as "open door" to perform different tasks.

If you're stuck, make sure to "look" at everything you can.

Try to win all the awards and earn the title of Shit King!

To see the credits, type "credits" in the title screen.

## System Requirements

* IBM PC or compatible running DOS
* CGA, EGA or VGA compatible graphics card


## How To Run
Either unzip the .zip file to a folder on your hard disk, or write the .img file to a floppy disk.

Then change to the folder or disk and run "dontshit.exe"

If you have a slower PC (8086, 8088) select "1" for text above, or for faster PCs (286 or better) select "2" for text below. (Yes, the text placement has a massive impact on performance!)

Then select your graphics card - CGA, EGA200, EGA350, or VGA.

If you have an EGA-compatible card, EGA200 is preferable, but may not work if you have a 350-line monitor. If you have issues, select EGA350.


## Technical Info
Don't Shit Your Pants XT is written in Watcom C, mainly because of its maturity and convenience (cross-compilation, debugger, etc)

I may try porting it to GCC IA-16 in the future, just to see what happens.

Graphics are in the pseudo-160x100 mode used by (for example) paku paku.

Split screen gfx/text is acheived using good ol' fashioned CPC-style CRTC tricks, which appear to also work on EGA and VGA.


## Acknowledgements
* Cellar Door Games for creating the original, and for allowing me to release this tribute.
* Jim Leonard/Trixter for the LZ4 decompression code.
* CRTC+Hornet for the demo Area 5150, which helped me understand CGA screen timings
* Daniel Balsom for the MartyPC emulator, which was invaluable during development and debugging
