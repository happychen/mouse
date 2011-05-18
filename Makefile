#this is for chess game

scr=fbtools.c mouse_draw.c
para=-o mouse -Wall

main:$(scr)
		gcc $(scr) $(para)
clean:
		rm -rf mouse
		rm -rf *.o
		rm -rf ~*
run:
		./mouse
