EXES = coords coords-cube many-cubes coords-ej1 coords-ej2 coords-ej3 coords-extra

default_target: $(EXES)

libGLAD.a: ../gl.c
	gcc -I ../../include/ -c ../gl.c -o gl.o
	ar qc libGLAD.a gl.o
	ranlib libGLAD.a

LDFLAGS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -g
CPPFLAGS = -I ../../include/ -I ../lib/ -std=c++20
EXTRAS = /usr/lib/x86_64-linux-gnu/libglfw.so libGLAD.a

# Libraries used by many chapters
texture.o: ../lib/texture.cpp ../lib/texture.h
	g++ $(CPPFLAGS) -c $< -o $@

shader.o: ../lib/shader.cpp ../lib/shader.h
	g++ $(CPPFLAGS) -c $< -o $@

stb_image.o: ../lib/stb_image.cpp ../lib/stb_image.h
	g++ $(CPPFLAGS) -c $< -o $@

stb_truetype.o: ../lib/stb_truetype.cpp ../lib/stb_truetype.h
	g++ $(CPPFLAGS) -c $< -o $@

write_text.o: ../lib/write_text.cpp ../lib/write_text.h stb_image.o
	g++ $(CPPFLAGS) -c $< -o $@

# This chapter's files
coords: coords.cpp libGLAD.a stb_image.o shader.o texture.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

coords-cube: coords-cube.cpp libGLAD.a stb_image.o shader.o texture.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

many-cubes: many-cubes.cpp libGLAD.a stb_image.o shader.o texture.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

coords-ej1: coords-ej1.cpp libGLAD.a stb_image.o stb_truetype.o shader.o texture.o write_text.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

coords-ej3: coords-ej3.cpp libGLAD.a stb_image.o stb_truetype.o shader.o texture.o write_text.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

coords-ej2: coords-ej2.cpp libGLAD.a stb_image.o stb_truetype.o shader.o texture.o write_text.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

coords-extra: coords-extra.cpp libGLAD.a stb_image.o stb_truetype.o shader.o texture.o write_text.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@


clean:
	rm -f libGLAD.a gl.o stb_image.o stb_truetype.o shader.o texture.o write_text.o $(EXES)
