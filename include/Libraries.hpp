#pragma once

#define WEB_BUILD 1
#define NATIVE_BUILD 2

#ifdef EMSCRIPTEN
#define BUILD_TARGET 1
#else
#define BUILD_TARGET 2
#endif

#if BUILD_TARGET == WEB_BUILD
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <webgl/webgl1_ext.h>
#include <webgl/webgl2_ext.h>
#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>

#elif BUILD_TARGET == NATIVE_BUILD
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#endif 

