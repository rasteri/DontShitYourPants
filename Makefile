CC = wcc
WASM = wasm
LINK = wlink
OBJDIR = build

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
$(OBJDIR)/crown.ega 

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
$(OBJDIR)/crown.cga 

CFLAGS := -w4 -e25 -zq -ot -d2 -bt=dos -mc

AFLAGS := 

makebuilddir:
	@-mkdir $(OBJDIR)
	@-mkdir floppy
	@-mkdir dosbox
	@-mkdir dosbox/.jsdos

$(OBJDIR)/encoder : encoder.c
	gcc encoder.c -o $(OBJDIR)/encoder

$(OBJDIR)/poopack : poopack.c
	gcc poopack.c -o $(OBJDIR)/poopack

$(OBJDIR)/%.obj : %.c
	$(CC) $(CFLAGS) -fr=$@.err -fo=$@ $<

$(OBJDIR)/%.obj : %.asm
	$(WASM) $(AFLAGS) -fr=$@.err -fo=$@ $<

$(OBJDIR)/crown.cga : gfx/crown.raw $(OBJDIR)/encoder
	$(OBJDIR)/encoder $< $@ s 20

$(OBJDIR)/crown.ega : gfx/crown.raw $(OBJDIR)/encoder
	$(OBJDIR)/encoder $< $@ s 20

$(OBJDIR)/%.cga : gfx/%.raw $(OBJDIR)/encoder
	$(OBJDIR)/encoder $< $@.tmp r
	./lz4 -c2st1 stdin $@ < $@.tmp > /dev/null 2>&1
	-rm $@.tmp

$(OBJDIR)/%.ega : gfx/%.raw $(OBJDIR)/encoder
	$(OBJDIR)/encoder $< $@ e
	./lz4 -c2st1 stdin $@pl2z < $@pl2 > /dev/null 2>&1
	./lz4 -c2st1 stdin $@pl1z < $@pl1 > /dev/null 2>&1
	./lz4 -c2st1 stdin $@pl4z < $@pl4 > /dev/null 2>&1
	./lz4 -c2st1 stdin $@pl8z < $@pl8 > /dev/null 2>&1
	cat $@pl1z $@pl2z $@pl4z $@pl8z > $@
	-rm $@pl*

$(OBJDIR)/monster.poo: makebuilddir $(EGAGFX) $(OBJDIR)/poopack
	$(OBJDIR)/poopack $(OBJDIR)/monster.poo $(EGAGFX)

$(OBJDIR)/massive.poo: makebuilddir $(CGAGFX) $(OBJDIR)/poopack
	$(OBJDIR)/poopack $(OBJDIR)/massive.poo $(CGAGFX)

$(OBJDIR)/dontshit: makebuilddir $(OBJS) $(OBJDIR)/massive.poo $(OBJDIR)/monster.poo
	$(LINK) name $(OBJDIR)/dontshit sys dos op m=$(OBJDIR)/dontshit.map op maxe=25 op quiet op symf=$(OBJDIR)/dontshit.sym file { $(OBJS) }
	-rm -f floppy/*
	cp $(OBJDIR)/dontshit floppy
	cp runny.poo floppy
	cp smelly.poo floppy
	cp sticky.poo floppy
	cp $(OBJDIR)/massive.poo floppy
	cp $(OBJDIR)/monster.poo floppy
	./bfi -t=0 -f=$(OBJDIR)/dontshit.img ./floppy

	-rm -f dosbox/*
	cp floppy/* dosbox
	cp jsdos/* dosbox/.jsdos
	cd dosbox && zip -r DontShit.jsdos * .jsdos
	cp dosbox/DontShit.jsdos DontShit.jsdos

.DEFAULT_GOAL := all

all: $(OBJDIR)/dontshit

clean:
	-rm -f $(OBJDIR)/*
	-rm -f floppy/*
	-rm -f dosbox/*

FORCE: ;