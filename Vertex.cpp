#include "Logger.hpp"
#include "Vertex.hpp"

glm::vec3 Vertex::Left = glm::vec3(-1.0f, 0.0f, 0.0f);
glm::vec3 Vertex::Right = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 Vertex::Down = glm::vec3(0.0f, -1.0f, 0.0f);
glm::vec3 Vertex::Up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 Vertex::In = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 Vertex::Out = glm::vec3(0.0f, 0.0f, 1.0f);

Vertex::Vertex() : Position(glm::vec3(0.0f)), Normal(glm::vec3(0.0f)), Texture(glm::vec2(0.0f))
{
}

Vertex::~Vertex()
{
	this->Reset();
}

Vertex::Vertex(const Vertex& vertex) : Position(vertex.Position), Normal(vertex.Normal), Texture(vertex.Texture)
{
}

Vertex::Vertex(Vertex&& vertex) noexcept : Position(vertex.Position), Normal(vertex.Normal), Texture(vertex.Texture)
{
	vertex.Reset();
}

Vertex::Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& texture) : Position(position), Normal(normal), Texture(texture)
{
}

Vertex& Vertex::operator=(const Vertex& vertex)
{
	this->Position = vertex.Position;
	this->Normal = vertex.Normal;
	this->Texture = vertex.Texture;

	return *this;
}

Vertex& Vertex::operator=(Vertex&& vertex) noexcept
{
	this->Position = vertex.Position;
	this->Normal = vertex.Normal;
	this->Texture = vertex.Texture;
	vertex.Reset();

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

bool Vertex::Reset()
{
	this->Position = glm::vec3(0.0f);
	this->Normal = glm::vec3(0.0f);
	this->Texture = glm::vec2(0.0f);

	return true;
}