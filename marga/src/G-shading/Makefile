EXES = points fragcoord ubo geometry-basic geometry-model geometry-normals instancing asteroids

default_target: $(EXES)

libGLAD.a: ../gl.c
	gcc -I ../../include/ -c ../gl.c -o gl.o
	ar qc libGLAD.a gl.o
	ranlib libGLAD.a

LDFLAGS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lstb -limgui -lassimp -g -L/usr/lib/x86_64-linux-gnu/
CPPFLAGS = -I ../../include/ -I ../lib/ -std=c++20 -I /usr/include/imgui -I /usr/include/stb -I /usr/include/assimp -g -Wall
EXTRAS = /usr/lib/x86_64-linux-gnu/libglfw.so libGLAD.a /usr/lib/x86_64-linux-gnu/libimgui.a /usr/lib/x86_64-linux-gnu/libstb.so /usr/lib/x86_64-linux-gnu/libassimp.so

# Libraries used by many chapters
shader.o: ../lib/shader.cpp ../lib/shader.h
	g++ $(CPPFLAGS) -c $< -o $@

lights.o: ../lib/lights.cpp ../lib/lights.h scene_state.o
	g++ $(CPPFLAGS) -c $< -o $@

texture.o: ../lib/texture.cpp ../lib/texture.h
	g++ $(CPPFLAGS) -c $< -o $@

model.o: ../lib/model.cpp ../lib/model.h
	g++ $(CPPFLAGS) -c $< -o $@

mesh.o: ../lib/mesh.cpp ../lib/mesh.h
	g++ $(CPPFLAGS) -c $< -o $@

scene_state.o: ../lib/scene_state.cpp ../lib/scene_state.h
	g++ $(CPPFLAGS) -c $< -o $@

imgui_dock.o: ../lib/imgui_dock.cpp ../lib/imgui_dock.h scene_state.o lights.o
	g++ $(CPPFLAGS) -c $< -o $@

basic_main.o: ../lib/basic_main.cpp ../lib/basic_main.h scene_state.o imgui_dock.o
	g++ $(CPPFLAGS) -c $< -o $@

# This chapter's files
points: points.cpp libGLAD.a shader.o ../lib/camera.h ../lib/geometry.h scene_state.o imgui_dock.o basic_main.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

fragcoord: fragcoord.cpp libGLAD.a shader.o ../lib/camera.h ../lib/geometry.h scene_state.o imgui_dock.o basic_main.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

ubo: ubo.cpp libGLAD.a shader.o ../lib/camera.h ../lib/geometry.h scene_state.o imgui_dock.o basic_main.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

geometry-basic: geometry-basic.cpp libGLAD.a shader.o ../lib/camera.h ../lib/geometry.h scene_state.o imgui_dock.o basic_main.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

geometry-model: geometry-model.cpp libGLAD.a shader.o ../lib/camera.h ../lib/geometry.h scene_state.o imgui_dock.o basic_main.o model.o mesh.o texture.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

geometry-normals: geometry-normals.cpp libGLAD.a shader.o ../lib/camera.h ../lib/geometry.h scene_state.o imgui_dock.o basic_main.o model.o mesh.o texture.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

instancing: instancing.cpp libGLAD.a shader.o ../lib/camera.h ../lib/geometry.h scene_state.o imgui_dock.o basic_main.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

asteroids: asteroids.cpp libGLAD.a shader.o ../lib/camera.h ../lib/geometry.h scene_state.o imgui_dock.o basic_main.o model.o mesh.o texture.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@


clean:
	rm -f libGLAD.a gl.o stb_image.o stb_truetype.o shader.o texture.o lights.o mesh.o model.o $(EXES)
