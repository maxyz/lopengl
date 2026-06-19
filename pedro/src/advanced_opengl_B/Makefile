EXES = 1refactor2 2cubemaps

LIB = libGLAD.a stb_image.o shader.o texture.o camera.o material.o model.o mesh.o  key_command.o buffers.o engine.o basic_main.o

default_target: $(EXES)

libGLAD.a: ../glad.c
	gcc -I ../../include/ -c ../glad.c -o glad.o
	ar qc libGLAD.a glad.o
	ranlib libGLAD.a

LDFLAGS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lassimp
CPPFLAGS = -I ../../include/ -I ../lib/ $(shell pkg-config --cflags assimp) -g
EXTRAS = /usr/lib/x86_64-linux-gnu/libglfw.so libGLAD.a

1refactor2: 1refactor2.cpp $(LIB)
	g++ $(CPPFLAGS) $^ $(EXTRAS) -o $@ $(LDFLAGS)

2cubemaps: 2cubemaps.cpp $(LIB)
	g++ $(CPPFLAGS) $^ $(EXTRAS) -o $@ $(LDFLAGS)

camera.o: ../lib/camera.cpp ../lib/camera.h
	g++ $(CPPFLAGS) -c $< -o $@

texture.o: ../lib/texture.cpp ../lib/texture.h
	g++ $(CPPFLAGS) -c $< -o $@

shader.o: ../lib/shader.cpp ../lib/shader.h
	g++ $(CPPFLAGS) -c $< -o $@

mesh.o: ../lib/mesh.cpp ../lib/mesh.h
	g++ $(CPPFLAGS) -c $< -o $@

model.o: ../lib/model.cpp ../lib/model.h
	g++ $(CPPFLAGS) -c $< -o $@

key_command.o: ../lib/key_command.cpp ../lib/key_command.h
	g++ $(CPPFLAGS) -c $< -o $@

stb_image.o: ../lib/stb_image.cpp
	g++ $(CPPFLAGS) -c $< -o $@
	
material.o: ../lib/material.cpp
	g++ $(CPPFLAGS) -c $< -o $@

buffers.o: ../lib/buffers.cpp ../lib/buffers.h
	g++ $(CPPFLAGS) -c $< -o $@

engine.o: ../lib/engine.cpp ../lib/engine.h
	g++ $(CPPFLAGS) -c $< -o $@

basic_main.o: ../lib/basic_main.cpp ../lib/basic_main.h
	g++ $(CPPFLAGS) -c $< -o $@

clean:
	rm -f glad.o $(LIB) $(EXES)

# en general:
# file: file.cpp libGLAD.a stb_image.o shader.o texture.o camera.o
#	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@