default_target: hellotriangle

libGLAD.a: ../gl.c
	gcc -I ../../include/ -c ../gl.c -o gl.o
	ar qc libGLAD.a gl.o
	ranlib libGLAD.a

hellotriangle: hellotriangle.cpp libGLAD.a
	g++ -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -I ../../include/ $^ /usr/lib/x86_64-linux-gnu/libglfw.so libGLAD.a -o $@

clean:
	rm hellotriangle libGLAD.a gl.o
