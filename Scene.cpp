#include "Camera.hpp"
#include "Dungeon.hpp"
#include "Logger.hpp"
#include "Scene.hpp"
#include "StaticShader.hpp"

Scene::Scene() : Models(std::map<std::string, Model>()), Lights(std::vector<Light>()), Boundaries(std::vector<AlignedBox>()), Nodes(AlignedBoxTree())
{
}

Scene::~Scene()
{
	this->Destroy();
}

Scene::Scene(const Scene& scene) : Models(scene.Models), Lights(scene.Lights), Boundaries(scene.Boundaries), Nodes(scene.Nodes)
{
}

Scene::Scene(Scene&& scene) noexcept : Models(std::move(scene.Models)), Lights(std::move(scene.Lights)), Boundaries(std::move(scene.Boundaries)), Nodes(std::move(scene.Nodes))
{
	scene.Destroy();
}

Scene& Scene::operator=(const Scene& scene)
{
	this->Models = scene.Models;
	this->Lights = scene.Lights;
	this->Boundaries = scene.Boundaries;
	this->Nodes = scene.Nodes;

	return *this;
}

Scene& Scene::operator=(Scene&& scene) noexcept
{
	this->Models = std::move(scene.Models);
	this->Lights = std::move(scene.Lights);
	this->Boundaries = std::move(scene.Boundaries);
	this->Nodes = std::move(scene.Nodes);
	scene.Destroy();

	return *this;
}

bool Scene::GenerateDungeon(const uint16_t sizeX, const uint16_t sizeY)
{
	std::map<std::string, Model>::iterator modelIterator;

	if (!this->Destroy())
		return Logger::SaveMessage("Error: Scene::GenerateDungeon() - 1.");

	modelIterator = this->Models.insert(this->Models.begin(), std::make_pair("DungeonShell", Model()));

	if (!Dungeon::GenerateModel(sizeX, sizeY, modelIterator->second, this->Boundaries))
		return Logger::SaveMessage("Error: Scene::GenerateDungeon() - 2.");

	else if (modelIterator->second.Meshes.size() != 2)
		return Logger::SaveMessage("Error: Scene::GenerateDungeon() - 3.");

	else if (!modelIterator->second.Meshes.front().LoadMaterial(glm::vec3(1.0f), glm::vec3(0.8f), glm::vec3(0.5f), 360.0f))
	{
		this->Destroy();
		return Logger::SaveMessage("Error: Scene::GenerateDungeon() - 4.");
	}

	else if(!modelIterator->second.Meshes.back().LoadMaterial(glm::vec3(1.0f), glm::vec3(0.8f), glm::vec3(0.5f), 360.0f))
	{
		this->Destroy();
		return Logger::SaveMessage("Error: Scene::GenerateDungeon() - 5.");
	}

	else if (!modelIterator->second.Meshes.front().LoadShaders(StaticShader::Shaders[0], StaticShader::Shaders[2]))
	{
		this->Destroy();
		return Logger::SaveMessage("Error: Scene::GenerateDungeon() - 6.");
	}

	else if (!modelIterator->second.Meshes.back().LoadShaders(StaticShader::Shaders[0], StaticShader::Shaders[2]))
	{
		this->Destroy();
		return Logger::SaveMessage("Error: Scene::GenerateDungeon() - 7.");
	}

	else if (!modelIterator->second.Meshes.front().LoadTextures("Assets/dirt_texture.png"))
	{
		this->Destroy();
		return Logger::SaveMessage("Error: Scene::GenerateDungeon() - 8.");
	}

	else if (!modelIterator->second.Meshes.back().LoadTextures("Assets/rock_texture.png"))
	{
		this->Destroy();
		return Logger::SaveMessage("Error: Scene::GenerateDungeon() - 9.");
	}

	else if (!modelIterator->second.Meshes.front().Create())
	{
		this->Destroy();
		return Logger::SaveMessage("Error: Scene::GenerateDungeon() - 10.");
	}

	else if (!modelIterator->second.Meshes.back().Create())
	{
		this->Destroy();
		return Logger::SaveMessage("Error: Scene::GenerateDungeon() - 11.");
	}

	this->Lights.emplace_back();
	this->Lights.back().Position = Camera::Position;
	this->Lights.back().Direction = Camera::Front;
	this->Lights.back().InnerCutoff = glm::cos(glm::radians(30.0f));
	this->Lights.back().OuterCutoff = glm::cos(glm::radians(45.0f));
	this->Lights.back().Constant = 1.0f;
	this->Lights.back().Linear = 0.14f;
	this->Lights.back().Quadradic = 0.07f;
	this->Lights.back().Type = SPOT_LIGHT;

	this->Nodes = AlignedBoxTree(this->Boundaries.size());

	for (size_t index = 0; index < this->Boundaries.size(); ++index)
		this->Nodes.InsertObject(this->Boundaries[index]);

	return true;
}

bool Scene::Update()
{
	if(this->Models.empty())
		return Logger::SaveMessage("Error: Scene::Update() - 1.");

	else if(this->Lights.empty())
		return Logger::SaveMessage("Error: Scene::Update() - 2.");

	this->Lights.front().Position = Camera::Position;
	this->Lights.front().Direction = Camera::Front;


	//TODO: initialize camera location if not done so; lock camera with collision detections
	//TODO: utilize new shaders with multiple dynamic lights

	return true;
}

bool Scene::Render() const
{
	std::vector<size_t> indices;
	Frustum frustum;
	bool result = true;

	if (!frustum.Update())
		return Logger::SaveMessage("Error: Scene::Render() - 2.");

	indices = this->Nodes.QueryOverlaps(frustum);

	for (std::map<std::string, Model>::const_iterator modelIterator = this->Models.begin(); modelIterator != this->Models.end(); ++modelIterator)
		if (modelIterator->first == "DungeonShell" && !modelIterator->second.RenderIndexed(indices) || !modelIterator->second.Render())
			result = false;

	if (!result)
		return Logger::SaveMessage("Error: Scene::Render() - 3.");

	return result;
}

bool Scene::Destroy()
{
	bool result = true;

	for (std::map<std::string, Model>::iterator modelIterator = Models.begin(); modelIterator != Models.end(); ++modelIterator)
		if (!modelIterator->second.Destroy())
			result = false;

	this->Models.clear();

	for (size_t index = 0; index < Lights.size(); ++index)
		if (!Lights[index].Reset())
			result = false;

	this->Lights.clear();
	this->Boundaries.clear();

	if (!this->Nodes.Reset())
		result = false;

	if (!result)
		return Logger::SaveMessage("Error: Scene::Destroy() - 1.");

	return true;
}