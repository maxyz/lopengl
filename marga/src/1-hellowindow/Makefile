default_target: hellowindow

libGLAD.a: ../gl.c
	gcc -I ../../include/ -c ../gl.c -o gl.o
	ar qc libGLAD.a gl.o
	ranlib libGLAD.a

hellowindow: 1-hellowindow.cpp
	g++ -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -I ../../include/ 1-hellowindow.cpp /usr/lib/x86_64-linux-gnu/libglfw.so.3.4 libGLAD.a -o hellowindow

