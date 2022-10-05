#ifndef Vertex_H
#define Vertex_H

#include "glm/glm.hpp"

class Vertex
{
public:
	static glm::vec3 Left, Right, Down, Up, In, Out;

	glm::vec3 Position, Normal;
	glm::vec2 Texture;

	Vertex();
	~Vertex();
	Vertex(const Vertex&);
	Vertex(Vertex&&) noexcept;
	Vertex(const glm::vec3&, const glm::vec3&, const glm::vec2&);
	Vertex& operator=(const Vertex&);
	Vertex& operator=(Vertex&&) noexcept;

	bool Normalize();
	bool Reset();
};

#endif