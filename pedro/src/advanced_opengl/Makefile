EXES = depth_testing depth_stencil_testing blending01 blending02 framebuffers

LIB = libGLAD.a stb_image.o shader.o texture.o camera.o material.o model.o mesh.o key_command.o

default_target: $(EXES)

libGLAD.a: ../glad.c
	gcc -I ../../include/ -c ../glad.c -o glad.o
	ar qc libGLAD.a glad.o
	ranlib libGLAD.a

LDFLAGS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lassimp -g 
CPPFLAGS = -I ../../include/ -I ../lib/ $(shell pkg-config --cflags assimp)
EXTRAS = /usr/lib/x86_64-linux-gnu/libglfw.so libGLAD.a

depth_testing: depth_testing.cpp $(LIB)
	g++ $(CPPFLAGS) $^ $(EXTRAS) -o $@ $(LDFLAGS)

depth_stencil_testing: depth_stencil_testing.cpp $(LIB)
	g++ $(CPPFLAGS) $^ $(EXTRAS) -o $@ $(LDFLAGS)
	
blending01: blending01.cpp $(LIB)
	g++ $(CPPFLAGS) $^ $(EXTRAS) -o $@ $(LDFLAGS)

blending02: blending02.cpp $(LIB)
	g++ $(CPPFLAGS) $^ $(EXTRAS) -o $@ $(LDFLAGS)

framebuffers: framebuffers.cpp $(LIB)
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

clean:
	rm -f glad.o $(LIB) $(EXES)

# en general:
# file: file.cpp libGLAD.a stb_image.o shader.o texture.o camera.o
#	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@