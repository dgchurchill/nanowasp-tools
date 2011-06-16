
TARGET=ana2dsk
OBJS=ana2dsk.o anadisk.o
LIBS=-ldsk

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $@

anadisk.o: anadisk.h
ana2dsk.o: anadisk.h

.PHONY: clean
clean:
	-rm $(OBJS)
	-rm $(TARGET)

