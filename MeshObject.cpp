#include "Camera.hpp"
#include "Logger.hpp"
#include "MeshObject.hpp"

MeshObject::MeshObject() : SubMesh(Mesh()), DiffuseMap(Texture()), SpecularMap(Texture()), SubMaterial(Material()), SubShader(Shader()), VertexShader(std::string()), FragmentShader(std::string()), LoadShaderFiles(false)
{
}

MeshObject::~MeshObject()
{
	this->Destroy();
}

MeshObject::MeshObject(const MeshObject& mesh) : SubMesh(mesh.SubMesh), DiffuseMap(mesh.DiffuseMap), SpecularMap(mesh.SpecularMap), SubMaterial(mesh.SubMaterial), SubShader(mesh.SubShader), VertexShader(mesh.VertexShader), FragmentShader(mesh.FragmentShader), LoadShaderFiles(mesh.LoadShaderFiles)
{
}

MeshObject::MeshObject(MeshObject&& mesh) noexcept : SubMesh(std::move(mesh.SubMesh)), DiffuseMap(std::move(mesh.DiffuseMap)), SpecularMap(std::move(mesh.SpecularMap)), SubMaterial(std::move(mesh.SubMaterial)), SubShader(std::move(mesh.SubShader)), VertexShader(std::move(mesh.VertexShader)), FragmentShader(std::move(mesh.FragmentShader)), LoadShaderFiles(mesh.LoadShaderFiles)
{
	mesh.Destroy();
}

MeshObject& MeshObject::operator=(const MeshObject& mesh)
{
	this->SubMesh = mesh.SubMesh;
	this->DiffuseMap = mesh.DiffuseMap;
	this->SpecularMap = mesh.SpecularMap;
	this->SubMaterial = mesh.SubMaterial;
	this->SubShader = mesh.SubShader;
	this->VertexShader = mesh.VertexShader;
	this->FragmentShader = mesh.FragmentShader;
	this->LoadShaderFiles = mesh.LoadShaderFiles;

	return *this;
}

MeshObject& MeshObject::operator=(MeshObject&& mesh) noexcept
{
	this->SubMesh = std::move(mesh.SubMesh);
	this->DiffuseMap = std::move(mesh.DiffuseMap);
	this->SpecularMap = std::move(mesh.SpecularMap);
	this->SubMaterial = std::move(mesh.SubMaterial);
	this->SubShader = std::move(mesh.SubShader);
	this->VertexShader = std::move(mesh.VertexShader);
	this->FragmentShader = std::move(mesh.FragmentShader);
	this->LoadShaderFiles = mesh.LoadShaderFiles;
	mesh.Destroy();

	return *this;
}

bool MeshObject::LoadMesh(Mesh& mesh)
{
	if (mesh.Vertices.empty())
		return Logger::SaveMessage("Error: MeshObject::LoadMesh() - 1.");

	this->SubMesh = std::move(mesh);
	mesh.Destroy();

	return true;
}

bool MeshObject::LoadTextures(const std::string& diffuseMap, const std::string& specularMap)
{
	this->SubMaterial.DiffuseTexture = diffuseMap;
	this->SubMaterial.SpecularTexture = specularMap;

	return true;
}

bool MeshObject::LoadMaterial(const Material& material)
{
	this->SubMaterial = material;
	return true;
}

bool MeshObject::LoadMaterial(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, const float shininess)
{
	this->SubMaterial.Ambient = ambient;
	this->SubMaterial.Diffuse = diffuse;
	this->SubMaterial.Specular = specular;
	this->SubMaterial.Shininess = shininess;

	return true;
}

bool MeshObject::LoadShaders(const std::string& vertex, const std::string& fragment, const bool loadFiles)
{
	this->VertexShader = vertex;
	this->FragmentShader = fragment;
	this->LoadShaderFiles = loadFiles;

	return true;
}

bool MeshObject::Create()
{
	if (this->LoadShaderFiles && !this->SubShader.Load(this->VertexShader, this->FragmentShader))
		return Logger::SaveMessage("Error: MeshObject::Create() - 1.");

	else if (!this->LoadShaderFiles && !this->SubShader.Create(this->VertexShader, this->FragmentShader))
		return Logger::SaveMessage("Error: MeshObject::Create() - 2.");

	else if (!this->DiffuseMap.Load(this->SubMaterial.DiffuseTexture))
		return Logger::SaveMessage("Error: MeshObject::Create() - 3.");

	else if (!this->SubMaterial.SpecularTexture.empty() && !this->SpecularMap.Load(this->SubMaterial.SpecularTexture))
	{
		this->DiffuseMap.Destroy();
		return Logger::SaveMessage("Error: MeshObject::Create() - 4.");
	}

	else if (!this->SubMesh.Create())
		return Logger::SaveMessage("Error: MeshObject::Create() - 5.");

	return true;
}

bool MeshObject::Render(const glm::mat4& transform) const
{
	if (!Camera::SetWorld(this->SubShader))
		return Logger::SaveMessage("Error: MeshObject::Render() - 1.");

	else if (!this->DiffuseMap.Use())
		return Logger::SaveMessage("Error: MeshObject::Render() - 2.");

	else if (this->SpecularMap.Id != 0 && !this->SpecularMap.Use(1))
		return Logger::SaveMessage("Error: MeshObject::Render() - 3.");

	else if (!this->SubShader.Use())
		return Logger::SaveMessage("Error: MeshObject::Render() - 4.");

	else if (!this->SubShader.SetInteger("material.diffuse", 0))
		return Logger::SaveMessage("Error: MeshObject::Render() - 5.");

	else if (!this->SubShader.SetInteger("material.specular", 1))
		return Logger::SaveMessage("Error: MeshObject::Render() - 6.");

	else if (!this->SubShader.SetVector("light.position", Camera::Position))
		return Logger::SaveMessage("Error: MeshObject::Render() - 7.");

	else if (!this->SubShader.SetVector("light.direction", Camera::Front))
		return Logger::SaveMessage("Error: MeshObject::Render() - 8.");

	else if (!this->SubShader.SetFloat("light.cutOff", glm::cos(glm::radians(30.0f))))
		return Logger::SaveMessage("Error: MeshObject::Render() - 9.");

	else if (!this->SubShader.SetFloat("light.outerCutOff", glm::cos(glm::radians(45.0f))))
		return Logger::SaveMessage("Error: MeshObject::Render() - 10.");

	else if (!this->SubShader.SetFloat("light.constant", 1.0f))
		return Logger::SaveMessage("Error: MeshObject::Render() - 11.");

	else if (!this->SubShader.SetFloat("light.linear", 0.14f))
		return Logger::SaveMessage("Error: MeshObject::Render() - 12.");

	else if (!this->SubShader.SetFloat("light.quadratic", 0.07f))
		return Logger::SaveMessage("Error: MeshObject::Render() - 13.");

	else if (!this->SubShader.SetVector("viewPos", Camera::Position))
		return Logger::SaveMessage("Error: MeshObject::Render() - 14.");

	else if (!this->SubShader.SetVector("light.ambient", this->SubMaterial.Ambient))
		return Logger::SaveMessage("Error: MeshObject::Render() - 15.");

	else if (!this->SubShader.SetVector("light.diffuse", this->SubMaterial.Diffuse))
		return Logger::SaveMessage("Error: MeshObject::Render() - 16.");

	else if (!this->SubShader.SetVector("light.specular", this->SubMaterial.Specular))
		return Logger::SaveMessage("Error: MeshObject::Render() - 17.");

	else if (!this->SubShader.SetFloat("material.shininess", this->SubMaterial.Shininess))
		return Logger::SaveMessage("Error: MeshObject::Render() - 18.");

	else if (!this->SubShader.SetMatrix(std::string("projection"), Camera::Projection))
		return Logger::SaveMessage("Error: MeshObject::Render() - 19.");

	else if (!this->SubShader.SetMatrix(std::string("view"), Camera::View))
		return Logger::SaveMessage("Error: MeshObject::Render() - 20.");

	else if (!this->SubShader.SetMatrix(std::string("model"), transform))
		return Logger::SaveMessage("Error: MeshObject::Render() - 21.");

	else if (!this->SubMesh.Render(this->SubShader))
		return Logger::SaveMessage("Error: MeshObject::Render() - 22.");

	return true;
}

bool MeshObject::RenderIndexed(const glm::mat4 &transform, const std::vector<size_t> &indices) const
{
	if (!Camera::SetWorld(this->SubShader))
		return Logger::SaveMessage("Error: MeshObject::Render() - 1.");

	else if (!this->DiffuseMap.Use())
		return Logger::SaveMessage("Error: MeshObject::Render() - 2.");

	else if (this->SpecularMap.Id != 0 && !this->SpecularMap.Use(1))
		return Logger::SaveMessage("Error: MeshObject::Render() - 3.");

	else if (!this->SubShader.Use())
		return Logger::SaveMessage("Error: MeshObject::Render() - 4.");

	else if (!this->SubShader.SetInteger("material.diffuse", 0))
		return Logger::SaveMessage("Error: MeshObject::Render() - 5.");

	else if (!this->SubShader.SetInteger("material.specular", 1))
		return Logger::SaveMessage("Error: MeshObject::Render() - 6.");

	else if (!this->SubShader.SetVector("light.position", Camera::Position))
		return Logger::SaveMessage("Error: MeshObject::Render() - 7.");

	else if (!this->SubShader.SetVector("light.direction", Camera::Front))
		return Logger::SaveMessage("Error: MeshObject::Render() - 8.");

	else if (!this->SubShader.SetFloat("light.cutOff", glm::cos(glm::radians(30.0f))))
		return Logger::SaveMessage("Error: MeshObject::Render() - 9.");

	else if (!this->SubShader.SetFloat("light.outerCutOff", glm::cos(glm::radians(45.0f))))
		return Logger::SaveMessage("Error: MeshObject::Render() - 10.");

	else if (!this->SubShader.SetFloat("light.constant", 1.0f))
		return Logger::SaveMessage("Error: MeshObject::Render() - 11.");

	else if (!this->SubShader.SetFloat("light.linear", 0.14f))
		return Logger::SaveMessage("Error: MeshObject::Render() - 12.");

	else if (!this->SubShader.SetFloat("light.quadratic", 0.07f))
		return Logger::SaveMessage("Error: MeshObject::Render() - 13.");

	else if (!this->SubShader.SetVector("viewPos", Camera::Position))
		return Logger::SaveMessage("Error: MeshObject::Render() - 14.");

	else if (!this->SubShader.SetVector("light.ambient", this->SubMaterial.Ambient))
		return Logger::SaveMessage("Error: MeshObject::Render() - 15.");

	else if (!this->SubShader.SetVector("light.diffuse", this->SubMaterial.Diffuse))
		return Logger::SaveMessage("Error: MeshObject::Render() - 16.");

	else if (!this->SubShader.SetVector("light.specular", this->SubMaterial.Specular))
		return Logger::SaveMessage("Error: MeshObject::Render() - 17.");

	else if (!this->SubShader.SetFloat("material.shininess", this->SubMaterial.Shininess))
		return Logger::SaveMessage("Error: MeshObject::Render() - 18.");

	else if (!this->SubShader.SetMatrix(std::string("projection"), Camera::Projection))
		return Logger::SaveMessage("Error: MeshObject::Render() - 19.");

	else if (!this->SubShader.SetMatrix(std::string("view"), Camera::View))
		return Logger::SaveMessage("Error: MeshObject::Render() - 20.");

	else if (!this->SubShader.SetMatrix(std::string("model"), transform))
		return Logger::SaveMessage("Error: MeshObject::Render() - 21.");

	else if (!this->SubMesh.RenderIndexed(this->SubShader, indices))
		return Logger::SaveMessage("Error: MeshObject::Render() - 22.");

	return true;
}

bool MeshObject::Destroy()
{
	this->SubMaterial = Material();
	this->VertexShader.clear();
	this->FragmentShader.clear();
	this->LoadShaderFiles = false;

	if (!this->SubMesh.Destroy())
	{
		this->DiffuseMap.Destroy();
		this->SpecularMap.Destroy();
		this->SubShader.Destroy();

		return Logger::SaveMessage("Error: MeshObject::Destroy() - 1.");
	}

	else if (!this->DiffuseMap.Destroy())
	{
		this->SpecularMap.Destroy();
		this->SubShader.Destroy();

		return Logger::SaveMessage("Error: MeshObject::Destroy() - 2.");
	}

	else if (this->SpecularMap.Id != 0 && !this->SpecularMap.Destroy())
	{
		this->SubShader.Destroy();
		return Logger::SaveMessage("Error: MeshObject::Destroy() - 3.");
	}

	else if (!this->SubShader.Destroy())
		return Logger::SaveMessage("Error: MeshObject::Destroy() - 4.");

	return true;
}