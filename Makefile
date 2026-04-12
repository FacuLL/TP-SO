CC     = gcc
CFLAGS = -Wall -Iinclude 

all: master player view

master: master.c libraries/*
	$(CC) $(CFLAGS) master.c libraries/*.c -o master -lrt -lpthread -lm

player: player.c libraries/*
	$(CC) $(CFLAGS) player.c libraries/*.c -o player -lrt -lpthread -lm

view: view.c libraries/*
	apt-get install libncurses5-dev libncursesw5-dev
	$(CC) $(CFLAGS) view.c libraries/*.c -o view -lrt -lpthread -lm -lncurses

clean:
	rm -f master player view

test: 
	pvs-studio-analyzer trace -- make
	pvs-studio-analyzer analyze
	plog-converter -a '64:1,2,3;GA:1,2,3;OP:1,2,3' -t tasklist -o report.tasks PVS-Studio.log
