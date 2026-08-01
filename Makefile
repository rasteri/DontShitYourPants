CC = wcc
WASM = wasm
LINK = wlink
OBJDIR = build
DOSBOX = "C:\DOSBox-X\dosbox-x.exe"
RM = "c:\Program Files (x86)\GnuWin32\bin\rm.exe"
CAT = "c:\Program Files (x86)\GnuWin32\bin\cat.exe"
ZIP = "C:\msys64\usr\bin\zip.exe"

OBJS = \
$(OBJDIR)/main.obj \
$(OBJDIR)/gamelogic.obj \
$(OBJDIR)/tunes.obj \
$(OBJDIR)/states.obj \
$(OBJDIR)/gfx.obj \
$(OBJDIR)/raster.obj \
$(OBJDIR)/random.obj \
$(OBJDIR)/LZ4_8088.obj

GFXS = \
$(OBJDIR)/1.cga \
$(OBJDIR)/2.cga \
$(OBJDIR)/3.cga \
$(OBJDIR)/4.cga \
$(OBJDIR)/5.cga \
$(OBJDIR)/6.cga \
$(OBJDIR)/7.cga \
$(OBJDIR)/8.cga \
$(OBJDIR)/9.cga \
$(OBJDIR)/10.cga \
$(OBJDIR)/11.cga \
$(OBJDIR)/12.cga \
$(OBJDIR)/13.cga \
$(OBJDIR)/18.cga \
$(OBJDIR)/19.cga \
$(OBJDIR)/22.cga \
$(OBJDIR)/23.cga \
$(OBJDIR)/26.cga \
$(OBJDIR)/27.cga \
$(OBJDIR)/29.cga \
$(OBJDIR)/30.cga \
$(OBJDIR)/33.cga \
$(OBJDIR)/39.cga \
$(OBJDIR)/42.cga \
$(OBJDIR)/43.cga \
$(OBJDIR)/45.cga \
$(OBJDIR)/46.cga \
$(OBJDIR)/48.cga \
$(OBJDIR)/unk.cga \
$(OBJDIR)/50.cga \
$(OBJDIR)/crown.cga \
$(OBJDIR)/1.ega \
$(OBJDIR)/2.ega \
$(OBJDIR)/3.ega \
$(OBJDIR)/4.ega \
$(OBJDIR)/5.ega \
$(OBJDIR)/6.ega \
$(OBJDIR)/7.ega \
$(OBJDIR)/8.ega \
$(OBJDIR)/9.ega \
$(OBJDIR)/10.ega \
$(OBJDIR)/11.ega \
$(OBJDIR)/12.ega \
$(OBJDIR)/13.ega \
$(OBJDIR)/18.ega \
$(OBJDIR)/19.ega \
$(OBJDIR)/22.ega \
$(OBJDIR)/23.ega \
$(OBJDIR)/26.ega \
$(OBJDIR)/27.ega \
$(OBJDIR)/29.ega \
$(OBJDIR)/30.ega \
$(OBJDIR)/33.ega \
$(OBJDIR)/39.ega \
$(OBJDIR)/42.ega \
$(OBJDIR)/43.ega \
$(OBJDIR)/45.ega \
$(OBJDIR)/46.ega \
$(OBJDIR)/48.ega \
$(OBJDIR)/unk.ega \
$(OBJDIR)/50.ega \
$(OBJDIR)/crown.ega \



CFLAGS := -i="C:\WATCOM/h" -w4 -e25 -zq -ot -d2 -bt=dos -ml

AFLAGS := 

LFLAGS := name dontshit d all sys dos op m op maxe=25 op q op symf

makebuilddir:
	@-mkdir $(OBJDIR)
	@-mkdir floppy

$(OBJDIR)/%.obj : %.c
	$(CC) $(CFLAGS) -fr=$@.err -fo=$@ $<

$(OBJDIR)/%.obj : %.asm
	$(WASM) $(AFLAGS) -fr=$@.err -fo=$@ $<

$(OBJDIR)/crown.cga : gfx/crown.raw
	gfx\encoder.exe $< $@ s 20

$(OBJDIR)/%.cga : gfx/%.raw
	gfx\encoder.exe $< $@.tmp r
	gfx\lz4.exe -c2 stdin $@ < $@.tmp
	-$(RM) $@.tmp

$(OBJDIR)/%.ega : gfx/%.raw
	gfx\encoder.exe $< $@ e
	gfx\lz4.exe -c2 stdin $@pl1z < $@pl1
	gfx\lz4.exe -c2 stdin $@pl2z < $@pl2
	gfx\lz4.exe -c2 stdin $@pl4z < $@pl4
	gfx\lz4.exe -c2 stdin $@pl8z < $@pl8
	$(CAT) $@pl1z $@pl2z $@pl4z $@pl8z > $@
	-$(RM) $@pl*

$(OBJDIR)/dontshit.exe: makebuilddir $(OBJS) $(GFXS)
	$(LINK) name $(OBJDIR)/dontshit.exe d all sys dos op m=$(OBJDIR)/dontshit.map op maxe=25 op quiet op symf=$(OBJDIR)/dontshit.sym file { $(OBJS) }
	-$(RM) -f floppy/*
	copy $(OBJDIR)\dontshit.exe floppy
	copy strings.txt floppy
	copy verbs.txt floppy
	copy $(OBJDIR)\*.cga floppy
	copy $(OBJDIR)\*.ega floppy
	copy gfx\crown.bin floppy
	bfi -t=4 -f=$(OBJDIR)\autofloppy.img .\floppy
	copy $(OBJDIR)\autofloppy.img C:\martypc\media\floppies
	$(DOSBOX) -conf dosbox.conf

.DEFAULT_GOAL := all

all: $(OBJDIR)/dontshit.exe

clean:
	-$(RM) -f $(OBJDIR)/*
	-$(RM) -f floppy/*

FORCE: ;