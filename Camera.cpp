#include "Camera.hpp"
#include "Decoder.hpp"
#include "Logger.hpp"
#include "Window.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include <chrono>
#include <random>

#define MOVEMENT_SPEED 4.0f
#define SPEED_MULTIPLIER 4.0f
#define LOOK_SPEED 36.0f

//	Generates a high quality pseudo-random number.
static size_t NextRandomIndex(size_t, size_t);

glm::mat4 Camera::Projection(glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 32.0f)), Camera::View(glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f))), Camera::Model = glm::mat4(1.0f);
glm::vec3 Camera::Position(0.0f), Camera::Right(1.0f, 0.0f, 0.0f), Camera::Up(0.0f, 1.0f, 0.0f), Camera::Front(0.0f, 0.0f, -1.0f);
double Camera::Mouse[2] = { -1.0 };
float Camera::Pitch = 0.0f, Camera::Yaw = -90.0f;
bool Camera::Click[2] = { false };

bool Camera::StartPosition()
{
	//TODO: Synchronise this with the definition used in "Dungeon.cpp"
	static const float meshScale = 1.33333333f;
	size_t indexX = 0, indexY = 0;

	if (Decoder::Cells.empty())
		return Logger::SaveMessage(std::string("Error: Cammera::StartPosition() - 1"));

	Camera::Yaw = 0.0f;

	for (size_t tries = 0; tries < 1048576; ++tries)
	{
		indexX = NextRandomIndex(Decoder::Cells.size() >> 2, (Decoder::Cells.size() >> 1) + (Decoder::Cells.size() >> 2));
		indexY = NextRandomIndex(Decoder::Cells.front().size() >> 2, (Decoder::Cells.front().size() >> 1) + (Decoder::Cells.front().size() >> 2));

		if (Decoder::Cells[indexX][indexY] == OPENED_TILE)
		{
			//TODO: Initial y-axis camera orientation / rotation is incorrect due to inversion of row / column cell ordering.
			if (Decoder::Cells[indexX - 1][indexY] == OPENED_TILE && Decoder::Cells[indexX - 2][indexY] == OPENED_TILE)
			{
				Camera::Yaw = -180.0f;
				break;
			}

			if (Decoder::Cells[indexX + 1][indexY] == OPENED_TILE && Decoder::Cells[indexX + 2][indexY] == OPENED_TILE)
			{
				Camera::Yaw = 180.0f;
				break;
			}

			if (Decoder::Cells[indexX][indexY - 1] == OPENED_TILE && Decoder::Cells[indexX][indexY - 2] == OPENED_TILE)
			{
				Camera::Yaw = -90.0f;
				break;
			}

			if (Decoder::Cells[indexX][indexY + 1] == OPENED_TILE && Decoder::Cells[indexX][indexY + 2] == OPENED_TILE)
			{
				Camera::Yaw = 90.0f;
				break;
			}
		}
	}

	if (Camera::Yaw < 0.0001f && Camera::Yaw > -0.0001f)
	{
		Camera::Yaw = -90.0f;
		return Logger::SaveMessage(std::string("Error: Cammera::StartPosition() - 2"));
	}

	Camera::Position = glm::vec3((static_cast<float>(indexX) + 0.5f) * meshScale, 0.0f, (static_cast<float>(indexY) + 0.5f) * meshScale);
	Camera::Front = glm::normalize(glm::vec3(glm::cos(glm::radians(Camera::Yaw)) * glm::cos(glm::radians(Camera::Pitch)), glm::sin(glm::radians(Camera::Pitch)), glm::sin(glm::radians(Camera::Yaw)) * glm::cos(glm::radians(Camera::Pitch))));
	Camera::Right = glm::normalize(glm::cross(Camera::Front, glm::vec3(0.0f, 1.0f, 0.0f)));
	Camera::Up = glm::normalize(glm::cross(Camera::Right, Camera::Front));
	Camera::View = glm::lookAt(Camera::Position, Camera::Position + Camera::Front, Camera::Up);

	return true;
}

bool Camera::Update(const bool lockY, const bool lockSpeed)
{
	double newMouse[2] = { Camera::Mouse[0], Camera::Mouse[1] };
	float movementScalar = 1.0f;

	if (Window::Interface == nullptr)
		return Logger::SaveMessage(std::string("Error: Camera::Update() - 1"));

	if (Camera::Mouse[0] < 0.0 && Camera::Mouse[1] < 0.0)
	{
		glfwGetCursorPos(Window::Interface, &Camera::Mouse[0], &Camera::Mouse[1]);
		Camera::Click[0] = (glfwGetMouseButton(Window::Interface, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);

		return true;
	}

	Camera::Click[1] = Camera::Click[0];
	glfwGetCursorPos(Window::Interface, &Camera::Mouse[0], &Camera::Mouse[1]);
	Camera::Click[0] = (glfwGetMouseButton(Window::Interface, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);

	if (!lockSpeed && (glfwGetKey(Window::Interface, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(Window::Interface, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS))
		movementScalar = SPEED_MULTIPLIER;

	if (lockY)
	{
		if (glfwGetKey(Window::Interface, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(Window::Interface, GLFW_KEY_LEFT) == GLFW_PRESS)
			Camera::Position -= glm::normalize(glm::vec3(Camera::Right.x, 0.0f, Camera::Right.z)) * Window::DeltaTime(MOVEMENT_SPEED * movementScalar);

		if (glfwGetKey(Window::Interface, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(Window::Interface, GLFW_KEY_RIGHT) == GLFW_PRESS)
			Camera::Position += glm::normalize(glm::vec3(Camera::Right.x, 0.0f, Camera::Right.z)) * Window::DeltaTime(MOVEMENT_SPEED * movementScalar);

		if (glfwGetKey(Window::Interface, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(Window::Interface, GLFW_KEY_UP) == GLFW_PRESS)
			Camera::Position += glm::normalize(glm::vec3(Camera::Front.x, 0.0f, Camera::Front.z)) * Window::DeltaTime(MOVEMENT_SPEED * movementScalar);

		if (glfwGetKey(Window::Interface, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(Window::Interface, GLFW_KEY_DOWN) == GLFW_PRESS)
			Camera::Position -= glm::normalize(glm::vec3(Camera::Front.x, 0.0f, Camera::Front.z)) * Window::DeltaTime(MOVEMENT_SPEED * movementScalar);

		if (lockY)
			Camera::Position.y = 0.0f;
	}

	else
	{
		if (glfwGetKey(Window::Interface, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(Window::Interface, GLFW_KEY_LEFT) == GLFW_PRESS)
			Camera::Position -= Camera::Right * Window::DeltaTime(MOVEMENT_SPEED * movementScalar);

		if (glfwGetKey(Window::Interface, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(Window::Interface, GLFW_KEY_RIGHT) == GLFW_PRESS)
			Camera::Position += Camera::Right * Window::DeltaTime(MOVEMENT_SPEED * movementScalar);

		if (glfwGetKey(Window::Interface, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(Window::Interface, GLFW_KEY_UP) == GLFW_PRESS)
			Camera::Position += Camera::Front * Window::DeltaTime(MOVEMENT_SPEED * movementScalar);

		if (glfwGetKey(Window::Interface, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(Window::Interface, GLFW_KEY_DOWN) == GLFW_PRESS)
			Camera::Position -= Camera::Front * Window::DeltaTime(MOVEMENT_SPEED * movementScalar);
	}

	if (Camera::Click[0] && Camera::Click[1])
	{
		Camera::Yaw -= static_cast<float>(newMouse[0] - Camera::Mouse[0]) * Window::DeltaTime(LOOK_SPEED);
		Camera::Pitch += static_cast<float>(newMouse[1] - Camera::Mouse[1]) * Window::DeltaTime(LOOK_SPEED);

		if (Camera::Pitch > 89.0f)
			Camera::Pitch = 89.0f;

		if (Camera::Pitch < -89.0f)
			Camera::Pitch = -89.0f;
	}

	Camera::Front = glm::normalize(glm::vec3(glm::cos(glm::radians(Camera::Yaw)) * glm::cos(glm::radians(Camera::Pitch)), glm::sin(glm::radians(Camera::Pitch)), glm::sin(glm::radians(Camera::Yaw)) * glm::cos(glm::radians(Camera::Pitch))));
	Camera::Right = glm::normalize(glm::cross(Camera::Front, glm::vec3(0.0f, 1.0f, 0.0f)));
	Camera::Up = glm::normalize(glm::cross(Camera::Right, Camera::Front));
	Camera::View = glm::lookAt(Camera::Position, Camera::Position + Camera::Front, Camera::Up);

	return true;
}

bool Camera::SetWorld(const Shader& shader)
{
	if (!shader.SetMatrix(std::string("projection"), Camera::Projection) || !shader.SetMatrix(std::string("view"), Camera::View) || !shader.SetMatrix(std::string("model"), Camera::Model))
		return Logger::SaveMessage(std::string("Error: Camera::SetWorld() - 1"));

	return true;
}

static size_t NextRandomIndex(size_t minimum, size_t maximum)
{
	static std::mt19937_64 generator(std::chrono::high_resolution_clock::now().time_since_epoch().count());
	std::uniform_int_distribution<size_t> distribution(minimum, maximum);

	return distribution(generator);
}