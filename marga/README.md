## Learning OpenGL

```sh
# Install necessary dependencies to link against GLFW
sudo apt install libglfw3-dev python3-glad

# Run a command to generate the necessary include files.
glad --api="gl:compatibility=4.6" --extensions GL_KHR_debug --out-path=. c
```

