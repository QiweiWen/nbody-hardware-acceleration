CC = gcc
CFLAGS = -c -Wall -O2 -pg -g -std=gnu99 -include config.h -D_GNU_SOURCE
LD = $(CC)
LDFLAGS = -lm -lpthread -pg 

TARGET = nbody

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

all: $(TARGET)
	echo $(HEADERS)

$(TARGET): $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS) 

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@

clean:	
	find . -name "*.o" -o -name "nbody"|xargs rm -f
