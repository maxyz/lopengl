
all: resultado

dependencias:
	echo "Hola" > dependencias

resultado: dependencias d2 d3
	ls dependencias d2 d3
	cat /dev/null > resultado

src/glad.o: src/glad.c
	gcc -Wall -g -c src/glad.c -o src/glad.o -I./include

src/04_textures/exercises/textures_ej02: src/04_textures/exercises/textures_ej02.cpp src/glad.o
	gcc -Wall -g $^ -o $@ -I./include

.PHONY: all
