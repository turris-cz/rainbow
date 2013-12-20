BIN=rainbow
#CC=gcc
CC=/home/robin/work/openwrt-testing/staging_dir/toolchain-powerpc_gcc-4.6-linaro_uClibc-0.9.33.2/bin/powerpc-openwrt-linux-gcc
FLAGS=-Wall -Wextra -pedantic -std=gnu99 -O0 -g
DEFINES=-D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64

$(BIN): main.o arg_parser.o reg_setters.o
	$(CC) $(FLAGS) $(DEFINES) -o $(BIN) main.o arg_parser.o reg_setters.o

main.o: main.c
	$(CC) $(FLAGS) $(DEFINES) -c -o main.o main.c

arg_parser.o: arg_parser.c
	$(CC) $(FLAGS) $(DEFINES) -c -o arg_parser.o  arg_parser.c

reg_setters.o: reg_setters.c
	$(CC) $(FLAGS) $(DEFINES) -c -o reg_setters.o reg_setters.c

clean:
	rm *.o
	rm $(BIN)
