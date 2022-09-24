#ifndef Camera_H
#define Camera_H

#include <cstdint>

#include "glm/glm.hpp"
#include "Shader.hpp"

class Camera
{
private:
	Camera() = delete;
	~Camera() = delete;
	Camera(const Camera&) = delete;
	Camera(Camera&&) noexcept = delete;
	Camera& operator=(const Camera&) = delete;
	Camera& operator=(Camera&&) noexcept = delete;

public:
	static glm::mat4 Projection, View, Model;
	static glm::vec3 Position, Right, Up, Front;
	static double Mouse[2];
	static float Pitch, Yaw;
	static bool Click[2];

	static bool StartPosition();
	static bool Update(const bool = true, const bool = true);
	static bool SetWorld(const Shader&);
};

#endif