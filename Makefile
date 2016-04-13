CC = gcc
CFLAGS = -c -Wall -g  -std=gnu99 -include config.h
LD = $(CC)
LDFLAGS = -lm -lmcheck

TARGET = nbody

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

all: $(TARGET)
	echo $(HEADERS)

$(TARGET): $(OBJECTS) $(HEADERS)
	$(LD) -o $@ $^ $(LDFLAGS) 

%.o: %.c config.h
	$(CC) $(CFLAGS) $< -o $@

clean:	
	find . -name "*.o" -o -name "nbody"|xargs rm -f
