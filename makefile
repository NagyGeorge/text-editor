CC=gcc
CFLAGS=-Wall -Wextra -std=c99
SRC=src/main.c src/term.c src/command.c

all: ted

ted: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o ted

clean:
	rm -f ted
