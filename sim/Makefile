# Host build of the server core.
CC      = gcc
CFLAGS  = -std=gnu99 -Wall -Wextra -Werror -O1 -I../server/core
SRC     = main.c ../server/core/game.c ../server/core/protocol.c

server_sim: $(SRC) ../server/core/game.h ../server/core/protocol.h
	$(CC) $(CFLAGS) -o $@ $(SRC)

clean:
	rm -f server_sim

.PHONY: clean
