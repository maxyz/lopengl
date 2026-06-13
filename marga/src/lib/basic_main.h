#ifndef BASICMAIN_H
#define BASICMAIN_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "scene_state.h"
#include "imgui_dock.h"

// Abstract Factory, defined in the application source
AbstractSceneRenderer* createSceneRenderer();

#endif
