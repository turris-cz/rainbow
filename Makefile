BIN=rainbow
FLAGS=-Wall -Wextra -pedantic -std=gnu99 -O0 -g
DEFINES=-D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64

$(BIN): main.o arg_parser.o reg_setters.o daemon.o turris.o
	$(CC) $(FLAGS) $(DEFINES) -o $(BIN) main.o arg_parser.o reg_setters.o daemon.o turris.o

main.o: main.c
	$(CC) $(FLAGS) $(DEFINES) -c -o main.o main.c

arg_parser.o: arg_parser.c
	$(CC) $(FLAGS) $(DEFINES) -c -o arg_parser.o  arg_parser.c

reg_setters.o: reg_setters.c
	$(CC) $(FLAGS) $(DEFINES) -c -o reg_setters.o reg_setters.c

daemon.o: daemon.c
	$(CC) $(FLAGS) $(DEFINES) -c -o daemon.o daemon.c

turris.o: turris.c
	$(CC) $(FLAGS) $(DEFINES) -c -o turris.o turris.c

clean:
	rm *.o
	rm $(BIN)
