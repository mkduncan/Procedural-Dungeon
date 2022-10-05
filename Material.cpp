#include "Logger.hpp"
#include "Material.hpp"

Material::Material() : DiffuseTexture(std::string()), SpecularTexture(std::string()), Ambient(glm::vec3(0.0f)), Diffuse(glm::vec3(0.0f)), Specular(glm::vec3(0.0f)), Shininess(0.0f)
{
}

Material::~Material()
{
	this->Reset();
}

Material::Material(const Material& material) : DiffuseTexture(material.DiffuseTexture), SpecularTexture(material.SpecularTexture), Ambient(material.Ambient), Diffuse(material.Diffuse), Specular(material.Specular), Shininess(material.Shininess)
{
}

Material::Material(Material&& material) noexcept : DiffuseTexture(std::move(material.DiffuseTexture)), SpecularTexture(std::move(material.SpecularTexture)), Ambient(material.Ambient), Diffuse(material.Diffuse), Specular(material.Specular), Shininess(material.Shininess)
{
	material.Reset();
}

Material& Material::operator=(const Material& material)
{
	this->DiffuseTexture = material.DiffuseTexture;
	this->SpecularTexture = material.SpecularTexture;
	this->Ambient = material.Ambient;
	this->Diffuse = material.Diffuse;
	this->Specular = material.Specular;
	this->Shininess = material.Shininess;

	return *this;
}

Material& Material::operator=(Material&& material) noexcept
{
	this->DiffuseTexture = std::move(material.DiffuseTexture);
	this->SpecularTexture = std::move(material.SpecularTexture);
	this->Ambient = material.Ambient;
	this->Diffuse = material.Diffuse;
	this->Specular = material.Specular;
	this->Shininess = material.Shininess;
	material.Reset();

	return *this;
}

bool Material::Load(const Shader& shader) const
{
	if (!shader.Use())
		return Logger::SaveMessage("Error: Material::Load() - 0.");

	else if (!shader.SetInteger("material.diffuse", 0))
		return Logger::SaveMessage("Error: Material::Load() - 1.");

	else if (!shader.SetInteger("material.specular", 1))
		return Logger::SaveMessage("Error: Material::Load() - 2.");

	else if (!shader.SetFloat("material.shininess", this->Shininess))
		return Logger::SaveMessage("Error: MeshObject::Render() - 3.");

	return true;
}

bool Material::Reset()
{
	this->DiffuseTexture.clear();
	this->SpecularTexture.clear();
	this->Ambient = glm::vec3(0.0f);
	this->Diffuse = glm::vec3(0.0f);
	this->Specular = glm::vec3(0.0f);
	this->Shininess = 0.0f;

	return true;
}