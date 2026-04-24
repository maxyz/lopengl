EXES = materials lightingmaps lm-ej2 lm-ej3 lm-ej4 lm-extra

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

# This chapter's files
materials: materials.cpp libGLAD.a shader.o ../lib/camera.h
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

lightingmaps: lightingmaps.cpp libGLAD.a shader.o texture.o ../lib/camera.h
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

lm-ej2: lm-ej2.cpp libGLAD.a shader.o texture.o ../lib/camera.h
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

lm-ej3: lm-ej3.cpp libGLAD.a shader.o texture.o ../lib/camera.h
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

lm-ej4: lm-ej4.cpp libGLAD.a shader.o texture.o ../lib/camera.h
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

lm-extra: lm-extra.cpp libGLAD.a shader.o texture.o ../lib/camera.h
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

clean:
	rm -f libGLAD.a gl.o stb_image.o stb_truetype.o shader.o texture.o write_text.o $(EXES)
