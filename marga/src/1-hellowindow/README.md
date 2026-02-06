The include file generated is `include/glad/gl.h`, so I had to add `include/glad` to the include path and include `glad/gl.h` instead of `glad/glad.h`

cmdline:
```
g++ -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -I ../../include/ 1-hellowindow.cpp /usr/lib/x86_64-linux-gnu/libglfw.so.3.4 libGLAD.a -o hellowindow
```

To build `libGLAD` statically:
```
gcc -I ../../include/ -c ../gl.c -o gl.o
ar qc libGLAD.a gl.o
ranlib libGLAD.a
```

The book uses GLAD 1, in Debian we have GLAD 2, so we need to make a few changes.

```
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
```

Becomes:
```
    int version = gladLoadGL(glfwGetProcAddress);
    if (!version)
```
