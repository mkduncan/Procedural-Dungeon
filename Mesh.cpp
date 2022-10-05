#include "Logger.hpp"
#include "Mesh.hpp"

#include "Libraries/meshoptimizer/meshoptimizer.h"

Mesh::Mesh() : Vertices(std::vector<Vertex>()), Array(0), Buffer(0), Index(0)
{
}

Mesh::~Mesh()
{
	this->Destroy();
}

Mesh::Mesh(const Mesh& mesh) : Vertices(mesh.Vertices), Array(mesh.Array), Buffer(mesh.Buffer), Index(mesh.Index)
{
}

Mesh::Mesh(Mesh&& mesh) noexcept : Vertices(std::move(mesh.Vertices)), Array(mesh.Array), Buffer(mesh.Buffer), Index(mesh.Index)
{
	mesh.Array = 0;
	mesh.Buffer = 0;
	mesh.Index = 0;
}

Mesh& Mesh::operator=(const Mesh& mesh)
{
	if ((this->Array != mesh.Array && this->Buffer != mesh.Buffer && this->Index != mesh.Index) || (this->Array == 0 && this->Buffer == 0 && this->Index == 0 && mesh.Array == 0 && mesh.Buffer == 0 && mesh.Index == 0))
	{
		this->Destroy();
		this->Vertices = mesh.Vertices;
		this->Array = mesh.Array;
		this->Buffer = mesh.Buffer;
		this->Index = mesh.Index;
	}

	return *this;
}

Mesh& Mesh::operator=(Mesh&& mesh) noexcept
{
	if (this->Array != mesh.Array && this->Buffer != mesh.Buffer && this->Index != mesh.Index || (this->Array == 0 && this->Buffer == 0 && this->Index == 0 && mesh.Array == 0 && mesh.Buffer == 0 && mesh.Index == 0))
	{
		this->Destroy();
		this->Vertices = std::move(mesh.Vertices);
		this->Array = mesh.Array;
		this->Buffer = mesh.Buffer;
		this->Index = mesh.Index;
	}

	mesh.Vertices.clear();
	mesh.Array = 0;
	mesh.Buffer = 0;
	mesh.Index = 0;

	return *this;
}

bool Mesh::AddVertex(const Vertex& vertex)
{
	if (this->Array != 0 || this->Buffer != 0 || this->Index != 0)
		return Logger::SaveMessage("Error: Mesh::AddVertex() - 1.");

	this->Vertices.push_back(vertex);

	if (!this->Vertices.back().Normalize())
	{
		this->Vertices.pop_back();
		return Logger::SaveMessage("Error: Mesh::AddVertex() - 2.");
	}

	return true;
}

bool Mesh::Create()
{
	std::vector<GLuint> indices;

	if (this->Vertices.empty())
		return Logger::SaveMessage("Error: Mesh::Create() - 1.");

	else if (this->Array != 0)
		return Logger::SaveMessage("Error: Mesh::Create() - 2.");

	else if (this->Buffer != 0)
		return Logger::SaveMessage("Error: Mesh::Create() - 3.");

	else if (this->Index != 0)
		return Logger::SaveMessage("Error: Mesh::Create() - 4.");

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
		return Logger::SaveMessage("Error: Mesh::Render() - 1.");

	else if (this->Buffer == 0)
		return Logger::SaveMessage("Error: Mesh::Render() - 2.");

	else if (this->Index == 0)
		return Logger::SaveMessage("Error: Mesh::Render() - 3.");

	else if (!shader.Use())
		return Logger::SaveMessage("Error: Mesh::Render() - 4.");

	if (renderFrame)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_TRIANGLES);

	glBindVertexArray(this->Array);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->Vertices.size()), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	return true;
}

bool Mesh::RenderIndexed(const Shader& shader, const std::vector<size_t>& treeIndices, const bool renderFrame) const
{
	std::vector<GLuint> indices;

	if (treeIndices.empty())
		return true;

	else if (this->Array == 0)
		return Logger::SaveMessage("Error: Mesh::RenderIndexed() - 1.");

	else if (this->Buffer == 0)
		return Logger::SaveMessage("Error: Mesh::RenderIndexed() - 2.");

	else if (this->Index == 0)
		return Logger::SaveMessage("Error: Mesh::RenderIndexed() - 3.");

	else if (!shader.Use())
		return Logger::SaveMessage("Error: Mesh::RenderIndexed() - 4.");

	if (renderFrame)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_TRIANGLES);

	for (size_t index = 0; index < treeIndices.size(); ++index)
		for (size_t offset = 0; offset < 24; ++offset)
			indices.push_back(static_cast<GLuint>(treeIndices[index] + offset));

	glBindVertexArray(this->Array);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), &indices[0]);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
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