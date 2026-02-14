CC = wcc
LINK = wlink
OBJDIR = ./build
RM = "C:\Program Files (x86)\GnuWin32\bin\rm"
DOSBOX = "C:\DOSBox-X\dosbox-x.exe"

OBJS = \
$(OBJDIR)/main.obj \
$(OBJDIR)/gamelogic.obj \
$(OBJDIR)/tunes.obj \
$(OBJDIR)/states.obj \
$(OBJDIR)/gfx.obj \

CFLAGS := -i="C:\WATCOM/h" -w4 -e25 -zq -od -d2 -bt=dos -ml

LFLAGS := name dontshit d all sys dos op m op maxe=25 op q op symf

makebuilddir:
	@-mkdir $(OBJDIR)

$(OBJDIR)/%.obj : %.c
	$(CC) $(CFLAGS) -fr=$@.err -fo=$@ $<

$(OBJDIR)/dontshit.exe: makebuilddir $(OBJS)
	$(LINK) name dontshit d all sys dos op m op maxe=25 op q op symf file { $(OBJS) }
	del /Q floppy\*.* 
	copy dontshit.exe floppy
	copy strings.txt floppy
	copy verbs.txt floppy
	copy *.bin floppy
	bfi -t=4 -f=autofloppy.img .\floppy
	copy autofloppy.img C:\martypc\media\floppies
	$(DOSBOX) -conf dosbox.conf

.DEFAULT_GOAL := all

all: $(OBJDIR)/dontshit.exe

clean:
	-$(RM) -f $(OBJDIR)/*
	-$(RM) -f dontshit.exe

FORCE: ;