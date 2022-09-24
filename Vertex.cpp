#include "Logger.hpp"
#include "Vertex.hpp"

Vertex::Vertex() : Position(glm::vec3(0.0f, 0.0f, 0.0f)), Normal(glm::vec3(0.0f, 0.0f, 0.0f)), Texture(glm::vec2(0.0f, 0.0f))
{
}

Vertex::~Vertex()
{
	this->Position = glm::vec3(0.0f, 0.0f, 0.0f);
	this->Normal = glm::vec3(0.0f, 0.0f, 0.0f);
	this->Texture = glm::vec2(0.0f, 0.0f);
}

Vertex::Vertex(const Vertex &vertex) : Position(vertex.Position), Normal(vertex.Normal), Texture(vertex.Texture)
{
}

Vertex::Vertex(const glm::vec3 &position, const glm::vec3 &normal, const glm::vec2 &texture) : Position(position), Normal(normal), Texture(texture)
{
}

Vertex& Vertex::operator=(const Vertex &vertex)
{
	this->Position = vertex.Position;
	this->Normal = vertex.Normal;
	this->Texture = vertex.Texture;

	return *this;
}

bool Vertex::Normalize()
{
	float magnitude = glm::length(this->Normal);

	if (magnitude < 0.0001f)
		return Logger::SaveMessage("Error: Vertex::Normalize() - 1.");

	magnitude = 1.0f / magnitude;
	this->Normal *= magnitude;

	return true;
}