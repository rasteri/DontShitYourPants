CC = wcc
WASM = wasm
LINK = wlink
OBJDIR = ./build
DOSBOX = "C:\DOSBox-X\dosbox-x.exe"
RM = "c:\Program Files (x86)\GnuWin32\bin\rm.exe"

OBJS = \
$(OBJDIR)/main.obj \
$(OBJDIR)/gamelogic.obj \
$(OBJDIR)/tunes.obj \
$(OBJDIR)/states.obj \
$(OBJDIR)/gfx.obj \
$(OBJDIR)/raster.obj \
$(OBJDIR)/LZ4_8088.obj

GFXS = \
$(OBJDIR)/1.lz4 \
$(OBJDIR)/2.lz4 \
$(OBJDIR)/3.lz4 \
$(OBJDIR)/4.lz4 \
$(OBJDIR)/5.lz4 \
$(OBJDIR)/6.lz4 \
$(OBJDIR)/7.lz4 \
$(OBJDIR)/8.lz4 \
$(OBJDIR)/9.lz4 \
$(OBJDIR)/10.lz4 \
$(OBJDIR)/11.lz4 \
$(OBJDIR)/12.lz4 \
$(OBJDIR)/13.lz4 \
$(OBJDIR)/18.lz4 \
$(OBJDIR)/19.lz4 \
$(OBJDIR)/22.lz4 \
$(OBJDIR)/23.lz4 \
$(OBJDIR)/26.lz4 \
$(OBJDIR)/27.lz4 \
$(OBJDIR)/29.lz4 \
$(OBJDIR)/30.lz4 \
$(OBJDIR)/33.lz4 \
$(OBJDIR)/39.lz4 \
$(OBJDIR)/42.lz4 \
$(OBJDIR)/43.lz4 \
$(OBJDIR)/45.lz4 \
$(OBJDIR)/46.lz4 \
$(OBJDIR)/48.lz4 \
$(OBJDIR)/unk.lz4 \
$(OBJDIR)/50.lz4 \
$(OBJDIR)/crown.lz4 \

CFLAGS := -i="C:\WATCOM/h" -w4 -e25 -zq -od -d2 -bt=dos -ml

AFLAGS := 

LFLAGS := name dontshit d all sys dos op m op maxe=25 op q op symf

makebuilddir:
	@-mkdir $(OBJDIR)
	@-mkdir floppy

$(OBJDIR)/%.obj : %.c
	$(CC) $(CFLAGS) -fr=$@.err -fo=$@ $<

$(OBJDIR)/%.obj : %.asm
	$(WASM) $(AFLAGS) -fr=$@.err -fo=$@ $<

$(OBJDIR)/%.lz4 : gfx/%.raw
	gfx\encoder.exe $< $@.tmp r
	gfx\lz4.exe -c2 stdin $@ < $@.tmp
	-$(RM) $@.tmp

$(OBJDIR)/dontshit.exe: makebuilddir $(OBJS) $(GFXS)
	$(LINK) name dontshit d all sys dos op m=$(OBJDIR)/dontshit.map op maxe=25 op quiet op symf=$(OBJDIR)/dontshit.sym file { $(OBJS) raster.obj LZ4_8088.obj }
	-$(RM) -f floppy/*
	copy dontshit.exe floppy
	copy strings.txt floppy
	copy verbs.txt floppy
	copy build\*.lz4 floppy
	copy gfx\crown.bin floppy
	bfi -t=4 -f=$(OBJDIR)\autofloppy.img .\floppy
	copy build\autofloppy.img C:\martypc\media\floppies
	$(DOSBOX) -conf dosbox.conf

.DEFAULT_GOAL := all

all: $(OBJDIR)/dontshit.exe

clean:
	-$(RM) -f $(OBJDIR)/*
	-$(RM) -f floppy/*

FORCE: ;