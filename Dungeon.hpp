#ifndef Dungeon_H
#define Dungeon_H

#include "Mesh.hpp"
#include "Shader.hpp"

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
	static Mesh ShellMesh, WallMesh;

	static bool GenerateMesh(const uint16_t, const uint16_t, const bool = true, const bool = true);
	static bool RenderShellMesh(const Shader&);
	static bool RenderWallMesh(const Shader&);
	static bool LockCamera();
	static bool DestroyMesh();
};

#endif