CC      = gcc
CFLAGS  = -std=gnu99 -Wall -Wextra -Werror -O1 -I../server/core -I../server/stm32
SRC     = main.c ../server/core/game.c ../server/core/protocol.c ../server/stm32/app.c
DEPS    = ../server/core/game.h ../server/core/protocol.h ../server/stm32/app.h \
          ../server/stm32/port.h

server_sim: $(SRC) $(DEPS)
	$(CC) $(CFLAGS) -o $@ $(SRC)

clean:
	rm -f server_sim

.PHONY: clean
