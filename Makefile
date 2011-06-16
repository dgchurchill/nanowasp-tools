

MIX_OBJS=mix.o serial.o
MOX_OBJS=mox.o serial.o

ASM=zmac13/zmac


.PHONY: all
all: mix mox talk.com ana2dsk dumpdisk dumprom

mix: $(MIX_OBJS)
	$(CC) $(MIX_OBJS) -o mix

mox: $(MOX_OBJS)
	$(CC) $(MOX_OBJS) -o mox

.PHONY: ana2dsk
ana2dsk:
	cd ana2dsk && $(MAKE)

.PHONY: dumpdisk
dumpdisk:
	cd dumpdisk && $(MAKE)

.PHONY: dumprom
dumprom:
	cd dumprom && $(MAKE)

.PHONY: clean
clean:
	-rm $(MIX_OBJS) mix
	-rm $(MOX_OBJS) mox
	-rm talk.com

.PHONY: cleanall
cleanall: clean
	cd ana2dsk && $(MAKE) clean
	cd dumpdisk && $(MAKE) clean
	cd dumprom && $(MAKE) clean

serial.o: serial.h
mox.o: serial.h
mix.o: serial.h


$(ASM):
	cd zmac13 && $(MAKE)

%.com: %.asm $(ASM)
	$(ASM) -o $@ -l $<

