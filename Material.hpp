#ifndef Material_H
#define Material_H

#include "glm/glm.hpp"

class Material
{
private:
	glm::vec3 Ambient, Diffuse, Specular;
	float Shininess;

public:
	Material();
	~Material();
	Material(const Material&);
	Material(Material&&) noexcept = default;
	Material(const glm::vec3&, const glm::vec3&, const glm::vec3&, const float);
	Material& operator=(const Material&);
	Material& operator=(Material&&) noexcept = default;
};

#endif