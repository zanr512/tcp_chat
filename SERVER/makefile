# tukaj napisite seznam izvornih datotek, samo .c dokumente
SOURCE_FILE_LIST = s.c 
# tukaj napisite zeljeno ime programa
TARGET = server

# tukaj lahko izberete drug prevajalnik
# ce zelite uporabljati C++ zamenjajte 
# gcc z g++
CC = gcc

# tukaj bomo kasneje vnesli dodatne knjiznice
LIBRARIES = -lpthread

CFLAGS = -Wall -std=c99

OBJECTS = $(SOURCE_FILE_LIST:.c=.o)
all: $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(LIBRARIES)
debug: CFLAGS += -g
debug: all
clean:
	rm -f $(TARGET) $(OBJECTS)
