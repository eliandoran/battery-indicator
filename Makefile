CC = gcc
CFLAGS = -c -Wall -g -Os
LD = $(CC)
LDFLAGS = -lncurses

TARGET = a.out
OBJECTS = ${shell find . -name "*.c"}

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS)
	
clean:
	rm $(TARGET) $(OBJECTS)
