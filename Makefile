CC = gcc
CFLAGS = -Wall -Iinclude `pkg-config --cflags gtk+-3.0 librsvg-2.0`
LDFLAGS = `pkg-config --libs gtk+-3.0 librsvg-2.0` -lm
SRC = src/main.c src/dns.c src/svg.c
OBJ = $(SRC:.c=.o)
TARGET = dns-toggle
all: $(TARGET)
$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
clean:
	rm -f $(OBJ) $(TARGET)
