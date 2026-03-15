all: master player view

master:
	gcc ./master.c defaultValues.h structs.h -o master

player:
	gcc ./player.c -o player

view:
	gcc ./view.c -o view

clean:
	rm -f master player view