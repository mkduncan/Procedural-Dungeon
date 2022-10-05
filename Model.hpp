#ifndef Model_H
#define Model_H

#include "MeshObject.hpp"

class Model
{
public:
	std::vector<MeshObject> Meshes;
	glm::mat4 Transform;

	Model();
	~Model();
	Model(const Model&);
	Model(Model&&) noexcept;
	Model& operator=(const Model&);
	Model& operator=(Model&&) noexcept;

	bool LoadModel(const std::string&, const bool = true);
	bool LoadMesh(MeshObject&);
	bool Render() const;
	bool RenderIndexed(const std::vector<size_t>&) const;
	bool Destroy();
};

#endif