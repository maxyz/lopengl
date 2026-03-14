default_target: textures

libGLAD.a: ../gl.c
	gcc -I ../../include/ -c ../gl.c -o gl.o
	ar qc libGLAD.a gl.o
	ranlib libGLAD.a

LDFLAGS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -g
CPPFLAGS = -I ../../include/
EXTRAS = /usr/lib/x86_64-linux-gnu/libglfw.so libGLAD.a

textures: textures.cpp libGLAD.a stb_image.o shader.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

shader.o: shader.cpp shader.h
	g++ $(CPPFLAGS) -c $< -o $@

stb_image.o: stb_image.cpp stb_image.h
	g++ $(CPPFLAGS) -c $< -o $@

clean:
	rm -f libGLAD.a gl.o stb_image.o shader.o textures
