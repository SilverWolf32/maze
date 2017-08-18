PROGRAM=maze
CC=gcc
CFLAGS=-g -pthread
LD=gcc
LFLAGS=-lncurses -pthread -lm
objects=curses.o

$(PROGRAM): $(PROGRAM).c
	$(CC) $(CFLAGS) -o $(PROGRAM) $(PROGRAM).c $(LFLAGS)
clean:
	@rm -f $(OBJECTS) $(PROGRAM)
	@rm -rf *.dSYM
%.o:%.c
	$(CC) -o $(OBJECTS) $(CFLAGS) -c $(PROGRAM).c

install:$(PROGRAM)
	make $PROGRAM
	@cp $(PROGRAM) ~/bin/
