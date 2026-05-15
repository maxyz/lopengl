EXES = depth-testing stencil-testing

default_target: $(EXES)

libGLAD.a: ../gl.c
	gcc -I ../../include/ -c ../gl.c -o gl.o
	ar qc libGLAD.a gl.o
	ranlib libGLAD.a

LDFLAGS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lstb -limgui -lassimp -g -L/usr/lib/x86_64-linux-gnu/
CPPFLAGS = -I ../../include/ -I ../lib/ -std=c++20 -I /usr/include/imgui -I /usr/include/stb -I /usr/include/assimp -g
EXTRAS = /usr/lib/x86_64-linux-gnu/libglfw.so libGLAD.a /usr/lib/x86_64-linux-gnu/libimgui.a /usr/lib/x86_64-linux-gnu/libstb.so /usr/lib/x86_64-linux-gnu/libassimp.so

# Libraries used by many chapters
shader.o: ../lib/shader.cpp ../lib/shader.h
	g++ $(CPPFLAGS) -c $< -o $@

lights.o: ../lib/lights.cpp ../lib/lights.h
	g++ $(CPPFLAGS) -c $< -o $@

texture.o: ../lib/texture.cpp ../lib/texture.h
	g++ $(CPPFLAGS) -c $< -o $@

# This chapter's files
depth-testing: depth-testing.cpp libGLAD.a shader.o lights.o texture.o ../lib/camera.h
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

stencil-testing: stencil-testing.cpp libGLAD.a shader.o lights.o texture.o ../lib/camera.h
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

clean:
	rm -f libGLAD.a gl.o stb_image.o stb_truetype.o shader.o texture.o lights.o mesh.o model.o $(EXES)
