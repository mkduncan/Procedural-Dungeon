#ifndef Dungeon_H
#define Dungeon_H

#include "AlignedBox.hpp"
#include "Model.hpp"

//
#include "MeshObject.hpp"
//

class Dungeon
{
private:
	Dungeon() = delete;
	~Dungeon() = delete;
	Dungeon(const Dungeon&) = delete;
	Dungeon(Dungeon&&) noexcept = delete;
	Dungeon& operator=(const Dungeon&) = delete;
	Dungeon& operator=(Dungeon&&) noexcept = delete;

public:
	static float MeshScale, MeshHeight, TextureScale;

	static bool GenerateModel(const uint16_t, const uint16_t, Model&, std::vector<AlignedBox>&);
	static bool GenerateMesh(const uint16_t, const uint16_t, Mesh&);
	static bool LockCamera();
};

#endif