#include "Logger.hpp"
#include "Plane.hpp"

Plane::Plane() : Direction(glm::vec3(0.0f)), Distance(0.0f)
{
}

Plane::~Plane()
{
	this->Reset();
}

Plane::Plane(const glm::vec4& row) : Direction(glm::vec3(row.x, row.y, row.z)), Distance(row.w)
{
}

Plane::Plane(const Plane& plane) : Direction(plane.Direction), Distance(plane.Distance)
{
}

Plane::Plane(Plane&& plane) noexcept : Direction(plane.Direction), Distance(plane.Distance)
{
	plane.Reset();
}

Plane& Plane::operator=(const Plane& plane)
{
	this->Direction = plane.Direction;
	this->Distance = plane.Distance;

	return *this;
}

Plane& Plane::operator=(Plane&& plane) noexcept
{
	this->Direction = plane.Direction;
	this->Distance = plane.Distance;
	plane.Reset();

	return *this;
}

float Plane::Classify(const glm::vec3& point) const
{
	return glm::dot(this->Direction, point) + this->Distance;
}

bool Plane::Normalize()
{
	float magnitude = glm::length(this->Direction);

	if (magnitude < 0.0001f)
		return Logger::SaveMessage("Error: Plane::Normalize() - 1.");

	magnitude = 1.0f / magnitude;
	this->Direction *= magnitude;
	this->Distance *= magnitude;

	return true;
}

bool Plane::Reset()
{
	this->Direction = glm::vec3(0.0f);
	this->Distance = 0.0f;

	return true;
}