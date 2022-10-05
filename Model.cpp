#include "Logger.hpp"
#include "Model.hpp"
#include "StaticShader.hpp"

#include <fstream>
#include <map>
#include <sstream>

static std::map<std::string, Material> ParseMaterialFile(const std::string&, const std::string&);

Model::Model() : Meshes(std::vector<MeshObject>()), Transform(glm::mat4(1.0f))
{
}

Model::~Model()
{
	this->Destroy();
}

Model::Model(const Model& model) : Meshes(model.Meshes), Transform(model.Transform)
{
}

Model::Model(Model&& model) noexcept : Meshes(std::move(model.Meshes)), Transform(model.Transform)
{
	model.Destroy();
}

Model& Model::operator=(const Model& model)
{
	this->Meshes = model.Meshes;
	this->Transform = model.Transform;

	return *this;
}

Model& Model::operator=(Model&& model) noexcept
{
	this->Meshes = std::move(model.Meshes);
	this->Transform = model.Transform;
	model.Destroy();

	return *this;
}

bool Model::LoadModel(const std::string& modelFile, const bool sendToGPU)
{
	std::ifstream inputFile;
	std::stringstream inputBuffer, lineBuffer;
	std::map<std::string, MeshObject> meshMap;
	std::map<std::string, Material> materialMap;
	std::vector<glm::vec3> Vertices, Normals;
	std::vector<glm::vec2> Coordinates;
	std::string directoryPath, inputLine, inputToken, faceToken[3];
	std::map<std::string, MeshObject>::iterator meshIterator = meshMap.end();
	size_t faceIndex[3] = { 0 }, directoryIndex = modelFile.rfind('/');

	if (directoryIndex != std::string::npos)
		directoryPath = modelFile.substr(0, directoryIndex + 1);

	inputFile.open(modelFile);

	if (!inputFile)
		return Logger::SaveMessage("Error: Model::LoadModel() - 1.");

	inputBuffer << inputFile.rdbuf();
	inputFile.close();

	while (std::getline(inputBuffer, inputLine))
	{
		if (inputLine.empty())
			continue;

		lineBuffer = std::stringstream();
		lineBuffer << inputLine;
		lineBuffer >> inputToken;

		if (inputToken == "mtllib")
		{
			lineBuffer >> inputToken;

			if (!lineBuffer.eof() || lineBuffer.fail())
				return Logger::SaveMessage("Error: Model::LoadModel() - 2.");

			materialMap = ParseMaterialFile(directoryPath, inputToken);

			if (materialMap.empty())
				return Logger::SaveMessage("Error: Model::LoadModel() - 3.");
		}

		else if (inputToken == "usemtl")
		{
			lineBuffer >> inputToken;
			meshIterator = meshMap.find(inputToken);

			if (meshIterator == meshMap.end())
				meshIterator = meshMap.insert(meshMap.begin(), std::make_pair(inputToken, MeshObject()));
		}

		else if (inputToken == "v")
		{
			Vertices.emplace_back();
			lineBuffer >> Vertices.back().x >> Vertices.back().y >> Vertices.back().z;
		}

		else if (inputToken == "vn")
		{
			Normals.emplace_back();
			lineBuffer >> Normals.back().x >> Normals.back().y >> Normals.back().z;
		}

		else if (inputToken == "vt")
		{
			Coordinates.emplace_back();
			lineBuffer >> Coordinates.back().x >> Coordinates.back().y;
		}

		else if (inputToken == "f")
		{
			if (meshIterator == meshMap.end())
				return Logger::SaveMessage("Error: Model::LoadModel() - 4.");

			lineBuffer >> faceToken[0] >> faceToken[1] >> faceToken[2];

			for (size_t index = 0; index < 3; ++index)
			{
				lineBuffer = std::stringstream();
				lineBuffer << faceToken[index];

				for (size_t split = 0; split < 3; ++split)
				{
					std::getline(lineBuffer, inputToken, '/');
					faceIndex[split] = std::stoull(inputToken) - 1;
				}

				if (faceIndex[0] >= Vertices.size())
					return Logger::SaveMessage("Error: Model::LoadModel() - 6.");

				else if (faceIndex[2] >= Normals.size())
					return Logger::SaveMessage("Error: Model::LoadModel() - 7.");

				else if (faceIndex[1] >= Coordinates.size())
					return Logger::SaveMessage("Error: Model::LoadModel() - 8.");

				else if (!meshIterator->second.SubMesh.AddVertex(Vertex(Vertices[faceIndex[0]], Normals[faceIndex[2]], Coordinates[faceIndex[1]])))
					return Logger::SaveMessage("Error: Model::LoadModel() - 9.");
			}
		}

		else if (!lineBuffer.fail())
			continue;

		if (!lineBuffer.eof() || lineBuffer.fail())
		{
			materialMap.clear();
			return Logger::SaveMessage("Error: Model::LoadModel() - 10.");
		}
	}

	for (meshIterator = meshMap.begin(); meshIterator != meshMap.end(); ++meshIterator)
	{
		if (!meshIterator->second.LoadMaterial(materialMap[meshIterator->first]))
			return Logger::SaveMessage("Error: Model::LoadModel() - 11.");

		else if (!meshIterator->second.LoadShaders(StaticShader::Shaders[0], StaticShader::Shaders[2]))
			return Logger::SaveMessage("Error: Model::LoadModel() - 12.");

		else if (sendToGPU && !meshIterator->second.Create())
			return Logger::SaveMessage("Error: Model::LoadModel() - 13.");

		this->Meshes.push_back(std::move(meshIterator->second));
	}

	return true;
}

bool Model::LoadMesh(MeshObject& mesh)
{
	this->Meshes.push_back(std::move(mesh));

	if (!this->Meshes.back().Create())
	{
		this->Meshes.pop_back();
		return Logger::SaveMessage("Error: Model::LoadMesh() - 1.");
	}

	return true;
}

bool Model::Render() const
{
	bool result = true;

	for (size_t index = 0; index < this->Meshes.size(); ++index)
		if (!this->Meshes[index].Render(this->Transform))
			result = false;

	if (!result)
		return Logger::SaveMessage("Error: Model::Render() - 1.");

	return result;
}

bool Model::RenderIndexed(const std::vector<size_t>& indices) const
{
	if(this->Meshes.size() != 2)
		return Logger::SaveMessage("Error: Model::RenderIndexed() - 1.");

	else if(!this->Meshes[0].Render(this->Transform))
		return Logger::SaveMessage("Error: Model::RenderIndexed() - 2.");

	else if(!this->Meshes[1].RenderIndexed(this->Transform, indices))
		return Logger::SaveMessage("Error: Model::RenderIndexed() - 3.");

	return true;
}

bool Model::Destroy()
{
	bool result = true;

	for (size_t index = 0; index < this->Meshes.size(); ++index)
		if (!this->Meshes[index].Destroy())
			result = false;

	this->Meshes.clear();
	this->Transform = glm::mat4(1.0f);

	if (!result)
		return Logger::SaveMessage("Error: Model::Destroy() - 1.");

	return result;
}

static std::map<std::string, Material> ParseMaterialFile(const std::string& materialPath, const std::string& materialFile)
{
	std::ifstream inputFile;
	std::stringstream inputBuffer, lineBuffer;
	std::map<std::string, Material> materialMap;
	std::string inputLine, inputToken;
	std::map<std::string, Material>::iterator materialIterator = materialMap.end();

	inputFile.open(materialPath + materialFile);

	if (!inputFile)
		return materialMap;

	inputBuffer << inputFile.rdbuf();
	inputFile.close();

	while (std::getline(inputBuffer, inputLine))
	{
		if (inputLine.empty())
			continue;

		lineBuffer = std::stringstream();
		lineBuffer << inputLine;
		lineBuffer >> inputToken;

		if (inputToken == "newmtl")
		{
			lineBuffer >> inputToken;
			materialIterator = materialMap.find(inputToken);

			if (materialIterator != materialMap.end())
			{
				materialMap.clear();
				return materialMap;
			}

			materialIterator = materialMap.insert(materialMap.begin(), std::make_pair(inputToken, Material()));
		}

		else if (inputToken == "Ns")
			lineBuffer >> materialIterator->second.Shininess;

		else if (inputToken == "Ka")
			lineBuffer >> materialIterator->second.Ambient.x >> materialIterator->second.Ambient.y >> materialIterator->second.Ambient.z;

		else if (inputToken == "Kd")
			lineBuffer >> materialIterator->second.Diffuse.x >> materialIterator->second.Diffuse.y >> materialIterator->second.Diffuse.z;

		else if (inputToken == "Ks")
			lineBuffer >> materialIterator->second.Specular.x >> materialIterator->second.Specular.y >> materialIterator->second.Specular.z;

		else if (inputToken == "map_Kd")
		{
			lineBuffer >> materialIterator->second.DiffuseTexture;
			materialIterator->second.DiffuseTexture = materialPath + materialIterator->second.DiffuseTexture;
		}

		else if (inputToken == "map_Ks")
		{
			lineBuffer >> materialIterator->second.SpecularTexture;
			materialIterator->second.SpecularTexture = materialPath + materialIterator->second.SpecularTexture;
		}

		else if (!lineBuffer.fail())
			continue;

		if (!lineBuffer.eof() || lineBuffer.fail())
		{
			materialMap.clear();
			return materialMap;
		}
	}

	return materialMap;
}