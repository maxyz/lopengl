EXES = coords

default_target: $(EXES)

libGLAD.a: ../gl.c
	gcc -I ../../include/ -c ../gl.c -o gl.o
	ar qc libGLAD.a gl.o
	ranlib libGLAD.a

LDFLAGS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -g
CPPFLAGS = -I ../../include/ -I ../lib/
EXTRAS = /usr/lib/x86_64-linux-gnu/libglfw.so libGLAD.a

# Libraries used by many chapters
texture.o: ../lib/texture.cpp ../lib/texture.h
	g++ $(CPPFLAGS) -c $< -o $@

shader.o: ../lib/shader.cpp ../lib/shader.h
	g++ $(CPPFLAGS) -c $< -o $@

stb_image.o: ../lib/stb_image.cpp ../lib/stb_image.h
	g++ $(CPPFLAGS) -c $< -o $@

# This chapter's files
coords: coords.cpp libGLAD.a stb_image.o shader.o texture.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@


clean:
	rm -f libGLAD.a gl.o stb_image.o shader.o texture.o $(EXES)
