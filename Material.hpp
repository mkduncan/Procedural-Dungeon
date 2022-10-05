#ifndef Material_H
#define Material_H

#include "Shader.hpp"
#include "glm/glm.hpp"

class Material
{
public:
	std::string DiffuseTexture, SpecularTexture;
	glm::vec3 Ambient, Diffuse, Specular;
	float Shininess;

	Material();
	~Material();
	Material(const Material&);
	Material(Material&&) noexcept;
	Material& operator=(const Material&);
	Material& operator=(Material&&) noexcept;

	bool Load(const Shader&) const;
	bool Reset();
};

#endif