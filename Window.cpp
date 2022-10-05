#include "Logger.hpp"
#include "Window.hpp"

#define WINDOW_WIDTH 768
#define WINDOW_HEIGHT 768

GLFWwindow* Window::Interface = nullptr;
std::chrono::time_point<std::chrono::steady_clock> Window::Current, Window::Previous;
bool Window::Timer = true;

bool Window::Open()
{
	GLFWmonitor* monitor = nullptr;
	int monitorX = 0, monitorY = 0;

	if (Window::Interface != nullptr)
		return Logger::SaveMessage("Error: Window::Open() - 1.");

	else if (glfwInit() == GLFW_FALSE)
		return Logger::SaveMessage("Error: Window::Open() - 2.");

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

	Window::Interface = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Dungeon", nullptr, nullptr);

	if (Window::Interface == nullptr)
	{
		glfwTerminate();
		return Logger::SaveMessage("Error: Window::Open() - 3.");
	}

	monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);

	if (videoMode == nullptr)
	{
		glfwTerminate();
		return Logger::SaveMessage("Error: Window::Open() - 4.");
	}

	glfwGetMonitorPos(monitor, &monitorX, &monitorY);
	glfwSetWindowPos(Window::Interface, monitorX + ((videoMode->width - WINDOW_WIDTH) >> 1), monitorY + ((videoMode->height - WINDOW_HEIGHT) >> 1));
	glfwMakeContextCurrent(Window::Interface);
	glfwSwapInterval(1);

	if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == GLFW_FALSE)
	{
		glfwDestroyWindow(Window::Interface);
		glfwTerminate();
		Window::Interface = nullptr;

		return Logger::SaveMessage("Error: Window::Open() - 5.");
	}

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return true;
}

bool Window::Update()
{
	if (Window::Interface == nullptr)
		return Logger::SaveMessage("Error: Window::Update() - 1.");

	else if (glfwWindowShouldClose(Window::Interface) == GLFW_TRUE)
	{
		Window::Close();
		return false;
	}

	glfwSwapBuffers(Window::Interface);
	glfwPollEvents();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (Window::Timer)
	{
		Window::Current = std::chrono::high_resolution_clock::now();
		Window::Timer = false;
	}

	else
	{
		Window::Previous = Window::Current;
		Window::Current = std::chrono::high_resolution_clock::now();
	}

	return true;
}

bool Window::Close()
{
	if (Window::Interface != nullptr)
		glfwDestroyWindow(Window::Interface);

	glfwTerminate();
	Window::Interface = nullptr;
	Window::Timer = true;

	return true;
}

float Window::DeltaTime(const float scalar)
{
	static uint64_t count = 0, maximum = 1000000;
	static double divisor = 1.0 / static_cast<double>(maximum);

	if (Window::Timer)
		return 0.0f;

	count = std::chrono::duration_cast<std::chrono::microseconds>(Window::Current - Window::Previous).count();

	if (count > maximum)
		return 0.0f;

	return static_cast<float>(static_cast<double>(scalar) * (static_cast<double>(count) * divisor));
}