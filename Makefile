CC = gcc
CFLAGS = -c -Wall -g  -std=gnu99
LD = $(CC)


TARGET = nbody

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

all: $(TARGET)
	echo $(HEADERS)

$(TARGET): $(OBJECTS) $(HEADERS)
	$(LD) -o $@ $^ 

%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm $(TARGET) $(OBJECTS)
