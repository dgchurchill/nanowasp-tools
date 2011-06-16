
ASM=../zmac13/zmac

.PHONY: all
all: readchr readrom showchr getchr.com getrom56.com

readchr: readchr.o ../serial.o ../serial.h
	$(CC) readchr.o ../serial.o -o readchr

readrom: readrom.o ../serial.o ../serial.h
	$(CC) readrom.o ../serial.o -o readrom

showchr: showchr.c
	$(CC) showchr.c $$(sdl-config --cflags --libs) -o showchr


.PHONY: clean
clean:
	-rm readchr.o readchr
	-rm readrom.o readrom
	-rm showchr
	-rm getchr.com
	-rm getrom56.com


$(ASM):
	cd ../zmac13 && $(MAKE)

%.com: %.asm $(ASM)
	$(ASM) -o $@ -l $<

