#ifndef MeshObject_H
#define MeshObject_H

#include "Material.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

class MeshObject
{
public:
	Mesh SubMesh;
	Texture DiffuseMap, SpecularMap;
	Material SubMaterial;
	Shader SubShader;
	std::string VertexShader, FragmentShader;
	bool LoadShaderFiles;

	MeshObject();
	~MeshObject();
	MeshObject(const MeshObject&);
	MeshObject(MeshObject&&) noexcept;
	MeshObject& operator=(const MeshObject&);
	MeshObject& operator=(MeshObject&&) noexcept;

	bool LoadMesh(Mesh&);
	bool LoadTextures(const std::string&, const std::string & = std::string());
	bool LoadMaterial(const Material&);
	bool LoadMaterial(const glm::vec3&, const glm::vec3&, const glm::vec3&, const float);
	bool LoadShaders(const std::string&, const std::string&, const bool = false);
	bool Create();
	bool Render(const glm::mat4&) const;
	bool RenderIndexed(const glm::mat4&, const std::vector<size_t>&) const;
	bool Destroy();
};

#endif