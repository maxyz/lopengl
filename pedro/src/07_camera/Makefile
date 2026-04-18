EXES = cam cam2 camDebounce1 camDebounce2 camLockPerspective cam_ejs 

default_target: $(EXES)

libGLAD.a: ../glad.c
	gcc -I ../../include/ -c ../glad.c -o glad.o
	ar qc libGLAD.a glad.o
	ranlib libGLAD.a

LDFLAGS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -g
CPPFLAGS = -I ../../include/ -I ../lib/
EXTRAS = /usr/lib/x86_64-linux-gnu/libglfw.so libGLAD.a

cam: cam.cpp libGLAD.a stb_image.o shader.o texture.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

cam2: cam2.cpp libGLAD.a stb_image.o shader.o texture.o camera.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

camDebounce1: camDebounce1.cpp libGLAD.a stb_image.o shader.o texture.o camera.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

camDebounce2: camDebounce2.cpp libGLAD.a stb_image.o shader.o texture.o camera.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

camLockPerspective: camLockPerspective.cpp libGLAD.a stb_image.o shader.o texture.o camera.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

cam_ejs: cam_ejs.cpp libGLAD.a stb_image.o shader.o texture.o camera.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

camera.o: ../lib/camera.cpp ../lib/camera.h
	g++ $(CPPFLAGS) -c $< -o $@

texture.o: ../lib/texture.cpp ../lib/texture.h
	g++ $(CPPFLAGS) -c $< -o $@

shader.o: ../lib/shader.cpp ../lib/shader.h
	g++ $(CPPFLAGS) -c $< -o $@

stb_image.o: ../lib/stb_image.cpp
	g++ $(CPPFLAGS) -c $< -o $@

clean:
	rm -f libGLAD.a glad.o stb_image.o stb_image.o shader.o texture.o camera.o $(EXES)

# en general:
# file: file.cpp libGLAD.a stb_image.o shader.o texture.o
#	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@