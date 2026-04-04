CC     = gcc
CFLAGS = -Wall -Iinclude 

all: master player view

master: master.c libraries/*
	$(CC) $(CFLAGS) master.c libraries/*.c -o master -lrt -lpthread -lm

player: player.c libraries/*
	$(CC) $(CFLAGS) player.c libraries/*.c -o player -lrt -lpthread -lm

view: view.c libraries/*
	$(CC) $(CFLAGS) view.c libraries/*.c -o view -lrt -lpthread -lm -lncurses

clean:
	rm -f master player view
