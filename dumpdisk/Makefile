
ASM=../zmac13/zmac

CLIENT_OBJS=client.o ../serial.o
CONV_OBJS=conv.o

.PHONY: all
all: client conv dd.com

client: $(CLIENT_OBJS) ../serial.h
	$(CC) $(CLIENT_OBJS) -o client

conv: $(CONV_OBJS)
	$(CC) $(CONV_OBJS) -o conv

.PHONY: clean
clean:
	-rm client.o client
	-rm $(CONV_OBJS) conv
	-rm dd.com


$(ASM):
	cd ../zmac13 && $(MAKE)

%.com: %.asm $(ASM)
	$(ASM) -o $@ -l $<

