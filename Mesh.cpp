#include "Logger.hpp"
#include "Mesh.hpp"

#include "Libraries/meshoptimizer/meshoptimizer.h"

Mesh::Mesh() : Vertices(std::vector<Vertex>()), Array(0), Buffer(0), Index(0)
{
}

Mesh::~Mesh()
{
	this->Vertices.clear();

	if (this->Array != 0 || this->Buffer != 0 || this->Index != 0)
	{
		glDeleteVertexArrays(1, &this->Array);
		glDeleteBuffers(1, &this->Buffer);
		glDeleteBuffers(1, &this->Index);

		this->Array = 0;
		this->Buffer = 0;
		this->Index = 0;
	}
}

Mesh::Mesh(const Mesh& model) : Vertices(model.Vertices), Array(model.Array), Buffer(model.Buffer), Index(model.Index)
{
}

Mesh& Mesh::operator=(const Mesh& model)
{
	if (this->Array != 0 || this->Buffer != 0 || this->Index != 0)
	{
		glDeleteVertexArrays(1, &this->Array);
		glDeleteBuffers(1, &this->Buffer);
		glDeleteBuffers(1, &this->Index);
	}

	this->Vertices = model.Vertices;
	this->Array = model.Array;
	this->Buffer = model.Buffer;
	this->Index = model.Index;

	return *this;
}

bool Mesh::AddVertex(const Vertex& vertex)
{
	if (this->Array != 0 || this->Buffer != 0 || this->Index != 0)
		return Logger::SaveMessage("Error: Model::AddVertex() - 1.");

	this->Vertices.push_back(vertex);
	return this->Vertices.back().Normalize();
}

bool Mesh::Create()
{
	std::vector<GLuint> indices;

	if (this->Vertices.empty())
		return Logger::SaveMessage("Error: Model::Create() - 1.");

	else if (this->Array != 0)
		return Logger::SaveMessage("Error: Model::Create() - 2.");

	else if (this->Buffer != 0)
		return Logger::SaveMessage("Error: Model::Create() - 3.");

	else if (this->Index != 0)
		return Logger::SaveMessage("Error: Model::Create() - 4.");

	indices.resize(this->Vertices.size());

	for (size_t index = 0; index < this->Vertices.size(); ++index)
		indices[index] = static_cast<GLuint>(index);

	glGenVertexArrays(1, &this->Array);
	glGenBuffers(1, &this->Buffer);
	glGenBuffers(1, &this->Index);
	glBindVertexArray(this->Array);
	glBindBuffer(GL_ARRAY_BUFFER, this->Buffer);
	glBufferData(GL_ARRAY_BUFFER, this->Vertices.size() * sizeof(Vertex), &this->Vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->Index);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Texture));
	glBindVertexArray(0);

	return true;
}

bool Mesh::Render(const Shader& shader, const bool renderFrame) const
{
	if (this->Array == 0)
		return Logger::SaveMessage("Error: Model::Render() - 1.");

	else if (this->Buffer == 0)
		return Logger::SaveMessage("Error: Model::Render() - 2.");

	else if (this->Index == 0)
		return Logger::SaveMessage("Error: Model::Render() - 3.");

	else if (!shader.Use())
		return Logger::SaveMessage("Error: Model::Render() - 4.");

	if (renderFrame)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_TRIANGLES);

	glBindVertexArray(this->Array);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->Vertices.size()), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	return true;
}

bool Mesh::Destroy()
{
	Vertices.clear();

	if (this->Array != 0 || this->Buffer != 0 || this->Index != 0)
	{
		glDeleteVertexArrays(1, &this->Array);
		glDeleteBuffers(1, &this->Buffer);
		glDeleteBuffers(1, &this->Index);

		this->Array = 0;
		this->Buffer = 0;
		this->Index = 0;
	}

	return true;
}