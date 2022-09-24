#ifndef Mesh_H
#define Mesh_H

#include <vector>

#include "glad/glad.h"
#include "Shader.hpp"
#include "Vertex.hpp"

class Mesh
{
public:
	std::vector<Vertex> Vertices;
	GLuint Array, Buffer, Index;

	Mesh();
	~Mesh();
	Mesh(const Mesh&);
	Mesh(Mesh&&) noexcept = default;
	Mesh& operator=(const Mesh&);
	Mesh& operator=(Mesh&&) noexcept = default;

	bool AddVertex(const Vertex&);
	bool Create();
	bool Render(const Shader&, const bool = false) const;
	bool Destroy();
};

#endif