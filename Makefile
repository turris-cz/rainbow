BIN=rainbow
SRC=rainbow.c
#CC=gcc
CC=/home/robin/work/openwrt-testing/staging_dir/toolchain-powerpc_gcc-4.6-linaro_uClibc-0.9.33.2/bin/powerpc-openwrt-linux-gcc
FLAGS=-Wall -Wextra -pedantic -std=gnu99 -O0 -g
DEFINES=-D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64

$(BIN): $(SRC)
	$(CC) $(FLAGS) $(DEFINES) -o $(BIN) $(SRC)

clean:
	rm $(BIN)
