#ifndef Window_H
#define Window_H

#include <chrono>

#define GLFW_INCLUDE_NONE
#include "glad/glad.h"
#include "glfw/glfw3.h"

class Window
{
private:
	Window() = delete;
	~Window() = delete;
	Window(const Window&) = delete;
	Window(Window&&) noexcept = delete;
	Window& operator=(const Window&) = delete;
	Window& operator=(Window&&) noexcept = delete;

public:
	static GLFWwindow* Interface;
	static std::chrono::time_point<std::chrono::steady_clock> Current, Previous;
	static bool Timer;

	static bool Open();
	static bool Update();
	static bool Close();
	static float DeltaTime(const float = 1.0f);
};

#endif