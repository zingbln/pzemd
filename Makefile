CC = gcc 
CC_OPTIONS = -O  -Wmissing-prototypes -Wall -pedantic 

LIBS = -lmodbus -lcurl 

default: all

TARGET = pzemd

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC)  -o $(TARGET) $(TARGET).c $(LIBS) $(CC_OPTIONS)

clean:
	$(RM) $(TARGET)
