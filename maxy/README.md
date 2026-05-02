## Learning OpenGL

```sh
sudo apt install cmake libassimp-dev libglfw3-dev libimgui-dev libstb-dev pkgconf python3-glad
glad --api="gl:compatibility=4.6" --extensions GL_KHR_debug --out-path=. c
```

```sh
mkdir build
cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..
make
```
