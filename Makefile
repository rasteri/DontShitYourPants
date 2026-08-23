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

EGAGFX = \
$(OBJDIR)/menu.ega \
$(OBJDIR)/standing.ega \
$(OBJDIR)/standingpantsoff.ega \
$(OBJDIR)/dooropen.ega \
$(OBJDIR)/dooropenpantsoff.ega \
$(OBJDIR)/ontoilet.ega \
$(OBJDIR)/ontoiletpantsoff.ega \
$(OBJDIR)/awards.ega \
$(OBJDIR)/credits.ega \
$(OBJDIR)/shitonfloor.ega \
$(OBJDIR)/shitintoilet.ega \
$(OBJDIR)/shitpantsstanding.ega \
$(OBJDIR)/shitinpantssitting.ega \
$(OBJDIR)/diepantson.ega \
$(OBJDIR)/diepantsoff.ega \
$(OBJDIR)/pillsstandingpantson1.ega \
$(OBJDIR)/pillsstandingpantson2.ega \
$(OBJDIR)/pillsstandingpantsoff1.ega \
$(OBJDIR)/pillsstandingpantsoff2.ega \
$(OBJDIR)/pillssittingpantson1.ega \
$(OBJDIR)/pillssittingpantson2.ega \
$(OBJDIR)/pillssittingpantsoff2.ega \
$(OBJDIR)/shitinpantswhileoff.ega \
$(OBJDIR)/diepantsonsitting.ega \
$(OBJDIR)/diepantsoffsitting.ega \
$(OBJDIR)/shitonbathroomfloor.ega \
$(OBJDIR)/elvis.ega \
$(OBJDIR)/crown.ega \
$(OBJDIR)/unk1.ega \
$(OBJDIR)/unk2.ega \
$(OBJDIR)/end.ega \

CGAGFX = \
$(OBJDIR)/menu.cga \
$(OBJDIR)/standing.cga \
$(OBJDIR)/standingpantsoff.cga \
$(OBJDIR)/dooropen.cga \
$(OBJDIR)/dooropenpantsoff.cga \
$(OBJDIR)/ontoilet.cga \
$(OBJDIR)/ontoiletpantsoff.cga \
$(OBJDIR)/awards.cga \
$(OBJDIR)/credits.cga \
$(OBJDIR)/shitonfloor.cga \
$(OBJDIR)/shitintoilet.cga \
$(OBJDIR)/shitpantsstanding.cga \
$(OBJDIR)/shitinpantssitting.cga \
$(OBJDIR)/diepantson.cga \
$(OBJDIR)/diepantsoff.cga \
$(OBJDIR)/pillsstandingpantson1.cga \
$(OBJDIR)/pillsstandingpantson2.cga \
$(OBJDIR)/pillsstandingpantsoff1.cga \
$(OBJDIR)/pillsstandingpantsoff2.cga \
$(OBJDIR)/pillssittingpantson1.cga \
$(OBJDIR)/pillssittingpantson2.cga \
$(OBJDIR)/pillssittingpantsoff2.cga \
$(OBJDIR)/shitinpantswhileoff.cga \
$(OBJDIR)/diepantsonsitting.cga \
$(OBJDIR)/diepantsoffsitting.cga \
$(OBJDIR)/shitonbathroomfloor.cga \
$(OBJDIR)/elvis.cga \
$(OBJDIR)/crown.cga \
$(OBJDIR)/unk1.cga \
$(OBJDIR)/unk2.cga \
$(OBJDIR)/end.cga

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

$(OBJDIR)/crown.ega : gfx/crown.raw
	gfx\encoder.exe $< $@ s 20

$(OBJDIR)/%.cga : gfx/%.raw
	gfx\encoder.exe $< $@.tmp r
	gfx\lz4.exe -c2 stdin $@ < $@.tmp
	-$(RM) $@.tmp

$(OBJDIR)/unk.cga : gfx/48.raw
	gfx\encoder.exe $< $@.tmp r
	gfx\lz4.exe -c2 stdin $@ < $@.tmp
	-$(RM) $@.tmp

$(OBJDIR)/unk.ega : gfx/48.raw
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

$(OBJDIR)/ega.poo: makebuilddir $(EGAGFX)
	gfx\poopack.exe $(OBJDIR)\ega.poo $(EGAGFX)

$(OBJDIR)/cga.poo: makebuilddir $(CGAGFX)
	gfx\poopack.exe $(OBJDIR)\cga.poo $(CGAGFX)

$(OBJDIR)/dontshit.exe: makebuilddir $(OBJS) $(OBJDIR)/cga.poo $(OBJDIR)/ega.poo
	$(LINK) name $(OBJDIR)/dontshit.exe d all sys dos op m=$(OBJDIR)/dontshit.map op maxe=25 op quiet op symf=$(OBJDIR)/dontshit.sym file { $(OBJS) }
	-$(RM) -f floppy/*
	copy $(OBJDIR)\dontshit.exe floppy
	copy strings.txt floppy
	copy verbs.txt floppy
	copy $(OBJDIR)\cga.poo floppy
	copy $(OBJDIR)\ega.poo floppy
	bfi -t=3 -f=$(OBJDIR)\dontshit.img .\floppy
	copy $(OBJDIR)\dontshit.img C:\martypc\media\floppies
	$(DOSBOX) -conf dosbox.conf

.DEFAULT_GOAL := all

all: $(OBJDIR)/dontshit.exe

clean:
	-$(RM) -f $(OBJDIR)/*
	-$(RM) -f floppy/*

FORCE: ;