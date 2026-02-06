default_target: hellotriangle hellorectangle twotriangles twotriangles-b twotriangles-c

libGLAD.a: ../gl.c
	gcc -I ../../include/ -c ../gl.c -o gl.o
	ar qc libGLAD.a gl.o
	ranlib libGLAD.a

LDFLAGS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl
CPPFLAGS = -I ../../include/
EXTRAS = /usr/lib/x86_64-linux-gnu/libglfw.so libGLAD.a

hellotriangle: hellotriangle.cpp libGLAD.a
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

hellorectangle: hellorectangle.cpp libGLAD.a
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

twotriangles: twotriangles.cpp libGLAD.a
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

twotriangles-b: twotriangles-b.cpp libGLAD.a
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

twotriangles-c: twotriangles-c.cpp libGLAD.a
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

clean:
	rm -f hellotriangle hellorectangle twotriangles twotriangles-b twotriangles-c libGLAD.a gl.o
