CC     = gcc
CFLAGS = -Wall -Iinclude

all: master player view

master: master.c libraries/semaphores.c libraries/shared.c
	$(CC) $(CFLAGS) master.c libraries/semaphores.c libraries/shared.c -o master -lrt -lpthread

player: player.c
	$(CC) $(CFLAGS) player.c -o player -lrt -lpthread

view: view.c
	$(CC) $(CFLAGS) view.c -o view -lrt -lpthread -lncurses

clean:
	rm -f master player view
