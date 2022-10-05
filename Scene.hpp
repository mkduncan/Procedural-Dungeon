#ifndef Scene_H
#define Scene_H

#include "AlignedBoxTree.hpp"
#include "Frustum.hpp"
#include "Light.hpp"
#include "Model.hpp"

#include <map>

class Scene
{
public:
	std::map<std::string, Model> Models;
	std::vector<Light> Lights;
	std::vector<AlignedBox> Boundaries;
	AlignedBoxTree Nodes;

	Scene();
	~Scene();
	Scene(const Scene&);
	Scene(Scene&&) noexcept;
	Scene& operator=(const Scene&);
	Scene& operator=(Scene&&) noexcept;

	bool GenerateDungeon(const uint16_t, const uint16_t);
	bool Update();
	bool Render() const;
	bool Destroy();
};

#endif