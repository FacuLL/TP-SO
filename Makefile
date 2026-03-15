all: master player view

master:
	gcc ./master.c -o master

player:
	gcc ./player.c -o player

view:
	gcc ./view.c -o view

clean:
	rm -f master player view