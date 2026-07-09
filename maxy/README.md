## Learning OpenGL

```sh
sudo apt install cmake glslc libassimp-dev libglm-dev libglfw3-dev libimgui-dev libsdl3-dev libsdl3-image-dev libstb-dev pkgconf python3-glad
glad --api="gl:compatibility=4.6" --extensions GL_KHR_debug --out-path=. c
```

```sh
mkdir build
cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..
make
```
