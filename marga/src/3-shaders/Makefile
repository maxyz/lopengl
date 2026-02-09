default_target: shadedtriangle refactored-triangle upsidedown-triangle offset-triangle outputcolor-triangle

libGLAD.a: ../gl.c
	gcc -I ../../include/ -c ../gl.c -o gl.o
	ar qc libGLAD.a gl.o
	ranlib libGLAD.a

LDFLAGS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl
CPPFLAGS = -I ../../include/
EXTRAS = /usr/lib/x86_64-linux-gnu/libglfw.so libGLAD.a

shadedtriangle: shadedtriangle.cpp libGLAD.a
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

refactored-triangle: refactored-triangle.cpp libGLAD.a shader.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

upsidedown-triangle: upsidedown-triangle.cpp libGLAD.a shader.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

offset-triangle: offset-triangle.cpp libGLAD.a shader.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

outputcolor-triangle: outputcolor-triangle.cpp libGLAD.a shader.o
	g++ $(LDFLAGS) $(CPPFLAGS) $^ $(EXTRAS) -o $@

shader.o: shader.cpp shader.h
	g++ $(CPPFLAGS) -c $< -o $@

clean:
	rm -f libGLAD.a gl.o shadedtriangle refactored-triangle shader.o upsidedown-triangle offset-triangle outputcolor-triangle
