CC = gcc
CFLAGS = -c -Wall  -std=gnu99 -O3 -include config.h
LD = $(CC)
LDFLAGS = -lm

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
