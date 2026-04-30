EXES = light_casters light_casters2 light_casters3

LIB = libGLAD.a stb_image.o shader.o texture.o camera.o material.o

default_target: $(EXES)

libGLAD.a: ../glad.c
	gcc -I ../../include/ -c ../glad.c -o glad.o
	ar qc libGLAD.a glad.o
	ranlib libGLAD.a

LDFLAGS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -g
CPPFLAGS = -I ../../include/ -I ../lib/
EXTRAS = /usr/lib/x86_64-linux-gnu/libglfw.so libGLAD.a

light_casters: light_casters.cpp $(LIB)
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@
	
light_casters2: light_casters2.cpp $(LIB)
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@
	
light_casters3: light_casters3.cpp $(LIB)
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@
	
camera.o: ../lib/camera.cpp ../lib/camera.h
	g++ $(CPPFLAGS) -c $< -o $@

texture.o: ../lib/texture.cpp ../lib/texture.h
	g++ $(CPPFLAGS) -c $< -o $@

shader.o: ../lib/shader.cpp ../lib/shader.h
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