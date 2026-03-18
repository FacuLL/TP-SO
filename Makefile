all: master player view

master:
	gcc ./master.c ./libraries/*.c -Iinclude -o master

player:
	gcc ./player.c ./libraries/*.c -Iinclude -o player

view:
	gcc ./view.c ./libraries/*.c -Iinclude -o view

clean:
	rm -f master player view