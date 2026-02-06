default_target: hellotriangle hellorectangle

libGLAD.a: ../gl.c
	gcc -I ../../include/ -c ../gl.c -o gl.o
	ar qc libGLAD.a gl.o
	ranlib libGLAD.a

LDFLAGS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl
CPPFLAGS = -I ../../include/

hellotriangle: hellotriangle.cpp libGLAD.a
	g++ $(LDFLAGS) $(CPPFLAGS) $^ /usr/lib/x86_64-linux-gnu/libglfw.so libGLAD.a -o $@

hellorectangle: hellorectangle.cpp libGLAD.a
	g++ $(LDFLAGS) $(CPPFLAGS) $^ /usr/lib/x86_64-linux-gnu/libglfw.so libGLAD.a -o $@

clean:
	rm hellotriangle hellorectangle libGLAD.a gl.o
