#include "renderer/core/OpenglContext.hpp"

#include "BuildSettings.hpp"

#include <string>
#include <iostream>

#include <iostream>
#include <string>

#if BUILD_TARGET == NATIVE_BUILD

static void GLAPIENTRY openglDebugCallback(GLenum source, GLenum type, GLuint id,
                                           GLenum severity, GLsizei length,
                                           const GLchar* message, const void* userParam)
{
    std::string sourceStr, typeStr, severityStr;

    // Decode the 'source' parameter
    switch (source) {
        case GL_DEBUG_SOURCE_API: sourceStr = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceStr = "Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: sourceStr = "Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION: sourceStr = "Application"; break;
        case GL_DEBUG_SOURCE_OTHER: sourceStr = "Other"; break;
        default: sourceStr = "Unknown";
    }

    // Decode the 'type' parameter
    switch (type) {
        case GL_DEBUG_TYPE_ERROR: typeStr = "Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behavior"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: typeStr = "Undefined Behavior"; break;
        case GL_DEBUG_TYPE_PORTABILITY: typeStr = "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE: typeStr = "Performance"; break;
        case GL_DEBUG_TYPE_OTHER: typeStr = "Other"; break;
        case GL_DEBUG_TYPE_MARKER: typeStr = "Marker"; break;
        default: typeStr = "Unknown";
    }

    // Decode the 'severity' parameter
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH: severityStr = "High"; break;
        case GL_DEBUG_SEVERITY_MEDIUM: severityStr = "Medium"; break;
        case GL_DEBUG_SEVERITY_LOW: severityStr = "Low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr = "Notification"; break;
        default: severityStr = "Unknown";
    }

    std::cerr << "OpenGL Debug Callback:\n"
              << "Source: " << sourceStr << "\n"
              << "Type: " << typeStr << "\n"
              << "ID: " << id << "\n"
              << "Severity: " << severityStr << "\n"
              << "Message: " << message << "\n";

    if (severity == GL_DEBUG_SEVERITY_HIGH) {
		assert(false);
        exit(EXIT_FAILURE);
    }
}

#endif // BUILD_TARGET == NATIVE_BUILD


static void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

OpenglContext::OpenglContext()
	: m_width(0), m_height(0), m_window(nullptr)
{
}

auto OpenglContext::init(std::string_view name, uint32_t width, uint32_t height) noexcept -> Expected<void, std::string_view>
{
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	

	m_window = glfwCreateWindow(width, height, name.data(), NULL, NULL);
	if (!m_window) {
		glfwTerminate();
		constexpr static std::string_view msg = "GFLW failed to create a window";
		return { msg };
	}
	glfwMakeContextCurrent(m_window);

	// Start OpenGL
#if BUILD_TARGET == NATIVE_BUILD
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		constexpr static std::string_view msg = "OPENGL failed to initialise.";
		return { msg };
	}
#endif // BUILD_TARGET == NATIVE_BUILD

#if BUILD_TARGET == NATIVE_BUILD
	if constexpr (BuildSettings::mode != BuildSettings::Mode::release && BuildSettings::mode != BuildSettings::Mode::debug_without_opengl_callbacks)
	{
		if (GLEW_ARB_debug_output) {
			glEnable(GL_DEBUG_OUTPUT);
			glDebugMessageCallback(openglDebugCallback, nullptr);
		}
	}
#endif // BUILD_TARGET == NATIVE_BUILD

	glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	return {};
}

void OpenglContext::clearBuffer() noexcept
{
	if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
		if (!m_window) {
			std::cerr << "Trying to \"clear buffer\" on invalid window ptr";
			exit(EXIT_FAILURE);
		}
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenglContext::stop() noexcept
{
	if (m_window) {
		glfwDestroyWindow(m_window);
		m_window = nullptr;
		m_width = 0;
		m_height = 0;
	}
}

OpenglContext::~OpenglContext()
{
	this->stop();
}

void OpenglContext::swapBuffers() noexcept
{
	if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
		if (!m_window) {
			std::cerr << "Trying to \"swap buffers\" on invalid window ptr";
			exit(EXIT_FAILURE);
		}
	}
	glfwSwapBuffers(m_window);
}

void OpenglContext::setBackgroundColour(glm::u8vec3 rgb) noexcept
{
	glClearColor(
		rgb.r / 255.0f,
		rgb.g / 255.0f,
		rgb.b / 255.0f,
		1.0f
	);
}
void OpenglContext::setBackgroundColour(glm::u8vec4 rgba) noexcept
{
	glClearColor(
		rgba.r / 255.0f,
		rgba.g / 255.0f,
		rgba.b / 255.0f,
		rgba.a / 255.0f
	);
}


bool OpenglContext::shouldClose() noexcept
{
	if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
		if (!m_window) {
			std::cerr << "Trying to check \"should close\" on invalid window ptr";
			exit(EXIT_FAILURE);
		}
	}
	return glfwWindowShouldClose(m_window);
}

bool OpenglContext::isInit() noexcept
{
	return m_window != nullptr;
}

void OpenglContext::pollEvents() noexcept
{
	if constexpr (BuildSettings::mode != BuildSettings::Mode::release) {
		if (!m_window) {
			std::cerr << "Trying to \"poll events\" on invalid window ptr";
			exit(EXIT_FAILURE);
		}
	}
	int width, height;
	glfwGetWindowSize(m_window, &width, &height);
	m_width = static_cast<uint32_t>(width);
	m_height = static_cast<uint32_t>(height);
	glfwPollEvents();
}
