EXES = coords01 coords02 coords_ej01

default_target: $(EXES)

libGLAD.a: ../glad.c
	gcc -I ../../include/ -c ../glad.c -o glad.o
	ar qc libGLAD.a glad.o
	ranlib libGLAD.a

LDFLAGS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -g
CPPFLAGS = -I ../../include/ -I ../lib/
EXTRAS = /usr/lib/x86_64-linux-gnu/libglfw.so libGLAD.a

coords01: coords01.cpp libGLAD.a stb_image.o shader.o texture.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

coords02: coords02.cpp libGLAD.a stb_image.o shader.o texture.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

coords_ej01: coords_ej01.cpp libGLAD.a stb_image.o shader.o texture.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

texture.o: ../lib/texture.cpp ../lib/texture.h
	g++ $(CPPFLAGS) -c $< -o $@

shader.o: ../lib/shader.cpp ../lib/shader.h
	g++ $(CPPFLAGS) -c $< -o $@

stb_image.o: ../lib/stb_image.cpp
	g++ $(CPPFLAGS) -c $< -o $@

clean:
	rm -f libGLAD.a glad.o stb_image.o stb_image.o shader.o texture.o $(EXES)

# en general:
# file: file.cpp libGLAD.a stb_image.o shader.o texture.o
#	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@