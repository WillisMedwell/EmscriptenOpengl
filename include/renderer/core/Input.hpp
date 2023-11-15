#pragma once

#include "OpenglContext.hpp"

#include <glm/glm.hpp>

class Input {
	OpenglContext* m_context;

	bool has_a_last_mouse_pos = false;
	glm::vec2 last_mouse_pos;
	glm::vec2 mouse_offset;

	static void mouseCallBackProxy(GLFWwindow* window, double x_pos, double y_pos)
	{
		Input* obj = static_cast<Input*>(glfwGetWindowUserPointer(window));
		obj->mouseCallBack(window, x_pos, y_pos);
	}

	void mouseCallBack(GLFWwindow* window, double x_pos, double y_pos)
	{
		if (!has_a_last_mouse_pos) {
			last_mouse_pos = glm::vec2{ x_pos, y_pos };
			has_a_last_mouse_pos = true;
		}

		mouse_offset = glm::vec2{ x_pos - last_mouse_pos.x,  y_pos - last_mouse_pos.y };
		last_mouse_pos = glm::vec2{ x_pos, y_pos };
	}

	int charToGLFWKey(char c) const
	{
		switch (c) {
		case 'W':
			return GLFW_KEY_W;
		case 'A':
			return GLFW_KEY_A;
		case 'S':
			return GLFW_KEY_S;
		case 'D':
			return GLFW_KEY_D;
		case 'R':
			return GLFW_KEY_R;
		case 'P':
			return GLFW_KEY_P;
		case 'B':
			return GLFW_KEY_B;
		case '1':
			return GLFW_KEY_1;
		case '2':
			return GLFW_KEY_2;
		case '3':
			return GLFW_KEY_3;
		case '4':
			return GLFW_KEY_4;
		case '5':
			return GLFW_KEY_5;
		case '6':
			return GLFW_KEY_6;
		case '7':
			return GLFW_KEY_7;
		case '8':
			return GLFW_KEY_8;
		case '9':
			return GLFW_KEY_9;
		case '0':
			return GLFW_KEY_0;
		default:
			return GLFW_KEY_UNKNOWN;
		}
	}



public:
	enum class Key : int {
		escape = GLFW_KEY_ESCAPE,
		left_mouse = GLFW_MOUSE_BUTTON_LEFT,
		left_shift = GLFW_KEY_LEFT_SHIFT,
		space = GLFW_KEY_SPACE
	};

	void init(OpenglContext* context) noexcept
	{
		m_context = context;
		last_mouse_pos = glm::vec2{ 0, 0 };
		has_a_last_mouse_pos = false;
		GLFWwindow* glfwWindow = reinterpret_cast<GLFWwindow*>(m_context->getWindowHandle());
		glfwSetWindowUserPointer(glfwWindow, this);
		glfwSetCursorPosCallback(glfwWindow, mouseCallBackProxy);
	}
	bool isKeyDown(char key) const
	{
		return glfwGetKey(reinterpret_cast<GLFWwindow*>(m_context->getWindowHandle()), charToGLFWKey(key)) == GLFW_PRESS;
	}
	bool isKeyDown(Key key) const
	{
		return glfwGetKey(reinterpret_cast<GLFWwindow*>(m_context->getWindowHandle()), (int)key) == GLFW_PRESS;
	}

	/*bool isKeyReleased(char key) const 
	{
		return glfwGetKey(reinterpret_cast<GLFWwindow*>(m_context->getWindowHandle()), charToGLFWKey(key)) == GLFW_RELEASE;
	}*/

	auto getMouseOffset() const -> glm::vec2
	{
		return mouse_offset;
	}
};