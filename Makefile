all: master player view

master:
	gcc ./master.c ./libraries/*.c -Iinclude -o master

player:
	gcc ./player.c ./libraries/*.c -o player

view:
	gcc ./view.c ./libraries/*.c -o view

clean:
	rm -f master player view