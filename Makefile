CC = wcc
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

GFXS = \
1.lz4 \
2.lz4 \
3.lz4 \
4.lz4 \
5.lz4 \
6.lz4 \
7.lz4 \
8.lz4 \
9.lz4 \
10.lz4 \
11.lz4 \
12.lz4 \
13.lz4 \
18.lz4 \
19.lz4 \
22.lz4 \
23.lz4 \
26.lz4 \
27.lz4 \
29.lz4 \
30.lz4 \
33.lz4 \
39.lz4 \
42.lz4 \
43.lz4 \
45.lz4 \
46.lz4 \
48.lz4 \
unk.lz4 \
50.lz4 \
crown.lz4 \

CFLAGS := -i="C:\WATCOM/h" -w4 -e25 -zq -od -d2 -bt=dos -ml

LFLAGS := name dontshit d all sys dos op m op maxe=25 op q op symf

makebuilddir:
	@-mkdir $(OBJDIR)

$(OBJDIR)/%.obj : %.c
	$(CC) $(CFLAGS) -fr=$@.err -fo=$@ $<

%.lz4 : gfx/%.raw
	gfx\encoder.exe $< $@.tmp r
	gfx\lz4.exe -c2 stdin $@ < $@.tmp
	-$(RM) $@.tmp

$(OBJDIR)/dontshit.exe: makebuilddir $(OBJS) $(GFXS)
	wasm raster.asm
	wasm LZ4_8088.ASM
	$(LINK) name dontshit d all sys dos op m op maxe=25 op q op symf file { $(OBJS) raster.obj LZ4_8088.obj }
	del /Q floppy\*.* 
	copy dontshit.exe floppy
	copy strings.txt floppy
	copy verbs.txt floppy
	copy *.lz4 floppy
	copy crown.bin floppy
	bfi -t=4 -f=autofloppy.img .\floppy
	copy autofloppy.img C:\martypc\media\floppies
	$(DOSBOX) -conf dosbox.conf

.DEFAULT_GOAL := all

all: $(OBJDIR)/dontshit.exe

clean:
	-$(RM) -f $(OBJDIR)/*
	-$(RM) -f dontshit.exe
	-$(RM) -f *.lz4

FORCE: ;