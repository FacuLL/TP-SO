all: master player view

master:
	gcc ./master.c ./inlcude/defaultValues.h ./inlcude/structs.h ./inlcude/structs.h ./include/semaphores.h ./include/shared.h -o master

player:
	gcc ./player.c -o player

view:
	gcc ./view.c -o view

clean:
	rm -f master player view