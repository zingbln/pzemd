CC = gcc

LIBS = -lmodbus -ljansson -lcurl 

default: all

TARGET = pzemd

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) -o $(TARGET) $(TARGET).c $(LIBS)

clean:
	$(RM) $(TARGET)
