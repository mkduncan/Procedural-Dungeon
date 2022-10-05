#ifndef Plane_H
#define Plane_H

#include "Libraries/glm/glm.hpp"

class Plane
{
public:
	glm::vec3 Direction;
	float Distance;

	Plane();
	~Plane();
	Plane(const glm::vec4&);
	Plane(const Plane&);
	Plane(Plane&&) noexcept;
	Plane& operator=(const Plane&);
	Plane& operator=(Plane&&) noexcept;

	float Classify(const glm::vec3&) const;
	bool Normalize();
	bool Reset();
};

#endif