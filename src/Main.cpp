#include <iostream>
// #include <glaze/glaze.hpp>
// #include <glm/glm.hpp>

#ifdef EMSCRIPTEN
#include <GLES2/gl2.h>
#else
#include <gl/gl.h>
#endif

#include <GLFW/glfw3.h>

int main()
{
	auto thing = glfwCreateWindow(100, 100, "hi", NULL, NULL);
	std::cout << "Hello World\n";
}

