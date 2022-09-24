#include "Material.hpp"

Material::Material() : Ambient(glm::vec3(0.0f, 0.0f, 0.0f)), Diffuse(glm::vec3(0.0f, 0.0f, 0.0f)), Specular(glm::vec3(0.0f, 0.0f, 0.0f)), Shininess(0.0f)
{
}

Material::~Material()
{
	this->Ambient = glm::vec3(0.0f, 0.0f, 0.0f);
	this->Diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
	this->Specular = glm::vec3(0.0f, 0.0f, 0.0f);
	this->Shininess = 0.0f;
}

Material::Material(const Material &material) : Ambient(material.Ambient), Diffuse(material.Diffuse), Specular(material.Specular), Shininess(material.Shininess)
{
}

Material::Material(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, const float shininess) : Ambient(ambient), Diffuse(diffuse), Specular(specular), Shininess(shininess)
{
}

Material& Material::operator=(const Material &material)
{
	this->Ambient = material.Ambient;
	this->Diffuse = material.Diffuse;
	this->Specular = material.Specular;
	this->Shininess = material.Shininess;

	return *this;
}