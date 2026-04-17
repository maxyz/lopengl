EXES = colors basic basic-ej1 basic-ej3 basic-ej4 basic-imgui

default_target: $(EXES)

libGLAD.a: ../gl.c
	gcc -I ../../include/ -c ../gl.c -o gl.o
	ar qc libGLAD.a gl.o
	ranlib libGLAD.a

LDFLAGS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lstb -limgui -g -L/usr/lib/x86_64-linux-gnu/
CPPFLAGS = -I ../../include/ -I ../lib/ -std=c++20 -I /usr/include/imgui -I /usr/include/stb
EXTRAS = /usr/lib/x86_64-linux-gnu/libglfw.so libGLAD.a /usr/lib/x86_64-linux-gnu/libimgui.a /usr/lib/x86_64-linux-gnu/libstb.so

# Libraries used by many chapters
texture.o: ../lib/texture.cpp ../lib/texture.h
	g++ $(CPPFLAGS) -c $< -o $@

shader.o: ../lib/shader.cpp ../lib/shader.h
	g++ $(CPPFLAGS) -c $< -o $@

write_text.o: ../lib/write_text.cpp ../lib/write_text.h stb_image.o stb_truetype.o
	g++ $(CPPFLAGS) -c $< -o $@

# This chapter's files
colors: colors.cpp libGLAD.a shader.o write_text.o ../lib/camera.h
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

basic: basic.cpp libGLAD.a shader.o write_text.o ../lib/camera.h
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

basic-ej1: basic-ej1.cpp libGLAD.a shader.o write_text.o ../lib/camera.h
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

basic-ej3: basic-ej3.cpp libGLAD.a shader.o write_text.o ../lib/camera.h
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

basic-ej4: basic-ej4.cpp libGLAD.a shader.o write_text.o ../lib/camera.h
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

basic-imgui: basic-imgui.cpp libGLAD.a shader.o write_text.o ../lib/camera.h
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

clean:
	rm -f libGLAD.a gl.o stb_image.o stb_truetype.o shader.o texture.o write_text.o $(EXES)
